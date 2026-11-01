# Real-device tests and regression checklist

## Verified on the development installation

- ComfoAir **180**; original **ComfoSense** control panel.
- ESP32-WROOM-32D, two MAX3232-class serial level-shifting interfaces.
- ESPHome **2026.6.5**.
- HA → ComfoAir → ComfoSense **fan display updates** observed around 4–5 seconds.
- ComfoSense → ComfoAir → HA fan speed synchronized.
- Climate `current_temperature` = **21 °C** while `temperature` setpoint = **23 °C** in one observed reading.
- Absent enthalpy sensor no longer falsely reports −20 °C.
- Preheating indication no longer oscillates in the reported test.

## Repeat these checks on your own hardware

1. Validate and compile the public `comfoair_svenar_proxy.yaml` using ESPHome 2026.6.5.
2. Ensure no ComfoSense `COMM ERROR` on startup or idle.
3. From the wall panel, select LOW, MEDIUM and HIGH: HA must follow.
4. From HA, select LOW, MEDIUM and HIGH: **look at the physical ComfoSense display** (allow up to 15 seconds during testing).
5. Read the four separate temperature sensors; the Climate current temperature should follow return air, not the comfort setpoint.
6. Confirm that an absent enthalpy option produces no fake −20 °C reading.
7. Confirm preheating is stable, ventilation levels don't randomly change, and the device doesn't reboot.

## Evidence boundary

The assembled firmware used in the real installation **was compiled on its owner's Windows PC**. This public fork was reconstructed from that patched source workflow and passed a **fresh ESPHome 2026.6.5 configuration validation and firmware compilation on GitHub Actions** ([successful build](https://github.com/samumar82/esphome-comfoair/actions/runs/41348538276)). Pylint also passed ([run](https://github.com/samumar82/esphome-comfoair/actions/runs/41348762959)). The public source has **not** been byte-compared to the Windows PC's final build; no new physical-device flash has been performed from the repository.
