#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Structure of incoming ESP-NOW message
typedef struct struct_message {
  float value1;
  float value2;
} struct_message;

struct_message incomingData;

// MAC Addresses of Node 1 and Node 2
String node1MAC = "3C:8A:1F:0C:63:2C";
String node2MAC = "78:42:1C:6C:15:90";

// Variables to store sensor data
float temp1 = 0, hum1 = 0;
float temp2 = 0, pres2 = 0;

void setup() {
  Serial.begin(115200);

  // Initialize OLED
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

  // WiFi + ESP-NOW Setup
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  // Simulated ML prediction with ±0.3 random noise
  float noise1 = random(-30, 31) / 100.0;  // Node1 temp noise
  float noise2 = random(-30, 31) / 100.0;  // Node2 temp noise

  float predTemp1 = temp1 + noise1;
  float predTemp2 = temp2 + noise2;

  // OLED Display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("GATEWAY");

  display.setCursor(0, 10);
  display.print("T1 A: ");
  display.print(temp1, 1);
  display.print(" P: ");
  display.print(predTemp1, 1);

  display.setCursor(0, 20);
  display.print("H1: ");
  display.print(hum1, 1);
  display.print(" %");

  display.setCursor(0, 35);
  display.print("T2 A: ");
  display.print(temp2, 1);
  display.print(" P: ");
  display.print(predTemp2, 1);

  display.setCursor(0, 45);
  display.print("P2: ");
  display.print(pres2, 1);
  display.print(" hPa");

  display.display();

  // Serial Debug
  Serial.print("Node1 - Temp: ");
  Serial.print(temp1); Serial.print("C | Pred: ");
  Serial.print(predTemp1); Serial.print("C | Hum: ");
  Serial.println(hum1);

  Serial.print("Node2 - Temp: ");
  Serial.print(temp2); Serial.print("C | Pred: ");
  Serial.print(predTemp2); Serial.print("C | Press: ");
  Serial.println(pres2);

  delay(2000);
}

// ESP-NOW receive callback
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  String senderMAC = String(macStr);

  if (senderMAC == node1MAC) {
    temp1 = incomingData.value1;
    hum1 = incomingData.value2;
    Serial.println("Received from Node 1");
  } else if (senderMAC == node2MAC) {
    temp2 = incomingData.value1;
    pres2 = incomingData.value2;
    Serial.println("Received from Node 2");
  }
}
