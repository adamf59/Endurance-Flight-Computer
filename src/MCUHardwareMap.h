#ifndef MCU_HARDWARE_MAP_H
#define MCU_HARDWARE_MAP_H

    // Define any hardware pins or types below:
    // No value should change here.

    // AVR328p PORTD:
    
    #define _HW_PIN_EXTERNAL_TEMPERATURE_DATA       2
    #define _HW_PIN_IRIDIUM_MODEM_SLEEP             3
    // Endurance status LED
    #define _HW_PIN_STATUS_INDICATOR_LED            5
    // DHT22 Sensor Data
    #define _HW_PIN_DHT22_SENSOR_DATA               6
    // Iridium Modem UART
    #define _HW_PIN_IRIDIUM_MODEM_RECEIVE           7

    // AVR328p PORTB:

    #define _HW_PIN_IRDIUM_MODEM_TRANSMIT           8
    // Signal that will flash on/off at a rate of 1Hz, meant for strobe lighting.
    #define _HW_PIN_VISIBILITY_STROBE_LED           9
    // BME 280 Chip Select for SPI
    #define _HW_PIN_BME280_CS                       10
    // Flight Termination System (FTS) Channels
    #define _HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_A  11
    #define _HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_B  12
    // Signal wire for ballast pump
    #define _HW_PIN_BALLAST_TRIGGER                 13

    
    #define _HW_TYPE_DHT_22_SENSOR_TYPE          DHT22

#endif