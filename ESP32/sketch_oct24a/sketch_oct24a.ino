
/*
#include <PN5180.h>
#include <PN5180ISO15693.h>
#include <SPI.h>

const byte nssPin = 5;
const byte busyPin = 16;
const byte resetPin = 17;

PN5180ISO15693 abc(5, 16, 17);

uint8_t previousUid[8];

//string tagID;
//uint8_t uidLength;
//uint8_t uid[10];

void setup() {
  
  Serial.begin(115200);
  SPI.begin();
  abc.begin();
  abc.reset();
  uint8_t productVersion[2];
  abc.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  abc.setupRF();

  Serial.println(F("PN5180 WORKING!"));
}

void loop() {

  uint8_t thisUid[8];

  ISO15693ErrorCode rc = abc.getInventory(thisUid);

  Serial.println(rc);
  abc.reset();
  abc.setupRF();

  if (rc == ISO15693_EC_OK) {
    Serial.print(F("ID: "));
    for (int j = 0; j < sizeof(thisUid); j++) {
      Serial.print(thisUid[j], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }else {
    Serial.println(F("No tag detected or an error occured."));
  }
  delay(1000);
  /*
  if(!abc.isCardPresent()){
    return;
  }
  uidLength = abc.readCardSerial(uid);
  Serial.println(uidLength);
  if (memcmp(uid, previousUid, uidLength) == 0) {
    return;
  }
  if (uidLength > 0) {
    Serial.print(abc.readCardSerial(uid));
    printHEX(uid, uidLength);
    // Update the previous UID
    memcpy(previousUid, uid, uidLength);
    return;
  }
}*/

/*
#include <PN5180ISO15693.h>

const byte nssPin = 5;      // Chip Select (NSS) pin
const byte busyPin = 16;    // BUSY pin
const byte resetPin = 17;   // RESET pin

PN5180ISO15693 nfcReader(nssPin, busyPin, resetPin);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting PN5180 Reader...");

  // Initialize the PN5180 module
  nfcReader.begin();  
  nfcReader.reset();
  
  nfcReader.setupRF();  // Set up RF configurations
}

void loop() {
  Serial.println("Scanning for tag...");

  uint8_t uid[8];  // Buffer for UID (Unique Identifier)
  ISO15693ErrorCode result = nfcReader.getInventory(uid);  // Attempt to read tag

  nfcReader.reset();
  nfcReader.setupRF();

  if (result == ISO15693_EC_OK) {  // Check if a tag was successfully read
    Serial.print("Tag detected with UID: ");
    for (int i = 0; i < sizeof(uid); i++) {
      Serial.print(uid[i], HEX);  // Print each byte of the UID in HEX
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("No tag detected.");
  }

  delay(1000);  // Wait a bit before trying again
}*/

// ISO 14443
#include <PN5180.h>
#include <PN5180ISO14443.h>
#include <SPI.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "time.h"
#include "config.h"
#include <WebServer.h>
#include <esp_task_wdt.h>

// Wifi
const char* ssid = "DD4A982.4";
const char* password = "ANUBIS032097!";

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

/*
void handleRoot() {
  if (server.args() > 0) {
    //String receivedData = server.arg("plain");
    //Serial.println("Received Data: " + receivedData);
    // Send an HTML page with the received data
    String html = "<!DOCTYPE html><html><head><title>ESP32 Server</title></head><body>";
    html += "<ul>";
    // Iterate through all arguments
    for (int i = 0; i < server.args(); i++) {
      String argName = server.argName(i);                    // Get argument name
      String argValue = server.arg(i);                       // Get argument value
      Serial.println("Arg: " + argName + " = " + argValue);  // Print to serial monitor

      // Add argument to HTML response
      html += "<li>" + argName + ": " + argValue + "</li>";
    }

    html += "</ul>";
    html += "<a href=\"/\">Refresh</a>";  // Add a refresh link
    html += "</body></html>";
    server.send(200, "text/html", html);
  } else {
    server.send(400, "text/plain", "No data received");
  }
}*/

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

  //server.on("/", handleRoot);
  server.begin();

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

  if (getLocalTime(&timeinfo)) {
    /*Serial.printf("Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);*/
    if (timeinfo.tm_hour == 16 && timeinfo.tm_min == 29 && (timeinfo.tm_sec >= 0 && timeinfo.tm_sec <= 8) && timeinfo.tm_year + 1900 == 2024 && timeinfo.tm_mon == 10 && timeinfo.tm_mday == 11) {
      if (hopperServoAngle == 0) {
        hopperServoAngle = 90;
        delay(3000);
      } else {
        hopperServoAngle = 0;
        delay(1000);
      }
      hopperServo.write(hopperServoAngle);
    } else {
      //Serial.println("Not time yet");
    }

  } else {
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
