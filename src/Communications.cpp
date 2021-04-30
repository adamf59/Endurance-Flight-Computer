#include "Arduino.h"
#include "Communications.h"
#include "MCUHardwareMap.h"
#include "FlightData.h"
#include "jsfc.h"
#include "SensorSystem.h"

#include <SoftwareSerial.h>

#include <stdint.h>

// Function pointer to built-in restart function. EMERGENCY ONLY!
void(* reset_flight_computer) (void) = 0;

using namespace FLIGHT_DATA;

//http://ugweb.cs.ualberta.ca/~c274/resources/arduino-ua/avr-libc-1.7.1-overlay/avr-libc/avr-libc-user-manual-1.7.1/group__util__crc.html

SoftwareSerial iridiumModem(_HW_PIN_IRIDIUM_MODEM_RECEIVE, _HW_PIN_IRDIUM_MODEM_TRANSMIT);

char iridiumRecieveBufferData[100];

void _com_init() {
    iridiumModem.begin(19200);
    Serial.begin(19200);
   // Serial.println(F("<ST_START>")); // Send "startup complete" to GL
}

void collect_data_for_tx() {
    memset(outbound_data, 0, sizeof(outbound_data));
    
    uint16_t bme280_temperature_measurement = (uint16_t) (_read_sen_bme280_temp() * 100);
    Serial.print("Raw was ");
    Serial.println(bme280_temperature_measurement);
    outbound_data[0] = 0xAA; // Start Byte
    outbound_data[1] = 0x00; // TODO Packet Checksum
    outbound_data[2] = hardware_status_bitfield;

    outbound_data[10] = (uint8_t) bme280_temperature_measurement & 0xFF;
    outbound_data[11] = (uint8_t) (bme280_temperature_measurement >> 8);

    Serial.print(outbound_data[10], HEX);
    Serial.print(" ");
    Serial.print(outbound_data[11], HEX);
    
}

void process_inbound_data() {
    
    // Valid packets have the start byte 0xAA at position 0.
    if (inbound_data[0] != 0xAA) return;
    
    for (int i = 2; i < inbound_data[1] + 2; i++) {
        
        switch(inbound_data[i]) {
            case 0x01: {
                // Set Mode
                if (inbound_data[i + 1] == 0x01) {
                    // set mode to standby
                } else if (inbound_data[i + 1] == 0x02) {
                    // set mode to terminal_count
                } else if (inbound_data[i + 2] == 0x03) {
                    // set mode to flight
                }
                i += 1;
                break;
            }
            case 0x02: {
                // Set Sea-Level Barometer for altitude computations
                uint16_t new_barometer = (inbound_data[i + 2] << 8) | inbound_data[i + 1];
                i += 2;
                break;
            }
            case 0x03: {
                // Set Ballast Autopilot State
                if (inbound_data[i + 1] == 0xFF) {
                    // enable ballast autopilot
                } else {
                    // disable ballast autopilot
                }
                i += 1;
                break;
            }
            case 0x04: {
                // Set Ballast Evaluation Period
                uint16_t ballast_eval_period = (inbound_data[i + 2] << 8) | inbound_data[i + 1];
                i += 2;
                break;
            }
            case 0x05: {
                // Set Ballast Coefficient
                i += 1;
                break;
            }
            case 0x06: {
                // Set Lower Altitude Threshold    
                uint16_t autopilot_lower_alitude_theshold = (inbound_data[i + 2] << 8) | inbound_data[i + 1];
                i += 2;
                break;
            }
            case 0x07: {
                // Set Upper Altitude Threshold    
                uint16_t autopilot_upper_alitude_theshold = (inbound_data[i + 2] << 8) | inbound_data[i + 1];
                i += 2;
                break;
            }
            case 0x08: {
                // Manual Ballast Dispense
                i += 1;
                break;
            }
            case 0x09: {
                set_strobes(inbound_data[i + 1]);
                i += 1;
                break;
            }
            case 0x0A: {
                break;
            }
            case 0x0B: {
                rx_check_num = inbound_data[i + 1];
                i += 1;
                break;
            }
            case 0x0C: {
                collect_data_for_tx();
                break;
            }

            // Emergency Commands
            case 0xF8: {
                if (inbound_data[i + 1] == 0xA8) {
                    // Welp, guess we're restarting...
                    reset_flight_computer();
                }
                // this won't get run in theory...
                i += 1;
                break;
            }
            case 0xF9: {
                    if (inbound_data[i + 1] == 0x55) {
                        /*
                         * Unfortunatly, this is it. Trigger the flight termination system,
                         * which should cut down the payload.
                         * We'll run the system for 8 seconds.
                         * Hopefully, this block of code NEVER gets run.
                         */

                        // rip
                        digitalWrite(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_A, 0x01);
                        digitalWrite(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_B, 0x01);

                        delay(8000);

                        digitalWrite(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_A, 0x00);
                        digitalWrite(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_B, 0x00);

                    }
                    i += 1;

                break;
            }

            // Default should fall through and terminate processing.
            default:
            // Stop Byte
            case 0xBB: {
                return;
            }
        }

    }
    
}

bool transmit_outbound() {
    // Start a session with the iridium modem:

    // check connectivity
    if (!check_iridium_ready()) {
        return false;
    }

    // disable flow control
    if (strcmp(send_modem_command("AT&K0\r", 50), "OK\r") == 0) {
        return false;
    }

    // request transfer of binary data to ISU
    if (strcmp(send_modem_command("AT+SBDWB=50\r", 50), "READY\r") == 0) {
        return false;
    }
    /*
    * Compute the checksum of the data stored in FLIGHT_DATA::outboundData
    * See docs at: https://docs.rockblock.rock7.com/reference#sbdwb
    */
    int outbound_summation = 0; // Max: (255 * 50) 12750 / 0x31CA / 00111001 11001110

    for (int i = 0; i < 50; i++) {
        outbound_summation += (int) FLIGHT_DATA::outbound_data[i];
    }

    FLIGHT_DATA::outbound_data[50] = (outbound_summation >> 8) & 0xFF;
    FLIGHT_DATA::outbound_data[51] = outbound_summation & 0xFF;

    flush_iridium_recieve_buffer();
    flush_iridium_serial_buffer();

    // transfer the binary data from FLIGHT_DATA::outbound_data
    for (int _b = 0; _b < 52; _b++) {
        iridiumModem.write((byte) outbound_data[_b]);
    }

    // now check to see that the ISU liked the data (should be 0OK)
    delay(50);
    read_iridium_buffer();
    if (!strcmp(iridiumRecieveBufferData, "0OK\r")) {
        return false;
    }
    
    // Finally, initiate a short burst session:

    // TODO do short burst session

    return true;
}

void const flush_iridium_recieve_buffer() {
    memset(iridiumRecieveBufferData, 0, sizeof(iridiumRecieveBufferData));
}

/**
 * Clears any characters waiting in the iridium software serial buffer 
 */
void const flush_iridium_serial_buffer() {
    while (iridiumModem.available() > 0)    iridiumModem.read();
}

bool check_iridium_ready() {
    return strcmp(send_modem_command("AT\r", 50), "OK") == 0;
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

    read_iridium_buffer();

    return iridiumRecieveBufferData;
    
}

void read_iridium_buffer() {

    int writeIdx = 0;
    // Read any data from the iridium modem
    while (iridiumModem.available()) {
        if (writeIdx == sizeof(iridiumRecieveBufferData)) break;   // prevent buffer overflows
        char inChar = iridiumModem.read();
        // only accepting ASCII letters, numbers, etc. (i.e. no control characters like \0, NL, CR, etc.)
        if (inChar >= 32 && inChar <= 122)  iridiumRecieveBufferData[writeIdx++] = inChar;
    }

}

void gm_check_groundlink() {
    // Check groundlink for commands and run executor if commands found.

    
    if (Serial.available()) {
            memset(FLIGHT_DATA::inbound_data, 0, sizeof(FLIGHT_DATA::inbound_data));
            // We are only reading the first 50 bytes, then clearing the buffer.
            // This is because the 50-byte = 1 credit limit of the iridium modem.
            for (int i = 0; i < 50; i++) {
                if (Serial.available() > 0) {
                    FLIGHT_DATA::inbound_data[i] = Serial.read();
                } else {
                    break;
                }
            }
            process_inbound_data();
                    
        }
}
