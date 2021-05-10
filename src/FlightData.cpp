#include "FlightData.h"
#include "Arduino.h"
#include <stdint.h>
#include <math.h>

namespace FLIGHT_DATA {

    // data to be sent:
    extern uint8_t outbound_data[52] = {0};  // 50 byte / credit + 2 for checksum
        
    // parsable data we get from iridium network OR GroundLink
    extern uint8_t inbound_data[50] = {0};

    extern char iridiumRecieveBufferData[65] = {0};


    /** Bitfield indicies:
     * 0 - Iridium Modem Status
     * 1 - External Temperature Status
     * 2 - DHT22 Sensor Status
     * 3 - BME280 Sensor Status
     * 4 - INA219 Sensor Status
     * 5 - BMP280 Sensor Status
     * 6 - MPL3115A2 Sensor Status
     * 7 - Flight Computer System Status
     */
    extern uint8_t hardware_status_bitfield = 0b00000000;

    extern uint8_t force_transmission = 0;

    extern uint8_t iridium_mt_queued = 0;

    // Stobe lights status
    extern int strobe_light_status = 0; // 0 or 1
    extern int ballast_autopilot_status = 0; // 0 or 1

    // check number we get from an RX-message (here) to send in the next TX
    extern uint8_t rx_check_num = 0;
    
    // Time between ballast evaluations and transmissions in seconds. default 7 minutes (420 seconds)
    extern uint16_t fcpu_update_interval = 10;

    // use 101321 for standard atmospheric model, usually effective above FL180 (equivalent to 29.92 inHg)     
    extern uint32_t sea_level_pressure_pascals = 101321;
    
    // use 288.15 K for standard atmospheric model
    extern uint16_t sea_level_temperature = 288.15;

    extern uint32_t previous_altitude = 500;
    // WARNING: Set this to altitude of your launch site in feet.

    extern uint32_t previous_altitude_measurement_time = 0;


    void set_hardware_bf_bit(int bit, bool to) {
        if(to) {
            FLIGHT_DATA::hardware_status_bitfield |= (1UL << bit);
        } else {
            FLIGHT_DATA::hardware_status_bitfield &= ~(1UL << bit);
        }
    }

    /**
     * Calculates altitude in feet, accurate when at or below 36090 feet.
     */
    uint32_t compute_altitude(uint32_t pressure_average) {
        /**
         * Required Variables:
         *  - Temperature at sea level
         *  - Pressure at sea level
         */ 

        return (uint32_t) ((((double) sea_level_temperature) / -0.0065D) * (pow(((double) pressure_average / (double) sea_level_pressure_pascals), 0.1902632D) - 1.00F) * 3.28);
    }
    

}