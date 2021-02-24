#include "Arduino.h"
#include "Communications.h"
#include "MCUHardwareMap.h"
#include "FlightData.h"

#include <SoftwareSerial.h>

SoftwareSerial iridiumModem(_HW_PIN_IRIDIUM_MODEM_RECEIVE, _HW_PIN_IRDIUM_MODEM_TRANSMIT);

char iridiumRecieveBufferData[100];

void processInboundData() {
    
}

void const flush_iridium_recieve_buffer() {
    for (int i = 0; i < 100; i++) {
        iridiumRecieveBufferData[i] = 0;
    }
}

/**
 * Clears any characters waiting in the iridium software serial buffer 
 */
void const flush_iridium_serial_buffer() {
    while (iridiumModem.available() > 0)    iridiumModem.read();
}

/**
 * Sends a message to the iridium modem, and stores the response into iridiumRecieveBufferData
 */
void send_modem_command(char transmission[], int read_timeout) {

    // Clear the buffers
    flush_iridium_recieve_buffer();
    flush_iridium_serial_buffer();

    // Write the transmission
    iridiumModem.write(transmission);

    delay(read_timeout);

    int writeIdx = 0;
    // Read any data from the iridium modem
    while (iridiumModem.available()) {
        char inChar = iridiumModem.read();
        if (writeIdx++ == 100) break;   // prevent buffer overflows
        if (inChar >= 32 && inChar <= 122)  iridiumRecieveBufferData[writeIdx] = inChar;
    }
    
}

/*
 * Computes the checksum of the data stored in FLIGHT_DATA::outboundData
 * See docs at: https://docs.rockblock.rock7.com/reference#sbdwb
 */
void compute_outbound_checksum() {

    int outbound_summation = 0; // Max: (255 * 50) 12750 / 0x31CA / 00111001 11001110

    for (int i = 0; i < 50; i++) {
        outbound_summation += (int) FLIGHT_DATA::outboundData[i];
    }

    FLIGHT_DATA::outboundData[50] = (outbound_summation >> 8) & 0xFF;
    FLIGHT_DATA::outboundData[51] = outbound_summation & 0xFF;
}

