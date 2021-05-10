#ifndef FLIGHT_DATA_H
#define FLIGHT_DATA_H

    #include <stdint.h>

    namespace FLIGHT_DATA {

        // System Mode

        extern uint8_t system_mode;

        // Buffers

        extern uint8_t outbound_data[52];
        
        extern uint8_t inbound_data[50];

        extern char iridiumRecieveBufferData[65];


        extern uint8_t iridium_mt_queued;

        // Schedulers

        extern uint16_t ballast_ap_interval;
        extern uint32_t ballast_ap_scheduled_time;

        extern uint16_t iridium_transmit_interval;
        extern uint32_t iridium_transmission_scheduled_time;

        // Iridium States

        extern uint8_t force_transmission;
        extern uint8_t last_transmission_status;

        extern uint8_t hardware_status_bitfield;

        extern int strobe_light_status;                             // <----- should not be int (2 bytes), should be uint8
        
        extern int ballast_autopilot_status;
        
        extern uint8_t rx_check_num ;

        extern uint32_t sea_level_pressure_pascals;
        extern uint16_t sea_level_temperature;

        extern uint32_t previous_altitude;
        extern uint32_t previous_altitude_measurement_time;

        void set_hardware_bf_bit(int bit, bool to);

        uint32_t compute_altitude(uint32_t pressure_average);

    }

#endif