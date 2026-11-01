# Credits and provenance

This project is a community fork of [Svenar's ESPHome ComfoAir project](https://github.com/svenar-iot/esphome-comfoair). It builds on the work of:

- **Svenar** — ESPHome `comfoair_ca` component, fan/climate controls, native Home Assistant integration and extensive ComfoAir sensors.
- **Julianpas** — [esphome-comfoair](https://github.com/julianpas/esphome-comfoair): working ComfoSense proxy architecture and the crucial complete-frame forwarding approach.
- **Wichers** — [esphome-comfoair](https://github.com/wichers/esphome-comfoair), the original project underlying Julianpas's fork.
- **yoziru** — [esphome-zehnder-comfoair](https://github.com/yoziru/esphome-zehnder-comfoair), credited by Svenar upstream.

The integration in this fork combines Svenar's modern component with a two-UART ComfoSense bridge, queue/arbitration logic, serial frame forwarding and fixes for absent enthalpy sensors, preheating and the Climate current-temperature API feature flag.

## Licensing

The Svenar, Julianpas and Wichers repositories use **GNU General Public License v3.0**. This combined derivative is distributed under the same license; see [LICENSE](LICENSE). Original copyright notices in copied files are retained.

A GitHub fork can have only one fork parent, so the parent is Svenar. Other related projects are linked and acknowledged here.

**Unofficial community project** — not endorsed by Zehnder or any upstream author.
