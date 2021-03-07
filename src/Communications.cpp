#include "Arduino.h"
#include "Communications.h"
#include "MCUHardwareMap.h"
#include "FlightData.h"
#include "jsfc.h"

#include <SoftwareSerial.h>

#include <util/crc16.h>
//http://ugweb.cs.ualberta.ca/~c274/resources/arduino-ua/avr-libc-1.7.1-overlay/avr-libc/avr-libc-user-manual-1.7.1/group__util__crc.html

SoftwareSerial iridiumModem(_HW_PIN_IRIDIUM_MODEM_RECEIVE, _HW_PIN_IRDIUM_MODEM_TRANSMIT);

char iridiumRecieveBufferData[100];

void _com_init() {
    iridiumModem.begin(19200);

#ifndef FLIGHT_MODE
    // GroundLink Serial
    Serial.begin(19200);
    Serial.println(F("<ST_START>")); // Send "startup complete" to GL
#endif

}

void process_inbound_data() {
    
    
}

void const flush_iridium_recieve_buffer() {
    fillArray(iridiumRecieveBufferData, 100, 0);
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
char* send_modem_command(char transmission[], int read_timeout) {

    // Clear the buffers
    flush_iridium_recieve_buffer();
    flush_iridium_serial_buffer();

    // Write the transmission
    iridiumModem.write(transmission);

    // Wait for the modem to respond
    // TODO there may be a better way to do this.
    delay(read_timeout);

    int writeIdx = 0;
    // Read any data from the iridium modem
    while (iridiumModem.available()) {
        if (writeIdx == sizeof(iridiumRecieveBufferData)) break;   // prevent buffer overflows
        char inChar = iridiumModem.read();
        // only accepting ASCII letters, numbers, etc. (i.e. no control characters like \0, NL, CR, etc.)
        if (inChar >= 32 && inChar <= 122)  iridiumRecieveBufferData[writeIdx++] = inChar;
    }

    return iridiumRecieveBufferData;
    
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

#ifndef FLIGHT_MODE
void gm_check_groundlink() {
    // Check groundlink for commands and run executor if commands found.

    fillArray(FLIGHT_DATA::inboundData, 50, 0);

    if (Serial.available()) {

            // We are only reading the first 50 bytes, then clearing the buffer.
            // This is because the 50-byte = 1 credit limit of the iridium modem.
            for (int i = 0; i < 50; i++) {
                if (Serial.available() > 0) {
                    FLIGHT_DATA::inboundData[i] = Serial.read();
                } else {
                    break;
                }
            }
            process_inbound_data();
                    
        }
}
#endif