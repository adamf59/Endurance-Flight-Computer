#ifndef SENSOR_SYSTEM_H
#define SENSOR_SYSTEM_H

    /**
     * Initializes all sensors defined
     */
    void init_sensor_system();

    // Temperature Readings:

    float _read_sen_bme280_temp();
    float _read_sen_bmp280_temp();
    float _read_sen_mpl3115a2_temp();
    float _read_sen_dht22_temp();
    float _read_sen_ds18b20_temp();

    // Pressure Readings

    uint32_t _read_sen_bmp280_pressure();
    uint32_t _read_sen_mpl3115a2_pressure();
    uint32_t _read_sen_bme280_pressure();

    // Humidity Readings

    uint8_t _read_sen_bme280_rhumidity();
    uint8_t _read_sen_dht22_rhumidity();

    // Battery Monitor

    uint16_t _read_sen_ina260_voltage();
    uint32_t _read_sen_ina260_current();

#endif SENSOR_SYSTEM_H