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
    // EEPROM.write(EEPROM_SYSTEM_MODE_ADDR, 0x00);
    // Set System Mode from EEPROM.
    // Mode byte stored at address 0.
    FLIGHT_DATA::system_mode = EEPROM.read(EEPROM_SYSTEM_MODE_ADDR);

    
    // Initialize communications and iridium modem
    _com_init();

    // Initialize IO Data Direction Registers
    DDRD |= 0b00111000;
    DDRB |= 0b00000010; // starts at pin 8
    pinMode(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_A, OUTPUT);
    pinMode(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_B, OUTPUT);


    // Perform Flight Data initialization
    memset(FLIGHT_DATA::inbound_data, 0, sizeof(FLIGHT_DATA::inbound_data));

    // Initialize sensor system
    init_sensor_system();

    // Perform startup system test
    //system_health_check();

    // Enable Strobes
    set_strobes(true);

    // Set the Pump and FTS to low.
    // PORTB &= 0b11000111;

    // Put Iridium to Sleep Mode
    digitalWrite(_HW_PIN_IRIDIUM_MODEM_SLEEP, LOW);

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

    FLIGHT_DATA::set_hardware_bf_bit(0, check_iridium_ready());
    FLIGHT_DATA::set_hardware_bf_bit(7, 1);

    // Send a test transmission to iridium modem

    // transmit_outbound();

    // Test sensors, compare results

}

// bool iridum_modem_ready = false;
// bool sensor_system_ready = false;
// long iridium_modem_startup_time = 0L;

void flight_loop() {

    // Run Data Collector
    collect_data_for_tx();

    if (FLIGHT_DATA::system_mode == 0) {
        // GROUND MODE

        // Check Groundlink for commands
        gm_check_groundlink();
        if (FLIGHT_DATA::force_transmission) {
            run_iridium_tx_rx_sequence();
            
            // Also, lets transmit to groundlink:
            for (uint8_t i = 0; i < 50; i++) {
                Serial.write(FLIGHT_DATA::outbound_data[i]);
            }
        }

    } else if (FLIGHT_DATA::system_mode == 1) {
        // TERMINAL COUNT MODE
        if (millis() >= FLIGHT_DATA::iridium_transmission_scheduled_time) {
            FLIGHT_DATA::iridium_transmission_scheduled_time = FLIGHT_DATA::iridium_transmission_scheduled_time + ((uint32_t) FLIGHT_DATA::iridium_transmit_interval * 1000); // set next target time
            // Trigger the pump briefly to signal operation.
            digitalWrite(_HW_PIN_BALLAST_TRIGGER, HIGH);
            delay(2000);
            digitalWrite(_HW_PIN_BALLAST_TRIGGER, LOW);

            // Run Transmission Sequence
            run_iridium_tx_rx_sequence();
        }
        
    } else if (FLIGHT_DATA::system_mode == 2) {
        // FLIGHT MODE

        // CHECK SCHEDULERS:
        if (millis() >= FLIGHT_DATA::iridium_transmission_scheduled_time) {
            FLIGHT_DATA::iridium_transmission_scheduled_time = FLIGHT_DATA::iridium_transmission_scheduled_time + ((uint32_t) FLIGHT_DATA::iridium_transmit_interval * 1000); // set next target time

            // Run Ballast Sequence
            run_ballast_evaluation();

            // Run Transmission Sequence
            run_iridium_tx_rx_sequence();

        }

        // if (millis() >= FLIGHT_DATA::ballast_ap_scheduled_time) {
        //     FLIGHT_DATA::ballast_ap_scheduled_time = FLIGHT_DATA::ballast_ap_scheduled_time + ((uint32_t) FLIGHT_DATA::ballast_ap_interval * 1000); // set next target time
            
        // }
        
    }
    
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
//                 // Serial.println(_read_sen_bme280_pressure());
//                 // Serial.println(_read_sen_bmp280_pressure());
//                 // Serial.println(_read_sen_mpl3115a2_pressure());

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

//             // (int)(_read_sen_bmp280_pressure() * 100),
//             // // (int)(_read_sen_bme280_pressure() * 100),
//             // 0,
//             // (int)(_read_sen_mpl3115a2_pressure() * 100),

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
        // Strobe LED Setup for 1Hz Flash, ~20% Duty Cycle
        // We're mandated anywhere between 40 and 120 flashes per minute by FAA, so 1Hz is OK.
        TCCR1A = _BV(COM1A1) | _BV(WGM11);                
        TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12);     
        ICR1 = 62499;                                     
        OCR1A = 3000;   // Se   t the duty-cycle to 10%: 62499 / 20 ~= 3000
        FLIGHT_DATA::strobe_light_status = 1;

    } else {

        digitalWrite(_HW_PIN_VISIBILITY_STROBE_LED, LOW);
        FLIGHT_DATA::strobe_light_status = 0;
    }
}

void run_ballast_evaluation() {

    if (!FLIGHT_DATA::ballast_autopilot_status)  return;
    // Check Activation Parameter
    if (!EEPROM.read(EEPROM_BALLAST_AUTOPILOT_ACTIVATION_ADDR)) {
        if (FLIGHT_DATA::current_altitude >= BALLAST_AP_ACTIVATION_ALTITUDE) {
            EEPROM.write(EEPROM_BALLAST_AUTOPILOT_ACTIVATION_ADDR, 0x01);       // Auto-Activate the Autopilot
        } else  {
            return;
        }
    }

    uint32_t altitude_divider = (FLIGHT_DATA::autopilot_upper_altitude_threshold - FLIGHT_DATA::autopilot_lower_alitude_threshold) / 5;
    FLIGHT_DATA::ballast_autopilot_drop_time = 0;

    if (FLIGHT_DATA::current_altitude > FLIGHT_DATA::autopilot_upper_altitude_threshold) {
        FLIGHT_DATA::ballast_autopilot_drop_time = FLIGHT_DATA::altitude_zone_constants[6] * FLIGHT_DATA::ballast_autopilot_coefficient;
    } else {
        for (int i = 0; i < 6; i++) {

            if (FLIGHT_DATA::current_altitude <= (FLIGHT_DATA::autopilot_lower_alitude_threshold + (altitude_divider * i))) {
                FLIGHT_DATA::ballast_autopilot_drop_time = FLIGHT_DATA::altitude_zone_constants[i] * FLIGHT_DATA::ballast_autopilot_coefficient;
                break;
            }

        }
    }

    // Now drop ballast.
    digitalWrite(_HW_PIN_BALLAST_TRIGGER, HIGH);
    delay(FLIGHT_DATA::ballast_autopilot_drop_time * 1000);
    digitalWrite(_HW_PIN_BALLAST_TRIGGER, LOW);


}