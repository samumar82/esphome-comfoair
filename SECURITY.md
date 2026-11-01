# Security notes

The public YAML is a starting point and intentionally uses **secret placeholders** for Wi-Fi, OTA and the fallback access point. Do not publish real values.

1. Copy `secrets.example.yaml` to `secrets.yaml`. Replace every example value.
2. Never commit `secrets.yaml`, `.esphome/`, firmware binaries, or logs containing network addresses or device identifiers.
3. The baseline configuration mirrors the tested native ESPHome API setup. It **does not configure API encryption or web_server authentication**. For deployments beyond a trusted, isolated LAN, enable appropriate authentication/encryption and adjust Home Assistant pairing accordingly.
4. Never expose ESPHome OTA, API, captive AP or web server directly to the internet. Avoid public port forwarding.
5. Use dedicated, properly rated RS232 transceivers. ESP32 GPIO is **not RS232 tolerant**.
6. Test firmware changes carefully: interruption of the serial link to a ventilation unit may affect normal operation.

Please do not post Wi-Fi passwords, OTA secrets, device-specific telemetry, or private network captures in public issues.
