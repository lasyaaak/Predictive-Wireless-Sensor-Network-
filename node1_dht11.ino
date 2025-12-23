#include <WiFi.h>
#include <esp_now.h>
#include "DHT.h"

#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

uint8_t gatewayAddress[] = {0x00, 0x4B, 0x12, 0x34, 0x20, 0x78};

typedef struct struct_message {
  float temp;
  float hum;
  char sender[10];
} struct_message;

struct_message data;

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);
}

void loop() {
  data.temp = dht.readTemperature();
  data.hum = dht.readHumidity();
  strcpy(data.sender, "Node1");

  esp_err_t result = esp_now_send(gatewayAddress, (uint8_t*)&data, sizeof(data));
  if (result == ESP_OK) {
    Serial.println("Sent successfully");
  } else {
    Serial.println("Send failed");
  }
  delay(5000);
}
