//https://www.mide.com/air-pressure-at-altitude-calculator
//https://forums.adafruit.com/viewtopic.php?f=22&t=58064
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_INA219.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>

#include "SensorSystem.h"
#include "FlightData.h"
#include "MCUHardwareMap.h"

Adafruit_BME280 _sen_bme280(_HW_PIN_BME280_CS);
Adafruit_BMP280 _sen_bmp280;
Adafruit_MPL3115A2 _sen_mpl3115a2;
Adafruit_INA219 _sen_ina219;
DHT _sen_dht22(_HW_PIN_DHT22_SENSOR_DATA, _HW_TYPE_DHT_22_SENSOR_TYPE);
OneWire _sen_ds18b20(_HW_PIN_EXTERNAL_TEMPERATURE_DATA);

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


/**
 * Read temperature data in celcius from the DHT22 sensor
 */
float _read_sen_dht22_temp() {
  return _sen_dht22.readTemperature();
}

/**
 * Read temperature data from the DS18B20 Sensor in degrees celcius.
 * Note: For Endurance Flight Computer Rev. 1.0, this is the EXTERNAL TEMPERATURE SENSOR.
 */
float _read_sen_ds18b20_temp() {

  byte data[12];
  byte addr[8];

  if ( !_sen_ds18b20.search(addr)) {
      //no more sensors on chain, reset search
      _sen_ds18b20.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println(F("CRC is not valid!"));
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print(F("Device is not recognized"));
      return -1000;
  }

  _sen_ds18b20.reset();
  _sen_ds18b20.select(addr);
  _sen_ds18b20.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = _sen_ds18b20.reset();
  _sen_ds18b20.select(addr);    
  _sen_ds18b20.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = _sen_ds18b20.read();
  }
  
  _sen_ds18b20.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}
