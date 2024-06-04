/**
   PostHTTPClient.ino

    Created on: 21.11.2016

*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

// This is the free api I'm using to get the time
// Notice how I'm using http because I don't want to deal with certificates
#define SERVER_IP "http://worldtimeapi.org/api/timezone/Europe/Berlin"

#ifndef STASSID
#define STASSID "YOUR-SSID"
#define STAPSK "YOUR-WIFI-PASSWORD"
#endif

#define RISE_HOUR 8
#define RISE_MINUTE 45

#define SERVO_PIN 13
#define FLIP_ANGLE 170
#define REST_ANGLE 90

Servo switchServo; 

int lastPerformedDayOfYear = -1; 

void clickSwitch() {
  switchServo.write(FLIP_ANGLE);
  delay(250);
  switchServo.write(REST_ANGLE);
}

void setup() {

  Serial.begin(115200);
  switchServo.attach(SERVO_PIN);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;
    JsonDocument doc;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, SERVER_IP);  // HTTP
    http.addHeader("Content-Type", "application/json");

    // start connection and send HTTP header and body
    //int httpCode = http.POST("{\"hello\":\"world\"}");
    int httpCode = http.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      const String& payload = http.getString();
      if (httpCode == HTTP_CODE_OK) {
        deserializeJson(doc, payload);
        String datetime = doc["datetime"];
        int hour = datetime.substring(11, 14).toInt();
        int minute = datetime.substring(14, 17).toInt();
        int dayOfYear = doc["day_of_year"];
        Serial.printf("CURRENT TIME: %02d:%02d\n", hour, minute);

        if (hour == RISE_HOUR && minute == RISE_MINUTE && dayOfYear != lastPerformedDayOfYear) {
          Serial.println("RISE AND SHINE: OPENING WINDOW...");
          clickSwitch();
          lastPerformedDayOfYear = dayOfYear;
        }
      } else {
        Serial.println("received error:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  // Blink light to let people know it's working
  digitalWrite(LED_BUILTIN, LOW); // Turn light on
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH); // Turn light off
  delay(9000);
}
