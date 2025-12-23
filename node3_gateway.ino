#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

typedef struct struct_message {
  float value1;
  float value2;
} struct_message;

struct_message incomingData;

String node1MAC = "3C:8A:1F:0C:63:2C";
String node2MAC = "78:42:1C:6C:15:90";

float temp1 = 0, hum1 = 0;
float temp2 = 0, pres2 = 0;

void setup() {
  Serial.begin(115200);

  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Gateway");
  display.display();

  // WiFi and ESP-NOW Initialization
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Gateway");

  display.setCursor(0, 16);
  display.print("T1: ");
  display.print(temp1);
  display.print(" C");

  display.setCursor(0, 26);
  display.print("H1: ");
  display.print(hum1);
  display.print(" %");

  display.setCursor(0, 42);
  display.print("T2: ");
  display.print(temp2);
  display.print(" C");

  display.setCursor(0, 52);
  display.print("P2: ");
  display.print(pres2);
  display.print(" hPa");

  display.display();
  delay(1000);
}

// Updated callback signature: esp_now_recv_info_t pointer first parameter
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);
  Serial.print("Received from: ");
  Serial.println(macStr);

  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  String senderMAC = String(macStr);

  if (senderMAC == node1MAC) {
    temp1 = incomingData.value1;
    hum1 = incomingData.value2;
    Serial.print("Node1 Temp: ");
    Serial.print(temp1);
    Serial.print(" C, Humidity: ");
    Serial.print(hum1);
    Serial.println(" %");
  } else if (senderMAC == node2MAC) {
    temp2 = incomingData.value1;
    pres2 = incomingData.value2;
    Serial.print("Node2 Temp: ");
    Serial.print(temp2);
    Serial.print(" C, Pressure: ");
    Serial.print(pres2);
    Serial.println(" hPa");
  }
}
