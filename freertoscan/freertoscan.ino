#include "driver/can.h"
#include <stdio.h>

// Define CAN configuration settings
can_general_config_t general_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, CAN_MODE_NORMAL);
can_timing_config_t timing_config = CAN_TIMING_CONFIG_500KBITS();
can_filter_config_t filter_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

void can_receive_task(void *arg) {
    can_message_t rx_msg;
    
    while (true) {
        // Wait for a CAN message indefinitely (portMAX_DELAY)
        if (can_receive(&rx_msg, portMAX_DELAY) == ESP_OK) {
            Serial.println("Message received!");
            Serial.print("ID: ");
            Serial.println(rx_msg.identifier, HEX);
            Serial.print("Data: ");
            for (int i = 0; i < rx_msg.data_length_code; i++) {
                Serial.print(rx_msg.data[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Install CAN driver
    if (can_driver_install(&general_config, &timing_config, &filter_config) == ESP_OK) {
        Serial.println("CAN driver installed");
    } else {
        Serial.println("Failed to install CAN driver");
        return;
    }

    // Start the CAN driver
    if (can_start() == ESP_OK) {
        Serial.println("CAN driver started");
    } else {
        Serial.println("Failed to start CAN driver");
        return;
    }

    // Create a FreeRTOS task for CAN reception
    xTaskCreatePinnedToCore(can_receive_task, "CAN Receive Task", 4096, NULL, 1, NULL, 1);
}

void loop() {
    // Nothing required in the loop; CAN messages are handled in the FreeRTOS task
}
