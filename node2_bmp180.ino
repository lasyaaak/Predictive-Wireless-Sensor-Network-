#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <esp_now.h>

Adafruit_BMP085 bmp;

struct SensorData {
  float temp;
  float pressure;
};

uint8_t gatewayAddress[] = {0x00, 0x4B, 0x12, 0x34, 0x20, 0x78}; // Gateway MAC

void setup() {
  Serial.begin(115200);

  if (!bmp.begin()) {
    Serial.println("BMP180 not detected. Check wiring.");
    while (1);
  }

  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("ESP-NOW Initialized");
}

void loop() {
  SensorData dataToSend;
  dataToSend.temp = bmp.readTemperature();
  dataToSend.pressure = bmp.readPressure() / 100.0;  // Convert Pa to hPa

  esp_err_t result = esp_now_send(gatewayAddress, (uint8_t *)&dataToSend, sizeof(dataToSend));

  if (result == ESP_OK) {
    Serial.print("Sent Temp: ");
    Serial.print(dataToSend.temp);
    Serial.print(" C, Pressure: ");
    Serial.print(dataToSend.pressure);
    Serial.println(" hPa");
  } else {
    Serial.println("Error sending data");
  }

  delay(3000);
}


