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

    float _read_sen_bmp280_q();
    float _read_sen_mpl3115a2_q();
    float _read_sen_bme280_q();

    // Humidity Readings

    float _read_sen_bme280_rhumidity();
    float _read_sen_dht22_rhumidity();

    // Battery Monitor

    float _read_sen_ina219_voltage();
    float _read_sen_ina219_current();

#endif SENSOR_SYSTEM_H