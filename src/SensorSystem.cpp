//https://www.mide.com/air-pressure-at-altitude-calculator
//https://forums.adafruit.com/viewtopic.php?f=22&t=58064
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#include "SensorSystem.h"
#include "FlightData.h"

Adafruit_BME280 _sen_bme280;


bool init_sensor_system() {

    if(_sen_bme280.begin()) {
        FLIGHT_DATA::hardware_status_bitfield |= 1UL << 3;
    } else {
        FLIGHT_DATA::hardware_status_bitfield = ~(1UL << 3);
    }


    return true;

}

//https://forum.mysensors.org/topic/7046/solved-bme280-power-consumtion/6