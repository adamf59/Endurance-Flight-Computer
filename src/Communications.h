#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

    /**
     * Initialization function, call once at startup.
     */
    void _com_init();


    void process_inbound_data();
    void compute_outbound_checksum();
    char* send_modem_command(char transmission[], int read_timeout);
    void const flush_iridium_recieve_buffer();
    void const flush_iridium_serial_buffer();

    void read_iridium_buffer();

    bool transmit_outbound();

    #ifndef FLIGHT_MODE
        void gm_check_groundlink();
    #endif

#endif