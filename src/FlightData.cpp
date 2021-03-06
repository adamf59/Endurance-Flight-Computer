#include "FlightData.h"
#include <stdint.h>

namespace FLIGHT_DATA {

    // data to be sent:
    extern uint8_t outboundData[52] = {0};  // 50 byte / credit + 2 for checksum
        
    // data we get from iridium network OR GroundLink
    extern char inboundData[50] = {0};

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
    
    // check number we get from an RX-message (here) to send in the next TX
    extern int checkNum = 0;

    // use 1013.2074 for standard atmospheric model, usually effective above FL180 (equivalent to 29.92 inHg)     
    extern float obd_sea_level_pressure_hpa = 1018.6097;

}