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

    // Initialize communications and iridium modem
    _com_init();

    // Initialize IO    
    pinMode(_HW_PIN_STATUS_INDICATOR_LED, OUTPUT);
    // pinMode(_HW_PIN_BALLAST_TRIGGER, OUTPUT);
    pinMode(_HW_PIN_IRIDIUM_MODEM_SLEEP, OUTPUT);
    pinMode(_HW_PIN_VISIBILITY_STROBE_LED, OUTPUT);
    
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
    memset(FLIGHT_DATA::inbound_data, 0, sizeof(FLIGHT_DATA::inbound_data));

    // Initialize sensor system
    init_sensor_system();

    // Perform startup system test
    system_health_check();

#ifndef FLIGHT_MODE
    // LED Indications in Ground Mode
    for (int i = 0; i < 5; i++) {
        digitalWrite(_HW_PIN_STATUS_INDICATOR_LED, true);
        digitalWrite(_HW_PIN_VISIBILITY_STROBE_LED, true);
        delay(150);
        digitalWrite(_HW_PIN_STATUS_INDICATOR_LED, false);
        digitalWrite(_HW_PIN_VISIBILITY_STROBE_LED, false);
        delay(150);
    }
#endif

    // Enable Strobes
    set_strobes(true);

    // Finally, enter the flight loop, which shouldn't ever really end.

    while (1) {
        // set_strobes(false);
        // delay(2000);
        // set_strobes(true);
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

    FLIGHT_DATA::set_hardware_bf_bit(0, check_iridium_ready());

    // Send a test transmission to iridium modem

    // transmit_outbound();

    // Test sensors, compare results


    Serial.print(FLIGHT_DATA::hardware_status_bitfield, BIN);
    Serial.print("\nDSI8B20 Temperature:");
    Serial.println(_read_sen_dht22_temp());

}

system_state fcpu_lifecycle_state = STARTUP;
bool iridum_modem_ready = false;
bool sensor_system_ready = false;
long iridium_modem_startup_time = 0L;

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

    gm_check_groundlink();
    delay(500);

// // #ifndef FLIGHT_MODE
// //     // Run GroundLink checks
// //     gm_check_groundlink();    
// // #endif
//     switch (fcpu_lifecycle_state) {
//         case STARTUP:
//             //Serial.println(F("FCPU in Startup..."));
//             iridium_modem_startup_time = millis();
//             digitalWrite(_HW_PIN_IRIDIUM_MODEM_SLEEP, 1);   // First, wakeup the Iridium Modem
//             fcpu_lifecycle_state = POLL_WAIT_IRIDIUM;
//             break;
//         case POLL_WAIT_IRIDIUM:
//             // Wakeup the sensors... Begin polling for data.

//             // Take a few samples to ensure the data is "accurate"
//             if (!sensor_system_ready) {
//                 // Serial.println("Sampling...");
//                 // Serial.println(_read_sen_bmp280_temp());
//                 // Serial.println(_read_sen_mpl3115a2_temp());
//                 // Serial.println(_read_sen_dht22_temp());
//                 // Serial.println(_read_sen_ds18b20_temp());
//                 // Serial.println(_read_sen_bme280_q());
//                 // Serial.println(_read_sen_bmp280_q());
//                 // Serial.println(_read_sen_mpl3115a2_q());

//                 delay(1200);    // wait a bit to retake measurements (so we're not spamming the sensor)
//                 sensor_system_ready = true;     // (Development Only)
//             }

//             // Check if iridium is ready...
//             if (!iridum_modem_ready && check_iridium_ready()) {
//                 iridum_modem_ready = true;
//                 //Serial.println(F("Iridium Modem Startup Complete!..."));
//             } else {
//                 if (sensor_system_ready)   delay(1200);     // Continue delaying if the sensor system is ready.
//             }
            
//             if (sensor_system_ready && iridum_modem_ready)      fcpu_lifecycle_state = TX_RX;

//             break;
//         case TX_RX:
//             //Serial.println("Attempting Iridium Transmission / GL");

//             // char  COLD_TEST_TMP_OBA[210];
//             // fillArray(COLD_TEST_TMP_OBA, 210, 0);

//             // sprintf(COLD_TEST_TMP_OBA, "<%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i>",
//             // (int)(_read_sen_bmp280_temp() * 100),
//             // // (int)(_read_sen_bme280_temp() * 100),
//             // 0,
//             // (int)(_read_sen_mpl3115a2_temp() * 100),
//             // (int)(_read_sen_dht22_temp() * 100),
//             // (int)(_read_sen_ds18b20_temp() * 100),

//             // (int)(_read_sen_bmp280_q() * 100),
//             // // (int)(_read_sen_bme280_q() * 100),
//             // 0,
//             // (int)(_read_sen_mpl3115a2_q() * 100),

//             // // (int)_read_sen_bme280_rhumidity(),
//             // 0,
//             // (int)_read_sen_dht22_rhumidity(),

//             // (int)(_read_sen_ina219_voltage() * 100),
//             // (int)_read_sen_ina219_current()
            
//             // );

//             // Serial.println(COLD_TEST_TMP_OBA);
            

//             digitalWrite(_HW_PIN_BALLAST_TRIGGER, HIGH);
//             delay(4000);
//             digitalWrite(_HW_PIN_BALLAST_TRIGGER, LOW);
//             fcpu_lifecycle_state = LOW_POWER_SLEEP;
//             break;
//         case LOW_POWER_SLEEP:
//             //Serial.println("Transitioning to sleep mode...");
//             iridum_modem_ready = false;
//             sensor_system_ready = false;
//             digitalWrite(_HW_PIN_IRIDIUM_MODEM_SLEEP, 0);
//             fcpu_lifecycle_state = STARTUP;
//             delay(240000);
//             break;
//     }

    
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

void set_strobes(bool state) {
    if(state) {
        // Strobe LED Setup for 1Hz Flash, 10% Duty Cycle
        TCCR1A = _BV(COM1A1) | _BV(WGM11);                
        TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12);     
        ICR1 = 62499;                                     
        OCR1A = 3000;   // Se   t the duty-cycle to 10%: 62499 / 10 = 6249
        FLIGHT_DATA::strobe_light_status = 1;
    } else {
        digitalWrite(_HW_PIN_VISIBILITY_STROBE_LED, LOW);
        FLIGHT_DATA::strobe_light_status = 0;
    }
}