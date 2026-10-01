/*
 * JakinsCraft WhatsApp Doorbell — ESP8266 + HC-SR501 PIR motion sensor
 *
 * When motion is detected at the door, this posts to the server-monitor
 * agent on rserver, which relays it to your WhatsApp via the Evolution API.
 *
 * Wiring (NodeMCU / Wemos D1 mini):
 *   HC-SR501 VCC -> VU (5V) pin
 *   HC-SR501 GND -> G
 *   HC-SR501 OUT -> D2 (GPIO4)
 *   ESP8266 powered via USB
 *
 * PIR tip: turn the "time" pot fully counter-clockwise (shortest retrigger)
 * and "sensitivity" to about middle. The server enforces a 3-minute cooldown
 * between WhatsApp alerts so one visitor can't spam you.
 */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// ---- Fill these in ----
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL = "http://192.168.2.139:7575/api/doorbell";
const char* DOORBELL_TOKEN = "PASTE_TOKEN_HERE";  // Muse sends this separately
const char* SENSOR_NAME = "Front door";
// -----------------------

const int PIR_PIN = D2;  // HC-SR501 OUT
const unsigned long DEBOUNCE_MS = 5000;  // ignore flicker right after a trigger

int lastPirState = LOW;
unsigned long lastTrigger = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // LED off (active-low)

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected, IP: " + WiFi.localIP().toString());
  Serial.println("Doorbell armed. Wave at the PIR to test.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, reconnecting...");
    WiFi.reconnect();
    delay(2000);
    return;
  }

  int pir = digitalRead(PIR_PIN);
  unsigned long now = millis();

  // Trigger on the rising edge only, with debounce
  if (pir == HIGH && lastPirState == LOW && (now - lastTrigger > DEBOUNCE_MS)) {
    lastTrigger = now;
    ringDoorbell();
  }
  lastPirState = pir;
  delay(100);
}

void ringDoorbell() {
  Serial.println("Motion detected -> ringing doorbell");
  digitalWrite(LED_BUILTIN, LOW);  // LED on while sending

  WiFiClient client;
  HTTPClient http;
  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  String body = String("{\"token\":\"") + DOORBELL_TOKEN +
                "\",\"sensor\":\"" + SENSOR_NAME + "\"}";
  int code = http.POST(body);
  Serial.printf("Server replied: %d\n", code);
  if (code > 0) {
    Serial.println(http.getString());
  } else {
    Serial.printf("POST failed: %s\n", http.errorToString(code).c_str());
  }
  http.end();
  digitalWrite(LED_BUILTIN, HIGH);  // LED off
}
