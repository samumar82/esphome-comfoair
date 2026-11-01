# Changelog

## 0.1.0-beta — 2026-11-01

- Created a two-UART bidirectional **ComfoAir 180 ↔ ESP32 ↔ ComfoSense** bridge with Home Assistant support.
- Kept Svenar's broad set of climate, ventilation, sensor and control entities.
- Implemented panel traffic priority, bounded UART frame queues and request ownership.
- Fixed HA → ComfoSense display synchronization by forwarding **complete, validated, unmodified RS232 frames** as a burst (Julianpas-style).
- Fixed absent enthalpy sensor reporting, avoiding misleading −20 °C.
- Prevented preheating state oscillation due to unrelated valve response fields.
- Published the measured return-air temperature to the Climate entity.
- Added `CLIMATE_SUPPORTS_CURRENT_TEMPERATURE`, required by ESPHome 2026.6.5 to send that value through the native API.
- Added standalone local-component ESPHome YAML, setup documentation and no-secret placeholders.

**Testing:** working on a real ComfoAir 180 installation with ESPHome 2026.6.5 and ComfoSense. The public repository sources were reconstructed from the patch sequence used for that installation. A clean ESPHome 2026.6.5 configuration validation and firmware compilation **passed on GitHub Actions** (run 41348538276). A byte-for-byte comparison with the final Windows build has not been performed.
