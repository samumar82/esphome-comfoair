# ComfoAir 180 + ComfoSense + Home Assistant
## The ESP32 bidirectional bridge

**Keep your original ComfoSense wall controller. Add full Home Assistant control. Keep both in sync.**

This project documents my working ESP32-based integration for the **Zehnder ComfoAir 180** ventilation unit and its original **ComfoSense** control panel.

The key feature is **a true two-way serial bridge**, rather than simply replacing the physical controller with Home Assistant:

- **Change ventilation speed on ComfoSense** → ComfoAir 180 responds → **Home Assistant updates**.
- **Change ventilation speed in Home Assistant** → ComfoAir 180 responds → **ComfoSense display updates too**, without touching the panel.
- In my installation, the panel typically reflects Home Assistant changes in **about 4–5 seconds**.

> **Tested in a real installation:** Zehnder ComfoAir **180**, ComfoSense, ESP32-WROOM-32D, ESPHome **2026.6.5**. Compatibility with other units or panels has not yet been validated for this combined bridge.

---

## Why I built it

I wanted the **best of both worlds**: keep the reliable, familiar **ComfoSense** controller on the wall, while exposing the ComfoAir 180 to Home Assistant for monitoring, control and automations.

Simply controlling fan speed from Home Assistant was not enough. The original panel needed to **stay aware of changes made from Home Assistant**, and a speed change on ComfoSense also needed to appear correctly in HA.

This was the missing piece — and the main reason for this project.

## How the bridge works

```text
                              HOME ASSISTANT
                           Fan control • Sensors
                             Climate • Automations
                                    │
                              ESPHome native API
                                    │
                              Wi-Fi connection
                                    │
                           ┌──────────────────┐
                           │ ESP32-WROOM-32D  │
                           │                  │
                           │  Svenar ESPHome  │
                           │  ComfoAir driver │
                           │        +         │
                           │ ComfoSense UART  │
                           │  bridge / proxy  │
                           └──────┬─────┬─────┘
                                  │     │
                          UART 1  │     │  UART 2
                                  │     │
                       MAX3232    │     │    MAX3232
                                  │     │
                         ┌────────┘     └─────────┐
                         │                        │
                  ZEHNDER COMFOAIR 180      COMFOSENSE
                    Ventilation unit       Original wall panel

        Home Assistant ↔ ESP32 ↔ ComfoAir ↔ ESP32 ↔ ComfoSense
                  Two-way fan-speed synchronization
```

The ESP32 connects to **two independent serial interfaces**. It speaks to the ventilation unit using the modern ComfoAir ESPHome component and proxies the original ComfoSense communication, coordinating both traffic streams.

### The essential synchronization fix

During development, **ComfoSense → Home Assistant** already worked, while **Home Assistant → ComfoSense** did *not*: the ventilation unit changed speed, but the wall panel display remained unchanged.

The working solution was to forward **complete, validated response frames** back to ComfoSense, following the approach used by Julianpas, rather than forwarding serial data byte by byte.

**Result:** both controllers now work together, with the physical panel updating after changes made in Home Assistant.

## My real hardware setup

| Component | My configuration |
|---|---|
| Ventilation unit | **Zehnder ComfoAir 180** |
| Physical controller | **Original Zehnder ComfoSense** — retained and fully usable |
| ESP32 board | **ESP32-WROOM-32D** |
| Serial level conversion | **MAX3232-based RS232 ↔ TTL converters** for both interfaces |
| ComfoAir UART | **ESP32 RX GPIO16 / TX GPIO17** |
| ComfoSense UART | **ESP32 RX GPIO25 / TX GPIO26** |
| UART speed | **9600 baud** |
| Power | ComfoAir 12 V supply → regulated ~5 V step-down → ESP32 5 V input |
| Software | **ESPHome 2026.6.5** (ESP-IDF) |
| Integration | **Home Assistant via native ESPHome API** |

> **Important electrical warning:** use proper RS232-to-3.3 V TTL level conversion; never attach RS232 voltages directly to ESP32 GPIO pins. Check your converter's TX/RX labeling and power requirements — module pin labels are not standardized. The wiring above describes **my tested setup**, not a universal Zehnder pinout.

## What works on my ComfoAir 180

| Function | Verified behavior |
|---|---|
| ComfoSense → ComfoAir | ✅ Original panel controls ventilation speed |
| ComfoSense → Home Assistant | ✅ HA reflects fan-speed changes |
| Home Assistant → ComfoAir | ✅ HA controls ventilation speed |
| Home Assistant → ComfoSense | ✅ Original panel display follows HA changes |
| Panel update time | Typically **4–5 seconds** |
| Temperature sensors | ✅ Outdoor, supply, return and exhaust |
| Home Assistant Climate entity | ✅ Current temperature and comfort setpoint are separate |
| Current Climate temperature | ✅ Uses the **return-air temperature** (e.g. 21 °C) |
| Comfort temperature setpoint | ✅ Remains independent (e.g. 23 °C) |
| No enthalpy option installed | ✅ Avoids a misleading `-20 °C` reading |
| Preheating indication | ✅ No spurious on/off oscillation observed |

The rest of Svenar's ComfoAir monitoring and controls remain part of the integration, including ventilation levels, bypass, fan speed, operating hours and other supported values.

## Device compatibility

**Confirmed on this combined bridge implementation:**
- **Zehnder ComfoAir 180 + ComfoSense** — working installation.

Svenar's upstream repository lists additional ComfoAir, ComfoD, WHR and ComfoAir Q models. **That does not mean this two-UART ComfoSense bridge has been tested on all of them.** Other owners are welcome to report successful or unsuccessful combinations.

## Source code and installation status

> **Important:** this fork currently contains Svenar's upstream repository. The **final modified source code and configuration of my working two-UART bridge have not yet been pushed to this GitHub repository**. The README describes a real, verified installation, but **cloning the repository as it stands does not yet reproduce the combined bridge**.

The working build is based on Svenar's `comfoair_ca` component plus the ComfoSense UART proxy and fixes described above. The release is ready to be documented here, but the exact locally tested `components/comfoair_ca/` and corresponding ESPHome bridge YAML must be committed before publishing an installation guide or release.

**Please do not flash the current repository expecting the bidirectional bridge until the source upload is complete.**

## Credits

This project combines and builds upon excellent open-source work:

- **[Svenar — esphome-comfoair](https://github.com/svenar-iot/esphome-comfoair):** the modern ESPHome ComfoAir integration, sensors and Climate controls.
- **[Julianpas — esphome-comfoair](https://github.com/julianpas/esphome-comfoair):** the reference ComfoSense proxy and the complete-frame forwarding behavior essential to this bridge.
- **[Wichers — esphome-comfoair](https://github.com/wichers/esphome-comfoair):** the original project lineage of the Julianpas fork.

This repository is a **community fork of Svenar**, and is not affiliated with Zehnder or the upstream authors. Source code remains under the **[GNU GPL v3.0](LICENSE)** license, with the original authors and notices retained.

---

**The goal:** one ComfoAir 180, two fully usable control interfaces — **ComfoSense on the wall and Home Assistant in the smart home**, kept in sync by a single ESP32 bridge.
