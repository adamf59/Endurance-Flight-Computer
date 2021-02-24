/**
 * JagSat Flight Control Program
 * Written by Adam Frank
 * 2/21/2021
 * 
 * #define FLIGHT_MODE to configure the software for flight mode
 */

#include <Arduino.h>
#include <EEPROM.h>

#include "jsfc.h"
#include "MCUHardwareMap.h"
#include "FlightData.h"
#include "Communications.h"



int main() {

    // Initialize the AVR Board:
    init();

    // Perform any startup setup:
    
    // GroundLink Serial
    Serial.begin(9600);

    pinMode(_HW_PIN_STATUS_INDICATOR_LED, OUTPUT);
    pinMode(_HW_PIN_BALLAST_TRIGGER, OUTPUT);
    pinMode(_HW_PIN_IRIDIUM_MODEM_SLEEP, OUTPUT);

    for (int i = 0; i < 5; i++) {
        digitalWrite(_HW_PIN_STATUS_INDICATOR_LED, true);
        delay(150);
        digitalWrite(_HW_PIN_STATUS_INDICATOR_LED, false);
        delay(150);
    }

    Serial.println("1"); // Send "startup complete" to GL
    // Perform Crash Check
    // Crash byte stored at address 0, and is SET at 0xFF.

#ifdef FLIGHT_MODE
        if (EEPROM.read(0x00) == 0xFF) {
            
            // TODO crash logic goes here.

        } else {
            EEPROM.write(0x00, 0xFF);
        }
#endif
    
    // Perform Flight Data initialization
    fillArray(FLIGHT_DATA::inboundData, sizeof(FLIGHT_DATA::inboundData), 0);


    while (1) {
        flight_loop();
    }
    
    return 0;
}

/**
 * Run continuously during normal flight. Performs
 * 1. Iridium Modem Startup Sequence
 * 2. Sensor Polling and Data Collection
 * 3. Data parsing and flight computer action
 * 4. Data Transmission and Receiving
 * 5. Iridium Modem Sleep Sequence
 * 6. Transition to low power mode
 */
void flight_loop() {

    // Check GroundLink for commands...
    if (Serial.available()) {

        // We are only reading the first 50 bytes, then clearing the buffer.
        // This is because the 50-byte = 1 credit limit of the iridium modem.
        for (int i = 0; i < 50; i++) {
            if (Serial.available() > 0) {
                FLIGHT_DATA::inboundData[i] = Serial.read();
            } else {
                break;
            }
        }

        Serial.println("Data read!");
        Serial.println(FLIGHT_DATA::inboundData);
        fillArray(FLIGHT_DATA::inboundData, 50, 0);
        
        
    } 

    delay(100);

}

/**
 * Clears the array pointed to by arr
 */
void fillArray(char* arr, size_t arr_size, int val) {
    for (size_t i = 0; i < arr_size; i++) {
            arr[i] = val;
        }
}