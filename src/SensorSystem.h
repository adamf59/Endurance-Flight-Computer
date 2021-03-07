#ifndef SENSOR_SYSTEM_H
#define SENSOR_SYSTEM_H

    /**
     * Initializes all sensors defined
     */
    bool init_sensor_system();

    float read_BME280();

    float _read_sen_dht22_temp();

#endif SENSOR_SYSTEM_H