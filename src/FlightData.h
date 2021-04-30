#ifndef FLIGHT_DATA_H
#define FLIGHT_DATA_H

    #include <stdint.h>

    namespace FLIGHT_DATA {

        extern uint8_t outbound_data[52];
        
        extern uint8_t inbound_data[50];

        extern uint8_t hardware_status_bitfield;

        extern int strobe_light_status; 
        
        extern uint8_t rx_check_num ;

        extern float obd_sea_level_pressure_hpa;

        void set_hardware_bf_bit(int bit, bool to);
    }

#endif
