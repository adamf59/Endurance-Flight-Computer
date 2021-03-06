/**
 * JagSat Flight Control Program
 * Written by Adam Frank
 * 2/21/2021
 * 
 * #define FLIGHT_MODE to configure the firmware for flight mode
 */

#include <Arduino.h>
#include <EEPROM.h>

#include "jsfc.h"
#include "MCUHardwareMap.h"
#include "FlightData.h"
#include "Communications.h"
#include "SensorSystem.h"


int main() {

    // Initialize the AVR Board:
    init();

    // Initialize sensor system
    init_sensor_system();
    

    // Initialize communications and iridium modem
    _com_init();
    
    pinMode(_HW_PIN_STATUS_INDICATOR_LED, OUTPUT);
    pinMode(_HW_PIN_BALLAST_TRIGGER, OUTPUT);
    pinMode(_HW_PIN_IRIDIUM_MODEM_SLEEP, OUTPUT);

    for (int i = 0; i < 5; i++) {
        digitalWrite(_HW_PIN_STATUS_INDICATOR_LED, true);
        delay(150);
        digitalWrite(_HW_PIN_STATUS_INDICATOR_LED, false);
        delay(150);
    }

#ifndef FLIGHT_MODE
    // GroundLink Serial
    Serial.begin(19200);
    Serial.println("1"); // Send "startup complete" to GL
#endif
    
    // Perform Crash Check
    // Crash byte stored at address 0, and is SET at 0xFF, CLEAR at 0x00.

#ifdef FLIGHT_MODE
        if (EEPROM.read(0x00) == 0xFF) {
            
            // TODO crash logic goes here.

        } else {
            EEPROM.write(0x00, 0xFF);
        }
#endif
    
    // Perform Flight Data initialization
    fillArray(FLIGHT_DATA::inboundData, sizeof(FLIGHT_DATA::inboundData), 0);

    // Perform startup system test
    system_health_check();

    // Finally, enter the flight loop, which shouldn't ever really end.

    while (1) {
        flight_loop();
    }

    return 0;
}

/**
 * Runs a health check to ensure all systems are working properly. Tests all sensors, iridium modem connection / transmission, etc.
 * Warning: This will use 1 credit if an antenna is connected to the iridum modem!
 */
void system_health_check() {

    // Check iridium modem connection:
    if (!send_modem_command("AT\r", 50) == "OK") {
        FLIGHT_DATA::hardware_status_bitfield &= ~(1UL << 0);
    } else {
        FLIGHT_DATA::hardware_status_bitfield |= 1UL << 0;
    }

    // Send a test transmission to iridium modem


    // Test sensors, compare results

    #ifndef FLIGHT_MODE
        // Echo result to groundlink
            Serial.print(F("<$"));
            Serial.print(FLIGHT_DATA::hardware_status_bitfield);
            Serial.println(F(">"));
    #endif

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

#ifndef FLIGHT_MODE
    // Run GroundLink checks
    gm_check_groundlink();    
#endif


    delay(100);

    // TODO "Smart Sleep": where the system time is clocked on wakeup
    // then we wait till 30 sec has passed (during that time do sensor collection / averaging, and packaging)
    // and finally when the time has passed, transmit that data.


    /**
     *  Start Iridium, and clock the system time (millis()). Begin collecting sensor data.
     * Usually, we should let the sensors run for about 10 sec or so and collect multiple samples before taking some "real" measuremnts.
     * Take 3 "real" measurements and average them. Then compare results with other sensors. If they match, we're good.
     * 
     * Now check to see if 30 seconds have passed. If so, continue with transmitting. Otherwise, wait.
     * Once iridium is ready, perform a check to see if the modem is responding, and finally run the tx sequence.
     * 
     * All said and done, put the modem into sleep mode as well as the sensors. Finally, we power down and wait for the next wakeup.
    */
   
    }

/**
 * Clears the array pointed to by arr
 */
void fillArray(char* arr, size_t arr_size, int val) {
    for (size_t i = 0; i < arr_size; i++) {
            arr[i] = val;
        }
}