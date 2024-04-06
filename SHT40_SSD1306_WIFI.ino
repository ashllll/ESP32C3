#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SHT4x.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <esp_system.h> // 引入ESP系统功能，如果需要显示更多系统信息


// 可配置的宏定义
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDRESS 0x3C // OLED的I2C地址
#define SERIAL_BAUD_RATE 115200 // 串口波特率
#define SHT4x_I2C_ADDRESS 0x44 // SHT4x传感器的I2C地址 (默认地址)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

const char* ssid = "llll";
const char* password = "ashllll1280!!";

// 将Web服务器定义为全局变量
AsyncWebServer server(8082);

// 存储温湿度数据的全局变量
sensors_event_t temp, humidity;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Wire.begin(4, 5); // 指定I2C引脚

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        return;
    }
    display.clearDisplay();

    if (!sht4.begin()) {
        Serial.println("Couldn't find SHT4x sensor!");
        while (1) delay(10);
    }

    WiFi.begin(ssid, password);
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (millis() - startTime > 15000) { // 15秒超时
            Serial.println("Failed to connect to WiFi. Please check your settings.");
            return;
        }
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());



     // 配置/data路由处理函数
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    sensors_event_t humidityEvent, tempEvent;
    sht4.getEvent(&humidityEvent, &tempEvent); // 获取最新的数据

    // 创建响应对象，并添加CORS头
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    String json = "{\"temperature\":" + String(tempEvent.temperature, 2) + ", \"humidity\":" + String(humidityEvent.relative_humidity, 2) + "}";
    response->print(json);
    request->send(response);
  });

    server.begin();
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 10000) { // 每2秒更新一次
        lastUpdate = millis();

        if (sht4.getEvent(&humidity, &temp)) {
            // 显示WiFi状态、IP地址和自由堆内存大小
            display.clearDisplay();
            display.setTextSize(1);      // 文本大小
            display.setTextColor(SSD1306_WHITE); // 文本颜色
            display.setCursor(0,0);     // 文本起始位置
            display.println("WiFi Connected");
            display.print("IP: ");
            display.println(WiFi.localIP());
            display.print("Heap: ");
            display.print(ESP.getFreeHeap()); // 显示ESP32的自由堆内存
            display.println(" bytes");           
            

            // 显示温度和湿度数据
            display.printf("Temp: %.2f C\n", temp.temperature);
            display.printf("Humid: %.2f%%\n", humidity.relative_humidity);

            display.display(); // 确保调用display()来更新显示内容
        } else {
            Serial.println("Failed to get SHT4x data.");
        }
    }
}