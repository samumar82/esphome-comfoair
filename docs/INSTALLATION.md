# Installation — ESPHome 2026.6.5

> **Important:** use `comfoair_svenar_proxy.yaml` for this project. The upstream `comfoair_ca.yaml` is retained in the fork but does **not** enable the two-UART ComfoSense bridge.

1. Clone/download this repository to a computer with Python 3 and ESPHome 2026.6.5.
2. Install/check the ESPHome version:
   ```powershell
   py -3 -m pip install "esphome==2026.6.5"
   py -3 -m esphome version
   ```
3. Create the local secrets file:
   ```powershell
   Copy-Item .\secrets.example.yaml .\secrets.yaml
   ```
   Edit all values in `secrets.yaml` to your private values.
4. Validate and compile:
   ```powershell
   py -3 -m esphome config .\comfoair_svenar_proxy.yaml
   py -3 -m esphome compile .\comfoair_svenar_proxy.yaml
   ```
5. **Only if wiring and UART levels are proven correct:** upload through your chosen USB or OTA route. For an initial USB flash, follow the official ESPHome ESP32 first-install procedure. Never flash a working appliance merely to check the documentation.
6. Integrate via ESPHome native API in Home Assistant. Find the `ComfoAir` Climate entity and the return/outside/supply/exhaust temperature sensors.

The YAML uses the **local component** path `components/comfoair_ca`; this avoids depending on a changing upstream branch and includes the bridge code.

## First-run checks

Use [TESTING.md](TESTING.md). Verify the real ComfoSense display, not merely HA's speed attribute; the latter alone does not prove bidirectional synchronization.

## Credentials and local security

The baseline uses secret placeholders for Wi-Fi, fallback access point and OTA password. See [../SECURITY.md](../SECURITY.md) before deployment. API encryption is not enabled in the sample; use a trusted LAN or secure it for your environment.
