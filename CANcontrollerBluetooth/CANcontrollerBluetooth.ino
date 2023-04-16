// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <CAN.h>
#include <esp_now.h>
#include <WiFi.h>

#define BUTTON_PIN 33
#define THROTTLE_PIN 32
#define LED 2

uint8_t broadcastAddress[] = {0xE0, 0x5A, 0x1B, 0xA2, 0x60, 0xB4};

String success;

typedef struct struct_message {
    unsigned int id;
    uint8_t packet[8];
} struct_message;

struct_message outgoingReadings;

struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  // //Serial.print("Bytes received: ");
  // //Serial.println(len);
  //Serial.print("Incoming Value: ");
  //Serial.println(incomingReadings.packet[0],HEX);

  //rebroadcast to CAN
  CAN.beginPacket(incomingReadings.id);
  for(int i = 0; i < 8; i ++){
    CAN.write(incomingReadings.packet[i]);    
  }
  CAN.endPacket();
  //Serial.println("Packet Sent - Data Read Request");  
}

void setup() {
  //Serial.begin(115200);
  // while (!//Serial);
  //Serial.println("Bluetooh CAN");
  pinMode(LED,OUTPUT);
  // set wifi mode
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    //Serial.println("Starting CAN failed!");
    while (1);
  }
}

long last_time = millis();
bool last_state = false;
void loop() {
  // char //SerialData = //Serial.read();
  // if(//SerialData == '1'){
  //   readData(1,0x60F6,0x01);
  // }
  if(millis()-last_time > 500){
    last_time = millis();
    if(last_state){
      digitalWrite(LED,HIGH);
    }else{
      digitalWrite(LED,LOW);
    }
    last_state = !last_state;
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
  //Serial.println("Packet Sent - Data Read Request");
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
  //Serial.println("Packet Sent - Torque set");
}

void recieveData() {
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // received a packet
    //Serial.print("Received ");

    if (CAN.packetExtended()) {
      //Serial.print("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      //Serial.print("RTR ");
    }

    //Serial.print("packet with id 0x");
    outgoingReadings.id = CAN.packetId();
    //Serial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
      //Serial.print(" and requested length ");
      //Serial.println(CAN.packetDlc());
    } else {
      //Serial.print(" and length ");
      //Serial.println(packetSize);
      
      int i = 0;
      // only print packet data for non-RTR packets
      while (CAN.available()) {
        outgoingReadings.packet[i] = CAN.read();
        //Serial.println(outgoingReadings.packet[i],HEX);
        i++;
      }
      //Serial.println();

      // broadcast packet
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
    
      if (result == ESP_OK) {
        //Serial.println("Sent with success");
      }
      else {
        //Serial.println("Error sending the data");
      }  

    }

    //Serial.println();
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
  //Serial.println("Packet Sent - Set mode Pre-operational");
}
void setOperational(int id){
  CAN.beginPacket(0x000);
  CAN.write(0x01);
  CAN.write(id);
  CAN.endPacket();
  //Serial.println("Packet Sent - Set mode Operational");
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
  //Serial.println("Packet Sent - Control Word 6");
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
  //Serial.println("Packet Sent - Control Word 15");
}