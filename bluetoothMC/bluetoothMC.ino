#include <esp_now.h>
#include <WiFi.h>

// uint8_t broadcastAddress[] = {0xE0, 0x5A, 0x1B, 0xA2, 0x60, 0xB4}; // enable for board node 3
uint8_t broadcastAddress[] = {0xC8, 0xF0, 0x9E, 0x9F, 0x72, 0xEC}; // enable for board node 3

uint8_t testPacket[] = {0x40, 0xF6, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00};

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
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
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
  // Serial.print("Bytes received: ");
  // Serial.println(len);
  Serial.print("Incoming Packet ID: ");
  Serial.println(incomingReadings.id,HEX);
  Serial.println("Incoming Packet: ");
  for(int i = 0; i < 8; i++){
    Serial.println(incomingReadings.packet[i],HEX);
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
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
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
int last_time = millis();

void loop() {
  // if(millis() > last_time+1000){
  //   // Set values to send
  //   // outgoingReadings.num = analogRead(36);
  //   outgoingReadings.packet[0] = 0x69;  

  //   // Send message via ESP-NOW
  //   esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
    
  //   if (result == ESP_OK) {
  //     Serial.println("Sent with success");
  //   }
  //   else {
  //     Serial.println("Error sending the data");
  //   }
  //   last_time = millis();
  // }

  char serialData = Serial.read();
  if(serialData == '1'){ 
    outgoingReadings.id = 0x601;
    Serial.print("ID: 0x");
    Serial.println(outgoingReadings.id,HEX);
    Serial.println("Packet: ");
    for(int i = 0; i<8; i++){
      outgoingReadings.packet[i] = testPacket[i];
      Serial.print("0x"); 
      Serial.println(testPacket[i],HEX); 
    }

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }    
      
  }
}
