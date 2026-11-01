# How the bidirectional bridge works

## The key design choice

An ESP32 maintains two UART buses:

1. `uart_bus` ↔ Zehnder ComfoAir 180 (GPIO17 TX / GPIO16 RX).
2. `uart_comfosense_proxy` ↔ original ComfoSense (GPIO26 TX / GPIO25 RX).

The **Svenar** ESPHome component parses machine replies into Home Assistant sensors and Climate state. The integrated **ComfoSenseBridge** receives complete panel requests and queues them for the machine UART, with ownership/quiet-time arbitration against ESPHome's own polling and control commands.

## Critical fix: machine → panel complete-frame transport

An early bridge version forwarded each byte as it arrived. This let Home Assistant control the ComfoAir and let ComfoSense control Home Assistant, **but changes made from Home Assistant did not update the wall panel display**.

`COMFOSENSE_JULIAN_FRAME_SYNC_V1` corrected this: the machine reply is buffered, checksum validated, and sent in one UART burst without rewriting the original wire bytes. The technique is adapted from Julianpas's working approach.

Real-device observation: the physical ComfoSense display now follows HA-initiated speed changes, typically within **4–5 seconds**.

## Other fixes

- `COMFOSENSE_SYNC_SENSOR_FIX_V1`: mask enthalpy temperature when the hardware option isn't installed; prevent contradictory preheating state updates.
- `COMFOAIR_CLIMATE_TEMP_FIX_V1`: update current Climate temperature from the ComfoAir return-air measurement.
- `COMFOAIR_CLIMATE_CURRENT_TEMP_FEATURE_V2`: advertise current-temperature support to the ESPHome API so HA receives it rather than `null`.
- `COMFOSENSE_TRACE_V1`: diagnostic frame summaries used during verification; they are currently compiled into the firmware source. Lower logging in a separate tested update.

## Limitations

- Serial bus access is single-owner/arbitrated; incoming panel messages take priority.
- Confirmed on **one ComfoAir 180 + ComfoSense** installation, not every Zehnder model.
- The four-to-five-second panel refresh observed in this installation should not be mistaken for real-time instantaneous synchronization.
