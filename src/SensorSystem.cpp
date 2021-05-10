//https://www.mide.com/air-pressure-at-altitude-calculator
//https://forums.adafruit.com/viewtopic.php?f=22&t=58064
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_INA260.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>

#include "SensorSystem.h"
#include "FlightData.h"
#include "MCUHardwareMap.h"

Adafruit_BME280 _sen_bme280(_HW_PIN_BME280_CS);
Adafruit_BMP280 _sen_bmp280;
Adafruit_MPL3115A2 _sen_mpl3115a2;
Adafruit_INA260 _sen_ina260;
//DHT _sen_dht22(_HW_PIN_DHT22_SENSOR_DATA, _HW_TYPE_DHT_22_SENSOR_TYPE);
OneWire _sen_ds18b20(_HW_PIN_EXTERNAL_TEMPERATURE_DATA);

void init_sensor_system() {

    FLIGHT_DATA::set_hardware_bf_bit(3, _sen_bme280.begin());

    FLIGHT_DATA::set_hardware_bf_bit(5, _sen_bmp280.begin());

    FLIGHT_DATA::set_hardware_bf_bit(6, _sen_mpl3115a2.begin());

    FLIGHT_DATA::set_hardware_bf_bit(4, _sen_ina260.begin());

    // _sen_dht22.begin();
    //FLIGHT_DATA::set_hardware_bf_bit(2, !isnan(_read_sen_dht22_temp()));
    
    FLIGHT_DATA::set_hardware_bf_bit(1, !isnan(_read_sen_ds18b20_temp()));    

    return true;
}

//https://forum.mysensors.org/topic/7046/solved-bme280-power-consumtion/6


// Temperature Readings

float _read_sen_bme280_temp() {
    return _sen_bme280.readTemperature();
}

float _read_sen_bmp280_temp() {
    return _sen_bmp280.readTemperature();
}

float _read_sen_mpl3115a2_temp() {
    return _sen_mpl3115a2.getTemperature();
}

// float _read_sen_dht22_temp() {
//   return _sen_dht22.readTemperature();
// }

/* (External Temperature) */
float _read_sen_ds18b20_temp() {

  byte data[12];
  byte addr[8];

  if ( !_sen_ds18b20.search(addr)) {
      //no more sensors on chain, reset search
      _sen_ds18b20.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
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

// Pressure Readings
// Unit: pascals (Pa)

uint32_t _read_sen_bmp280_pressure() {
    return (uint32_t) _sen_bmp280.readPressure();
}

uint32_t _read_sen_mpl3115a2_pressure() {
    return (uint32_t) _sen_mpl3115a2.getPressure();
}

uint32_t _read_sen_bme280_pressure() {
    return (uint32_t) _sen_bme280.readPressure();
}

// Humidity Readings
// Unit: 0-100 percent
uint8_t _read_sen_bme280_rhumidity() {
    return (uint8_t) _sen_bme280.readHumidity();
}

// uint8_t _read_sen_dht22_rhumidity() {
//     return (uint8_t) _sen_dht22.readHumidity();
// }


// INA260 Battery Monitor

uint16_t _read_sen_ina260_voltage() {
    return (uint16_t) ((_sen_ina260.readBusVoltage() / 1000.0) * 100);
}

uint32_t _read_sen_ina260_current() {
    return (uint32_t) (_sen_ina260.readCurrent() * 100);
}