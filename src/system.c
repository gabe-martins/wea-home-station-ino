#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include <math.h>
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define DHT_PIN 10

const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";
const char* API_URL = ""; 
const char* API_KEY = "";

DHT dht(DHT_PIN, DHT22);
RTC_DS1307 rtc;

// Initialize WiFi
void initWiFi() {
  Serial.print("Connecting to WiFi.");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(100);
  }
  Serial.println(WiFi.localIP());
  Serial.println("Connected!");
}

void initClock() {
  Wire.begin(5, 6); // SDA, SCL

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // Uncomment the next line to set the date and time to the moment the sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  initWiFi();
  initClock();
}

void loop() {
  // read dht values
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    float humidity = -999;
    float temperature = -999;
  }

  // read ds1307 sensor
  DateTime now = rtc.now();
  char buffer[20];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());

  // Construir o objeto JSON
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["date_time"] = buffer;

  String jsonData;
  serializeJson(jsonDoc, jsonData);

  Serial.println(jsonData);


  // Enviar dados para o servidor
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(API_URL);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", API_KEY); // Adicionar cabeçalho de Authorization

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Erro na requisição: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Erro de conexão WiFi");
  }

  delay(60000);
}