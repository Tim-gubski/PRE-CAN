#include <CAN.h>
#define LED 2

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);

  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("CAN Receiver Callback");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1)
      ;
  }
  Serial.println("CAN STARTED");
}

void loop() {
  char serialData = Serial.read();
  // put your main code here, to run repeatedly:
  // readData(2, 0x60F6, 0x01);
  // sleep(1);
  recieveData();
  // if (serialData == '0') {
  //   Serial.println("tring to send");
  //   readData(2, 0x60F6, 0x01);
  // }
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
        Serial.print(CAN.read(), HEX);
      }
      Serial.println();
    }

    Serial.println();
  }
}

void readData(int id, unsigned int address, unsigned int sub_index) {
  byte b1 = (address >> 8);
  byte b2 = (address >> 0);
  CAN.beginPacket(0x600 + id);
  Serial.println("begin");
  CAN.write(0x40);
  Serial.println("writing");
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


void setPreOperational(int id) {
  CAN.beginPacket(0x000);
  CAN.write(0x03);
  CAN.write(id);
  CAN.endPacket();
  Serial.println("Packet Sent - Set mode Pre-operational");
}