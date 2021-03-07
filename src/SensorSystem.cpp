//https://www.mide.com/air-pressure-at-altitude-calculator
//https://forums.adafruit.com/viewtopic.php?f=22&t=58064
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_INA219.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "SensorSystem.h"
#include "FlightData.h"
#include "MCUHardwareMap.h"

Adafruit_BME280 _sen_bme280(_HW_PIN_BME280_CS);
Adafruit_BMP280 _sen_bmp280;
Adafruit_MPL3115A2 _sen_mpl3115a2;
Adafruit_INA219 _sen_ina219;
DHT _sen_dht22(_HW_PIN_DHT22_SENSOR_DATA, _HW_TYPE_DHT_22_SENSOR_TYPE);

bool init_sensor_system() {

    if(_sen_bme280.begin()) {
        FLIGHT_DATA::hardware_status_bitfield |= (1UL << 3);
    } else {
        FLIGHT_DATA::hardware_status_bitfield &= ~(1UL << 3);
    }

    if(_sen_bmp280.begin()) {
        FLIGHT_DATA::hardware_status_bitfield |= (1UL << 5);
        
    } else {
        FLIGHT_DATA::hardware_status_bitfield &= ~(1UL << 5);
    }

    if(_sen_mpl3115a2.begin()) {
        FLIGHT_DATA::hardware_status_bitfield |= (1UL << 6);
        
    } else {
        FLIGHT_DATA::hardware_status_bitfield &= ~(1UL << 6);
    }

    if(_sen_ina219.begin()) {
        FLIGHT_DATA::hardware_status_bitfield |= (1UL << 4);
        
    } else {
        FLIGHT_DATA::hardware_status_bitfield &= ~(1UL << 4);
    }

    //DHT22 Setup
    _sen_dht22.begin();
    if (!isnan(_read_sen_dht22_temp())) {
        FLIGHT_DATA::hardware_status_bitfield |= (1UL << 2);
        
    } else {
        FLIGHT_DATA::hardware_status_bitfield &= ~(1UL << 2);
    }


    return true;

}

//https://forum.mysensors.org/topic/7046/solved-bme280-power-consumtion/6


float _read_sen_dht22_temp() {
  return _sen_dht22.readTemperature();
}