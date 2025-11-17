#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#include "DHT.h"

// ====== CONFIG ======
#define WIFI_SSID "OPPO A5 5G x3b4"
#define WIFI_PASS "gbig4735"

#define DHTPIN 4
#define DHTTYPE DHT11
#define DEVICE_ID "envBot_01"
#define ENDPOINT_URL "https://prs-tech-project-portal/api/brain"

// ====== SETUP ======
DHT dht(DHTPIN, DHTTYPE);
unsigned long lastPost = 0;
const unsigned long postInterval = 5000;

void setup() {
  Serial.begin(115200);
  dht.begin();

  Serial.println("\n[WiFi] Connecting...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[WiFi] Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ====== LOOP ======
void loop() {
  if (millis() - lastPost >= postInterval) {
    lastPost = millis();

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("[DHT] Failed to read sensor data!");
      return;
    }

    Serial.printf("[DHT] Temp: %.2f °C | Hum: %.2f %%\n", temp, hum);

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;        // <-- required for ESP8266 HTTP
      HTTPClient http;

      http.begin(client, ENDPOINT_URL);  
      http.addHeader("Content-Type", "application/json");

      String payload = "{\"machineID\":\"" + String(DEVICE_ID) +
                       "\",\"data\":{\"temp\":" + String(temp, 2) +
                       ",\"hum\":" + String(hum, 2) + "}}";

      int code = http.POST(payload);

      if (code > 0) {
        Serial.printf("[HTTP] POST %d\n", code);
        String resp = http.getString();
        Serial.println(resp);
      } else {
        Serial.printf("[HTTP] Error: %s\n", http.errorToString(code).c_str());
      }

      http.end();
    } else {
      Serial.println("[WiFi] Disconnected, retrying...");
      WiFi.reconnect();
    }
  }
}