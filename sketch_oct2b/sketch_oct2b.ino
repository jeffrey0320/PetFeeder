#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
//#include <DS3231.h>
//#include <Wire.h>
#include <RTClib.h>

#define RST_PIN 9
#define SS_PIN 10

int servoPin = 7;
Servo servo1;
int angle = 0;

int lidServoPin = 6;
Servo lidServo;
int lidServoAngle = 0;

byte readCard[4];
String MasterTag = "2496963F";
String tagID = "2496963F";

//DS3231 myRTC;

MFRC522 mfrc522(SS_PIN, RST_PIN);

RTC_DS3231 rtc;

byte year = '2024';
bool century = false;
bool h12Flag = true;
bool pmFlag = true;

char t[32];

void setup() {
  SPI.begin();
  mfrc522.PCD_Init();

  servo1.attach(servoPin);
  servo1.write(angle);

  lidServo.attach(lidServoPin);
  lidServo.write(lidServoAngle);

  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  //myRTC.setClockMode(true);

  //myRTC.setYear(24);
  //myRTC.setMonth(10);
  //myRTC.setDate(7);
  //myRTC.setDoW(1);
  //myRTC.setHour(6);
  //myRTC.setMinute(26);
  //myRTC.setSecond(30);
}

void loop() {


  while (getID()) {

    //Serial.print(myRTC.getYear(), DEC);
    //Serial.print('/');
    //Serial.print(myRTC.getMonth(century), DEC);
    //Serial.print('/');
    //Serial.print(myRTC.getDoW(), DEC);
    //Serial.print(' ');
    //Serial.print(myRTC.getHour(h12Flag, pmFlag), DEC);
    //Serial.print(':');
    //Serial.print(myRTC.getMinute(), DEC);
    //Serial.print(':');
    //Serial.print(myRTC.getSecond(), DEC);
    //Serial.println();
    if (tagID == MasterTag) {
      if (angle == 0) {
        angle = 90;
      } else if (angle == 90) {
        angle = 0;
      }
      servo1.write(angle);
    }
  }
  DateTime now = rtc.now();
  //Serial.println(String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL));
  sprintf(t, "%02d:%02d:%02d %02d/%02d/%02d", now.hour(), now.minute(), now.second(), now.month(), now.day(), now.year());
  Serial.print(F("Date/Time: "));
  Serial.println(t);
  if (now.hour() == 11 && now.minute() == 19 && (now.second() >= 0 && now.second() <= 5) && now.year() == 2024 && now.month() == 10 && now.day() == 25) {
    if (lidServoAngle == 0) {
      lidServoAngle = 90;
      delay(1000);
      delay(3000);
    } else if (lidServoAngle == 90) {
      lidServoAngle = 0;
      delay(1000);
    }
    lidServo.write(lidServoAngle);
  } else {
    Serial.println("NOT TIME YET");
  }
  
  delay(1000);
}

boolean getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  tagID = "";

  for (uint8_t i = 0; i < 4; i++) {
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();
  return true;
}
