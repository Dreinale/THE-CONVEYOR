#include <M5Stack.h>
#include <Wire.h>
#include <driver/rmt.h>
#include <math.h>

#include "GoPlus2.h"
#include "MFRC522_I2C.h"
#include "Module_GRBL_13.2.h"

#define STEPMOTOR_I2C_ADDR 0x70
Module_GRBL _GRBL = Module_GRBL(STEPMOTOR_I2C_ADDR);
int speed = 100;

GoPlus2 goPlus;

MFRC522 mfrc522(0x28);

void header() {
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.lcd.setTextSize(4);
    M5.lcd.setBrightness(100);
    M5.Lcd.setCursor(80, 5);
    M5.Lcd.println("Conveyor");
}

void body() {
    //M5.Lcd.setCursor(5, 50);
    M5.lcd.setTextSize(2);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.println("UID:");
}

void setup() {
    M5.begin();
    M5.Power.begin();

    //Titre
    header();

    Wire.begin(21, 22);
    _GRBL.Init(&Wire);
    Serial.begin(115200);

    body();

    mfrc522.PCD_Init();

    _GRBL.setMode("absolute");
}

void set_NFC () {
    M5.Lcd.setCursor(40, 37);
    if (!mfrc522.PICC_IsNewCardPresent() ||
        !mfrc522.PICC_ReadCardSerial()) {
        Serial.println("No new cards detected");
        delay(200);
        return;
    }
    M5.Lcd.fillRect(42, 47, 320, 20, BLACK);
}

char read_NFC () {
    set_NFC();
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        M5.Lcd.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        M5.Lcd.print(mfrc522.uid.uidByte[i], HEX);
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    M5.Lcd.println("");

    Serial.print("UID lu: ");
    Serial.println(uid);

    char destination = 'B';
    if (uid == "95d36c2") {
        destination = 'C';
    } else if (uid == "c6b2592b") {
        destination = 'D';
    }

    Serial.print("Destination set for UID: ");
    Serial.println(destination);
    return destination;
}

void change_direction(char destination) {
   Serial.println("Change of direction of the servomotor...");
  // Control the SG90 servo motor based on destination
    if (destination == 'B') {
        goPlus.Servo_write_angle(SERVO_NUM0, 0); // Point B
    } else if (destination == 'C') {
        goPlus.Servo_write_angle(SERVO_NUM0, 90); // Point C
    } else if (destination == 'D') {
        goPlus.Servo_write_angle(SERVO_NUM0, 180); // Point D
    }
}

void step_motor() {
  if (M5.BtnA.wasPressed()) {
        M5.Lcd.println("A");
        Serial.println("Button A pressed");
        Serial.print(_GRBL.readStatus());
        _GRBL.setMotor(5, 0, 0, speed);
    }

    // If Button B was pressed, speed of the motor will increase
    if (M5.BtnB.wasPressed()) {
      M5.Lcd.println("B");
      Serial.println("Button B pressed");
      if (speed == 100) {
        speed = 200;
      } else if (speed == 200) {
        speed = 400;
      } else if (speed == 400) {
        speed = 100;
      }
    }
    
    // If Button C was pressed, reset the motor to X = 0 & stop all.
    if (M5.BtnC.wasReleased()) {
        M5.Lcd.println("C");
        Serial.println("Button C pressed");
        _GRBL.unLock();
        _GRBL.setMotor(0, 0, 0, speed);
    }
}

void loop() {
    step_motor();

    char destination = read_NFC();

    change_direction(destination);

    M5.update();
}
