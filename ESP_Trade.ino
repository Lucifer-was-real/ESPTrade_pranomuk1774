#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

const char* ssid = "PicoTest";
const char* password = "importConnect";
const char* serverURL = "http://192.168.240.139:5000/api/trade?symbol=AMZN";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String stockSymbol = "AMZN";
float closePrice = 0.0;
float highPrice = 0.0;
float lowPrice = 0.0;
float openPrice = 0.0;
String pattern = "Unknown";
String signal = "HOLD";

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  WiFi.begin(ssid, password);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Attempting WiFi connection...");
  }
  Serial.println("WiFi connected");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.display();
  delay(2000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, serverURL);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("API Response: " + response);

      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);
      if (!error) {
        stockSymbol = "AMZN";
        closePrice = doc["close"];
        highPrice = doc["high"];
        lowPrice = doc["low"];
        openPrice = doc["open"];
        pattern = doc["pattern"].as<String>();
        signal = doc["signal"].as<String>();

        displayFirstScreen();
        delay(5000);
        displaySecondScreen();
        delay(5000);
        displaySignalScreen();
        delay(5000);
      } else {
        Serial.println("JSON Parsing Error: " + String(error.c_str()));
      }
    } else {
      Serial.println("HTTP Request failed");
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void displayFirstScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Symbol: ");
  display.println(stockSymbol);
  display.setCursor(0, 10);
  display.print("Close: $");
  display.println(closePrice, 2);
  display.setCursor(0, 20);
  display.print("High: $");
  display.println(highPrice, 2);
  display.display();
}

void displaySecondScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Low: $");
  display.println(lowPrice, 2);
  display.setCursor(0, 10);
  display.print("Open: $");
  display.println(openPrice, 2);
  display.setCursor(0, 20);
  display.print("Pattern: ");
  display.println(pattern);
  display.display();
}

void displaySignalScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Signal: ");
  display.println(signal);
  display.display();
}
