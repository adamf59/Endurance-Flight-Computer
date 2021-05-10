#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

    /**
     * Initialization function, call once at startup.
     */
    void _com_init();

    void collect_data_for_tx();
    void process_inbound_data();
    void compute_outbound_checksum();
    char* send_modem_command(char transmission[], int read_timeout);
    void const flush_iridium_recieve_buffer();
    void const flush_iridium_serial_buffer();

    void run_iridium_tx_rx_sequence();

    bool check_iridium_ready();

    void read_iridium_buffer();

    uint8_t transmit_outbound();

    #ifndef FLIGHT_MODE
        void gm_check_groundlink();
    #endif

#endif