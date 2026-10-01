# WhatsApp Doorbell — setup guide

Your ESP8266 + PIR motion sensor, talking to rserver, which messages your
WhatsApp through the Evolution gateway you already run.

## How it works

```
[PIR sensor] -> [ESP8266] --WiFi--> [rserver:7575/api/doorbell] --Evolution API--> [your WhatsApp]
   motion!        POST w/ token        checks token, 3-min cooldown, sends "Someone's at the door!"
```

The server side is already live and tested end-to-end.

## Parts

- ESP8266 board (NodeMCU or Wemos D1 mini — you have this)
- HC-SR501 PIR motion sensor (~$2-3)
- Micro-USB cable + phone charger (to power the ESP near the door)
- 3x jumper wires (female-to-female)

## Wiring

| HC-SR501 | ESP8266 (NodeMCU / D1 mini) |
|----------|------------------------------|
| VCC      | VU (5V)                      |
| GND      | G                            |
| OUT      | D2 (GPIO4)                   |

PIR adjustment: turn the **time** pot fully counter-clockwise (shortest
retrigger delay) and **sensitivity** to roughly the middle.

## Flashing the ESP8266

1. Install Arduino IDE.
2. File → Preferences → add to "Additional Boards Manager URLs":
   `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. Tools → Board → Boards Manager → install **esp8266**.
4. Tools → Board → select **NodeMCU 1.0 (ESP-12E Module)**
   (or LOLIN(WEMOS) D1 R2 & mini if that's your board).
5. Open `doorbell-esp8266.ino`, fill in:
   - `WIFI_SSID` / `WIFI_PASS` — your home WiFi
   - `DOORBELL_TOKEN` — the token Muse sent you (keep it private)
6. Upload.

## Placement

- Mount the PIR facing the door, about 2m high, angled slightly down.
- Keep it out of direct sunlight and away from heat vents (they false-trigger PIRs).
- The ESP just needs WiFi + USB power — a hallway outlet works.

## Testing

Wave your hand in front of the sensor. The ESP's blue LED blinks while it
sends, and your WhatsApp should buzz within a few seconds:
"🔔 Someone's at the door! (Front door, 08:35 AM)".

For debugging, open the Arduino serial monitor at 115200 baud — the sketch
logs WiFi status and every server reply.

## Notes

- **Cooldown:** the server sends at most one WhatsApp per 3 minutes, so a
  lingering visitor won't spam you. (The ESP also debounces locally.)
- **Security:** the webhook needs the secret token; wrong token = rejected,
  nothing sent. The token lives only on your server and in your sketch.
- **Want more doors?** Flash a second ESP with a different `SENSOR_NAME`
  (e.g. "Back door") using the same token — the WhatsApp names the sensor.
- **Power draw** is tiny (~70mA); any old phone charger runs it 24/7.
