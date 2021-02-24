#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

    void processInboundData();
    void compute_outbound_checksum();
    void const flush_iridium_recieve_buffer();
    void const flush_iridium_serial_buffer();

#endif