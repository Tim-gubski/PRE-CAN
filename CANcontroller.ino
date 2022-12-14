// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <CAN.h>
#define BUTTON_PIN 33
#define THROTTLE_PIN 32

int currentState;
int torque;
long lastTime = millis();
long lastTorqueTime = millis();

bool driveEnabled = false;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Receiver Callback");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

}

void loop() {
  long currentTime = millis();
  currentState = digitalRead(BUTTON_PIN);
  char serialData = Serial.read();
  if(currentTime-lastTime > 500 && serialData == '1'){
    // readData(1,0x60F6,0x01);
    enableDriveMode(1);
    driveEnabled = true;
    lastTime = currentTime;
  }
  if(currentTime-lastTime > 500 && serialData == '2'){
    // readData(1,0x60F6,0x01);
    disableDriveMode(1);
    driveEnabled = false;
    lastTime = currentTime;
  }
  if(currentTime-lastTime > 500 && serialData == '3'){
    // readData(1,0x60F6,0x01);
    setTorque(1,80);
    lastTime = currentTime;
  }
  if(currentTime-lastTime > 500 && serialData == '4'){
    // readData(1,0x60F6,0x01);
    setTorque(1,0);
    lastTime = currentTime;
  }

  if(driveEnabled && currentTime - lastTorqueTime > 100){
    int analogValue = analogRead(THROTTLE_PIN);
    torque = map(analogValue, 0, 4095, 100, 0);
    setTorque(1,torque);
    lastTorqueTime = currentTime;
  }

  recieveData();
}

void readData(int id, unsigned int address, unsigned int sub_index){
  byte b1 = (address >> 8);
  byte b2 = (address >> 0);
  CAN.beginPacket(0x600+id);
  CAN.write(0x40);
  // info address little endian
  CAN.write(b2);
  CAN.write(b1);
  CAN.write(sub_index);
  // zeroes for padding
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
  Serial.println("Packet Sent - Data Read Request");
}

void setTorque(int id, unsigned long velocity){
  byte b1 = (velocity >> 0);
  CAN.beginPacket(0x600+id);
  CAN.write(0x2B);
  CAN.write(0x71);
  CAN.write(0x60);
  CAN.write(0x00);
  CAN.write(b1);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
  Serial.println("Packet Sent - Torque set");
}

void recieveData() {
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // received a packet
    Serial.print("Received ");

    if (CAN.packetExtended()) {
      Serial.print("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      Serial.print("RTR ");
    }

    Serial.print("packet with id 0x");
    Serial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
      Serial.print(" and requested length ");
      Serial.println(CAN.packetDlc());
    } else {
      Serial.print(" and length ");
      Serial.println(packetSize);

      // only print packet data for non-RTR packets
      while (CAN.available()) {
        Serial.print(CAN.read(),HEX);
      }
      Serial.println();
    }

    Serial.println();
  }
}

void enableDriveMode(int id){
  setPreOperational(id);
  delay(100);
  setOperational(id);
  delay(100);
  controlword6(id);
  delay(100);
  controlword15(id);
  delay(100);
}

void disableDriveMode(int id){
  controlword6(id);
  delay(100);
  setPreOperational(id);
  delay(100);
}

void setPreOperational(int id){
  CAN.beginPacket(0x000);
  CAN.write(0x03);
  CAN.write(id);
  CAN.endPacket();
  Serial.println("Packet Sent - Set mode Pre-operational");
}
void setOperational(int id){
  CAN.beginPacket(0x000);
  CAN.write(0x01);
  CAN.write(id);
  CAN.endPacket();
  Serial.println("Packet Sent - Set mode Operational");
}
void controlword6(int id){
  CAN.beginPacket(0x600+id);
  CAN.write(0x2B);
  CAN.write(0x40);
  CAN.write(0x60);
  CAN.write(0x00);
  CAN.write(0x06);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
  Serial.println("Packet Sent - Control Word 6");
}
void controlword15(int id){
  CAN.beginPacket(0x600+id);
  CAN.write(0x2B);
  CAN.write(0x40);
  CAN.write(0x60);
  CAN.write(0x00);
  CAN.write(0x0F);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
  Serial.println("Packet Sent - Control Word 15");
}