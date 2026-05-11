# Loxone Template

This folder contains a Loxone Config Network template for the Clack Reader V4 `/json` endpoint.

## Files

- `clack-reader-v4-json.xml` - Virtual HTTP Input template for Loxone Config.

## Import

1. In Loxone Config, import `clack-reader-v4-json.xml` as a Network template or copy it into the Loxone templates folder for Network templates.
2. Insert the template under Virtual Inputs / Network templates.
3. Change the Virtual HTTP Input address from `http://192.168.x.x/json` to the real device URL, for example `http://192.168.1.50/json`.
4. Keep the polling cycle at `10` seconds or higher. Loxone documents `10s` as the minimum polling cycle.

The `/json` endpoint is intentionally unauthenticated in this firmware, so no token is required for this template.

## Included Values

The template includes 34 analog inputs for numeric JSON fields such as salt level, flow rate, water meter, capacity remaining, power, voltage, WiFi signal, uptime, hardness, and configuration values.

It also includes 25 digital flags for boolean JSON fields and useful string states, including leakage detected, water flowing, chlorinator relay, fill salt needed, cycle step, function mode, regeneration mode, and hardness class.

The string-state flags assume the currently active English labels package in `esphome/clack.yaml`. If you switch labels and the JSON text values change, update the matching `Check` strings in the XML.

Text-only values such as `timestamp`, `version`, `water_softener_ip`, date/time strings, and formatted duration strings are not included as analog values because Loxone Virtual HTTP Input command recognition is primarily numeric or digital. Use the matching digital state flags when you need logic based on these textual states.

## Pattern Notes

The command recognition patterns match the compact JSON emitted by `json_endpoint.h`, for example:

```text
"water_meter":\v
"leakage_detected":true
"cycle_step":"Brine"
```

If the firmware output is changed to pretty-printed JSON with spaces around colons, update the `Check` attributes in the XML accordingly.
