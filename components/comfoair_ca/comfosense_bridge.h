#pragma once
// ComfoSense two-UART proxy for Svenar's comfoair_ca (ESPHome ESP32).
// Origin: this project's verified multi-UART integration; design based on
// https://github.com/julianpas/esphome-comfoair (GPL-3.0).
// Bounded frame collection and single-owner half-duplex arbitration.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include "frame.h"
#include "checksum.h"
#include "registers.h"

namespace esphome {
namespace comfoair_ca {

class ComfoSenseBridge {
 public:
  void set_proxy_uart_component(uart::UARTComponent *parent) { panel_.set_uart_parent(parent); enabled_ = (parent != nullptr); }
  bool enabled() const { return enabled_; }

  // Polling must yield whenever a panel request (or its reply) is outstanding.
  bool defer_poll() { return enabled_ && (owner_ != Owner::NONE || count_ != 0 || panel_.available() != 0); }

  // Called at the end of ComfoAirComponent::write_tx(), before any local UART write.
  // False means caller may send now, true means this command was deferred.
  bool intercept_local_tx(const uint8_t *data, std::size_t length) {
    if (!enabled_) return false;
    if (owner_ == Owner::NONE && count_ == 0 && panel_.available() == 0) {
      set_owner_(Owner::LOCAL);
      return false;
    }
    if (length == 0 || length > MAX_WIRE || pending_local_valid_) {
      ESP_LOGW("comfosense_proxy", "Local command queue full/invalid (len=%u)", static_cast<unsigned>(length));
      return true;
    }
    memcpy(pending_local_, data, length);
    pending_local_len_ = static_cast<uint16_t>(length);
    pending_local_valid_ = true;
    return true;
  }

  void read_panel() {
    if (!enabled_) return;
    const uint32_t now = millis();
    if (partial_.raw_len != 0 && static_cast<uint32_t>(now - last_panel_byte_ms_) > FRAME_TIMEOUT_MS) {
      reset_partial_();
    }
    // Finite budget: do not starve ESPHome WiFi/API scheduler.
    for (unsigned i = 0; i < 128 && panel_.available() != 0; ++i) {
      uint8_t byte = 0;
      if (!panel_.read_byte(&byte)) break;
      last_panel_byte_ms_ = millis();
      accept_panel_byte_(byte);
    }
  }

  // COMFOSENSE_JULIAN_FRAME_SYNC_V1
  // Julianpas forwards each *complete validated frame* to the panel.
  // Byte-wise forwarding can introduce gaps inside one serial frame.
  // Preserve exact wire data (including escaping); no synthetic commands.
  void on_machine_byte(uint8_t byte) {
    if (!enabled_) return;
    const uint32_t now = millis();
    if (owner_ != Owner::NONE) {
      saw_rx_ = true;
      last_machine_byte_ms_ = now;
    }
    if (machine_partial_.raw_len != 0 &&
        static_cast<uint32_t>(now - last_machine_frame_byte_ms_) > FRAME_TIMEOUT_MS) {
      machine_partial_ = Collector{};
    }
    last_machine_frame_byte_ms_ = now;
    accept_machine_byte_(byte);
  }

  // Kept for compatibility with ComfoAirComponent::loop().
  // Panel writes are flushed immediately on a complete frame.
  void flush_panel() {}

  template <typename MachineUart>
  void tick(MachineUart &machine) {
    if (!enabled_) return;
    const uint32_t now = millis();
    if (owner_ != Owner::NONE) {
      // Avoid interleaving a machine response with a different request.
      if ((saw_rx_ && (uint32_t)(now - last_machine_byte_ms_) >= QUIET_AFTER_RX_MS) ||
          (uint32_t)(now - owner_start_ms_) >= REQUEST_TIMEOUT_MS) {
        owner_ = Owner::NONE;
      }
    }
    if (owner_ != Owner::NONE) return;

    // Panel always gets priority over Home Assistant polling/control.
    if (count_ != 0) {
      const Packet &p = pending_[head_];
      machine.write_array(p.data, p.len);
      machine.flush();
      const bool is_ack = p.len == 2 && p.data[0] == FRAME_PREFIX && p.data[1] == FRAME_ACK;
      head_ = (head_ + 1) % QUEUE_DEPTH;
      --count_;
      // A two-byte ACK has no response. Do not monopolize the line for 500ms.
      if (!is_ack) set_owner_(Owner::PANEL);
      return;
    }
    if (pending_local_valid_) {
      machine.write_array(pending_local_, pending_local_len_);
      machine.flush();
      pending_local_valid_ = false;
      pending_local_len_ = 0;
      set_owner_(Owner::LOCAL);
    }
  }

 private:
  static constexpr std::size_t MAX_WIRE = (2 * COMMAND_MAX_LENGTH) + 8;
  static constexpr std::size_t QUEUE_DEPTH = 4;
  static constexpr uint32_t FRAME_TIMEOUT_MS = 250;
  static constexpr uint32_t REQUEST_TIMEOUT_MS = 500;
  static constexpr uint32_t QUIET_AFTER_RX_MS = 110;

  enum class Owner : uint8_t { NONE, LOCAL, PANEL };
  struct Packet { uint8_t data[MAX_WIRE]{}; uint16_t len{0}; };
  struct Collector {
    uint8_t normalized[COMMAND_MAX_LENGTH]{};
    uint8_t raw[MAX_WIRE]{};
    uint16_t raw_len{0};
    uint8_t index{0};
    bool skip_escaped{false};
  };

  void set_owner_(Owner o) {
    owner_ = o;
    owner_start_ms_ = millis();
    last_machine_byte_ms_ = owner_start_ms_;
    saw_rx_ = false;
  }
  void reset_partial_() { partial_ = Collector{}; }

  void enqueue_complete_() {
    if (count_ >= QUEUE_DEPTH) {
      ESP_LOGW("comfosense_proxy", "Panel command queue full: frame dropped");
    } else {
      Packet &p = pending_[tail_];
      memcpy(p.data, partial_.raw, partial_.raw_len);
      p.len = partial_.raw_len;
      tail_ = (tail_ + 1) % QUEUE_DEPTH;
      ++count_;
    }
    reset_partial_();
  }

  // COMFOSENSE_JULIAN_FRAME_SYNC_V1
  // Independent bounded RX collector for machine -> panel UART.
  // Never modifies source bytes; validates before emitting as one burst.
  void accept_machine_byte_(uint8_t byte) {
    Collector &p = machine_partial_;
    if (p.raw_len >= MAX_WIRE || p.index >= COMMAND_MAX_LENGTH) {
      p = Collector{};
    }
    if (p.raw_len == 0 && byte != FRAME_PREFIX) return;

    p.raw[p.raw_len++] = byte;
    if (p.skip_escaped) {
      p.skip_escaped = false;
      if (byte == ESCAPE_BYTE) return;
    }
    const uint8_t index = p.index;
    p.normalized[index] = byte;
    const ByteCheck check = check_frame_byte(p.normalized, index);
    if (check.verdict == ByteVerdict::Complete) {
      panel_.write_array(p.raw, p.raw_len);
      panel_.flush();
      p = Collector{};
      return;
    }
    if (check.verdict == ByteVerdict::BadChecksum ||
        check.verdict == ByteVerdict::Invalid) {
      p = Collector{};
      if (byte == FRAME_PREFIX) {
        p.raw[0] = byte;
        p.normalized[0] = byte;
        p.raw_len = 1;
        p.index = 1;
      }
      return;
    }
    if (byte == ESCAPE_BYTE &&
        index_is_escaped_region(index, p.normalized[COMMAND_IDX_DATA])) {
      p.skip_escaped = true;
    }
    ++p.index;
  }

  void accept_panel_byte_(uint8_t byte) {
    if (partial_.raw_len >= MAX_WIRE || partial_.index >= COMMAND_MAX_LENGTH) {
      reset_partial_();
    }
    // Resynchronize at a new 0x07 prefix only after an invalid frame.
    if (partial_.raw_len == 0 && byte != FRAME_PREFIX) return;

    partial_.raw[partial_.raw_len++] = byte;
    if (partial_.skip_escaped) {
      partial_.skip_escaped = false;
      if (byte == ESCAPE_BYTE) return;
    }

    const uint8_t index = partial_.index;
    partial_.normalized[index] = byte;
    const ByteCheck check = check_frame_byte(partial_.normalized, index);
    if (check.verdict == ByteVerdict::Complete) {
      enqueue_complete_();
      return;
    }
    if (check.verdict == ByteVerdict::BadChecksum || check.verdict == ByteVerdict::Invalid) {
      reset_partial_();
      if (byte == FRAME_PREFIX) {
        partial_.raw[0] = byte;
        partial_.normalized[0] = byte;
        partial_.raw_len = 1;
        partial_.index = 1;
      }
      return;
    }
    if (byte == ESCAPE_BYTE && index_is_escaped_region(index, partial_.normalized[COMMAND_IDX_DATA])) {
      partial_.skip_escaped = true;
    }
    ++partial_.index;
  }

  uart::UARTDevice panel_;
  bool enabled_{false};
  // COMFOSENSE_JULIAN_FRAME_SYNC_V1
  Collector machine_partial_{};
  uint32_t last_machine_frame_byte_ms_{0};
  Owner owner_{Owner::NONE};
  uint32_t owner_start_ms_{0};
  uint32_t last_machine_byte_ms_{0};
  uint32_t last_panel_byte_ms_{0};
  bool saw_rx_{false};
  Collector partial_{};
  Packet pending_[QUEUE_DEPTH]{};
  uint8_t head_{0};
  uint8_t tail_{0};
  uint8_t count_{0};
  uint8_t pending_local_[MAX_WIRE]{};
  uint16_t pending_local_len_{0};
  bool pending_local_valid_{false};
};

}  // namespace comfoair_ca
}  // namespace esphome
