# Actual hardware setup — ComfoAir 180 + ComfoSense

This documents the installation on which the bidirectional bridge was tested, **not** a universally safe wiring diagram for every Zehnder model.

| Component | Tested configuration |
| --- | --- |
| Ventilation unit | Zehnder ComfoAir 180 |
| Original wall control | Zehnder ComfoSense |
| Microcontroller | ESP32-WROOM-32D |
| Serial hardware | Two RS232 ↔ 3.3 V TTL interfaces (MAX3232 family) |
| Machine-side UART | GPIO17 TX, GPIO16 RX |
| Wall-panel UART | GPIO26 TX, GPIO25 RX |
| Serial speed | 9600 baud for both UARTs |
| Firmware | ESPHome 2026.6.5 / ESP-IDF |
| Power | ComfoAir low-voltage source → **regulated** 5 V step-down → ESP32 power input |
| Home Assistant | Native ESPHome API over local Wi-Fi |

## Logical connections

```text
ComfoAir 180 RS232     RS232↔TTL #1        ESP32
Unit TX ────────────── RS232 RX   TTL TX ── GPIO16 RX
Unit RX ────────────── RS232 TX   TTL RX ── GPIO17 TX

ComfoSense RS232       RS232↔TTL #2        ESP32
Panel TX ───────────── RS232 RX   TTL TX ── GPIO25 RX
Panel RX ───────────── RS232 TX   TTL RX ── GPIO26 TX

Power → properly rated regulated 5 V input → ESP32
Grounding/shielding → consult the exact transceiver board & ComfoAir documentation
```

**Pin-name warning:** different MAX3232 boards label their TTL pins differently. The important verified firmware mapping is `GPIO17/16` for the machine and `GPIO26/25` for the panel. Check actual signal direction before wiring.

## Safety and verification

- RS232 signal voltage is **not** ESP32 3.3 V TTL. Never attach RS232 wires directly to ESP32 GPIO.
- Do not apply ComfoAir 12 V directly to the ESP32 USB, 3V3 or 5V pin.
- Confirm wiring against the *actual* ComfoAir 180 and ComfoSense connectors; never rely on wire colors alone.
- Power down before reconnecting serial leads, avoid short circuits, and preserve an original recovery path.
- No additional AI hardware or Home Assistant serial integration is required for this bridge.
