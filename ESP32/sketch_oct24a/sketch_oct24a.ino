// ISO 14443
#include <PN5180.h>
#include <PN5180ISO14443.h>
#include <SPI.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "time.h"
#include "config.h"
#include <WebServer.h>
#include <map>
#include <string>
using namespace std;

// Wifi
const char* ssid = "DD4A982.4";
const char* password = WIFI_PASSWORD;

// Define PN5180 pins
const int nssPin = 5;     // NSS pin
const int busyPin = 16;   // BUSY pin
const int resetPin = 17;  // RESET pin

// Define servo pins for lid
const int servoPin = 13;
int angle = 0;

Servo servo;

// Define servo pins for hopper
const int hopperServoPin = 27;
int hopperServoAngle = 0;

Servo hopperServo;

// Setup NTP server
const char* ntpServer = "pool.ntp.org";  // network time protocol
const long gmtOffset_sec = -18000;       // Eastern standard time
const int daylightOffset_sec = 3600;

// Initialize PN5180
PN5180ISO14443 nfcReader(nssPin, busyPin, resetPin);

WebServer server(80);

struct mealTime {
  int hour;
  int minute;
};

std::map<String, mealTime> mealTimes;
mealTime breakfastTime = mealTimes["breakfast"];
mealTime lunchTime = mealTimes["lunch"];
mealTime dinnerTime = mealTimes["dinner"];

mealTime parseTime(String time) {
  int indexOfColon = time.indexOf(':');
  int hour = time.substring(0, indexOfColon).toInt();
  int minute = time.substring(indexOfColon + 1).toInt();
  return { hour, minute };
}

void handleRoot() {

  if (server.args() > 0) {
    // Iterate through all arguments
    for (int i = 0; i < server.args(); i++) {
      String argName = server.argName(i);
      String argValue = server.arg(i);

      mealTime timeOfMeal = parseTime(argValue);
      mealTimes[argName] = timeOfMeal;

      breakfastTime = mealTimes["breakfast"];
      lunchTime = mealTimes["lunch"];
      dinnerTime = mealTimes["dinner"];
    }

    printMealTimes();

  } else {
    server.send(400, "text/plain", "No data received");
  }
}

void setup() {
  Serial.begin(115200);

  servo.attach(servoPin);
  hopperServo.attach(hopperServoPin);

  // Connect to Wifi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;

  server.on("/", handleRoot);
  server.begin();

  printMealTimes();

  // Initialize SPI
  SPI.begin();

  // Initialize the PN5180
  nfcReader.begin();

  // Reset and setup the PN5180 for ISO14443 mode
  nfcReader.reset();
  nfcReader.setupRF();  // Configures the RF settings for ISO14443

  Serial.println("PN5180 Initialized for ISO14443 (NFC)!");
}

void loop() {

  server.handleClient();

  uint8_t uid[10];                                    // Buffer for the UID
  uint8_t uidLength = nfcReader.readCardSerial(uid);  // Actual length of UID
  String tagID;

  nfcReader.reset();
  nfcReader.setupRF();

  //connectServer();

  struct tm timeinfo;

  time_t now = time(nullptr);
  struct tm* current = localtime(&now);

  if (getLocalTime(&timeinfo)) {
    for (auto const& m : mealTimes) {
      const String& mealName = m.first;
      const mealTime& mealTime = m.second;

      if(timeinfo.tm_hour == mealTime.hour && timeinfo.tm_min == mealTime.minute && (timeinfo.tm_sec >= 0 && timeinfo.tm_sec <= 8)){
        if (hopperServoAngle == 0) {
          hopperServoAngle = 90;
          delay(3000);
        } else {
          hopperServoAngle = 0;
          delay(1000);
        }
        hopperServo.write(hopperServoAngle);
      }else {
        //Serial.println("Not time yet");
      }
    }
  }else {
    Serial.println("Failed to obtain time");
  }

    // Check for an ISO14443 tag
    if (nfcReader.readCardSerial(uid)) {

      for (int i = 0; i < uidLength; i++) {
        tagID.concat(String(uid[i], HEX));
        tagID.toUpperCase();
      }

      if (tagID == "3DE1490") {
        if (angle == 90) {
          angle = 0;
        } else {
          angle = 90;
        }
        servo.write(angle);
      }
    }

    delay(1000);  // Wait for 1 second before scanning again
  }

  void printMealTimes() {
    Serial.print("Current meal times: ");
    for (auto const& m : mealTimes) {
      const String& mealName = m.first;
      const mealTime& time = m.second;
      Serial.print(time.hour);
      Serial.print(':');
      if (time.minute < 10) {
        Serial.print('0');
      }
      Serial.print(time.minute);
      Serial.print(" ");
    }
    Serial.println();
  }
