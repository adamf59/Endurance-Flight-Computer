#include "Arduino.h"
#include "Communications.h"
#include "MCUHardwareMap.h"
#include "FlightData.h"
#include "jsfc.h"
#include "SensorSystem.h"
#include "EEPROM.h"

#include <SoftwareSerial.h>

#include <stdint.h>

// Function pointer to built-in restart function. EMERGENCY ONLY!
void(* reset_flight_computer) (void) = 0;

#define MAXIMUM_IRIDIUM_TRANSMIT_DURATION       30000

using namespace FLIGHT_DATA;

//http://ugweb.cs.ualberta.ca/~c274/resources/arduino-ua/avr-libc-1.7.1-overlay/avr-libc/avr-libc-user-manual-1.7.1/group__util__crc.html

SoftwareSerial iridiumModem(_HW_PIN_IRIDIUM_MODEM_RECEIVE, _HW_PIN_IRDIUM_MODEM_TRANSMIT);



void _com_init() {
    iridiumModem.begin(19200);
    if (system_mode == 0) {
        Serial.begin(19200);
    }
}

void collect_data_for_tx() {
    memset(outbound_data, 0, sizeof(outbound_data));
    
    //Collect Temperatures
    uint16_t bme280_temperature_measurement = (uint16_t) (_read_sen_bme280_temp() * 100);
    uint16_t bmp280_temperature_measurement = (uint16_t) (_read_sen_bmp280_temp() * 100);
    // uint16_t dht22_temperature_measurement = (uint16_t) (_read_sen_dht22_temp() * 100);
    uint16_t mpl3115a2_temperature_measurement = (uint16_t) (_read_sen_mpl3115a2_temp() * 100);
    uint16_t ds18b20_temperature_measurement = (uint16_t) (_read_sen_ds18b20_temp() * 100);

    // Collect Pressures:
    uint32_t bme280_pressure_measurement =  _read_sen_bme280_pressure();
    uint32_t bmp280_pressure_measurement = _read_sen_bmp280_pressure();
    uint32_t mpl3115a2_pressure_measurement = _read_sen_mpl3115a2_pressure();

    // Compute the current Altitude
    current_altitude = compute_altitude(((bme280_pressure_measurement + bmp280_pressure_measurement + mpl3115a2_pressure_measurement) / 3));

    //Collect Power System
    uint16_t ina260_voltage_measuremnet = _read_sen_ina260_voltage();
    
    uint32_t ina260_current_measurement = _read_sen_ina260_current();

    // Structure the response packet:

    outbound_data[0] = 0xAA; // Start Byte
    outbound_data[1] = 0x00; // TODO Packet Checksum
    outbound_data[2] = hardware_status_bitfield;

    // Sea Level Pressure
    outbound_data[4] = (uint8_t) sea_level_pressure_pascals & 0xFF;
    outbound_data[5] = (uint8_t) (sea_level_pressure_pascals >> 8) & 0xFF;
    outbound_data[6] = (uint8_t) (sea_level_pressure_pascals >> 16) & 0xFF;

    outbound_data[7] = (uint8_t) sea_level_temperature & 0xFF;
    outbound_data[8] = (uint8_t) (sea_level_temperature >> 8) & 0xFF;

    outbound_data[9] = last_transmission_status;

    // Temperatures:
    outbound_data[10] = (uint8_t) bme280_temperature_measurement & 0xFF;
    outbound_data[11] = (uint8_t) (bme280_temperature_measurement >> 8);

    outbound_data[12] = (uint8_t) bmp280_temperature_measurement & 0xFF;
    outbound_data[13] = (uint8_t) (bmp280_temperature_measurement >> 8);

    // outbound_data[14] = (uint8_t) dht22_temperature_measurement & 0xFF;
    // outbound_data[15] = (uint8_t) (dht22_temperature_measurement >> 8);

    outbound_data[16] = (uint8_t) mpl3115a2_temperature_measurement & 0xFF;
    outbound_data[17] = (uint8_t) (mpl3115a2_temperature_measurement >> 8);

    outbound_data[18] = (uint8_t) ds18b20_temperature_measurement & 0xFF;
    outbound_data[19] = (uint8_t) (ds18b20_temperature_measurement >> 8);
    
    // Pressures:
    outbound_data[20] = (uint8_t) bme280_pressure_measurement & 0xFF;
    outbound_data[21] = (uint8_t) (bme280_pressure_measurement >> 8) & 0xFF;
    outbound_data[22] = (uint8_t) (bme280_pressure_measurement >> 16) & 0xFF;

    outbound_data[23] = (uint8_t) bmp280_pressure_measurement & 0xFF;
    outbound_data[24] = (uint8_t) (bmp280_pressure_measurement >> 8) & 0xFF;
    outbound_data[25] = (uint8_t) (bmp280_pressure_measurement >> 16) & 0xFF;

    outbound_data[26] = (uint8_t) mpl3115a2_pressure_measurement & 0xFF;
    outbound_data[27] = (uint8_t) (mpl3115a2_pressure_measurement >> 8) & 0xFF;
    outbound_data[28] = (uint8_t) (mpl3115a2_pressure_measurement >> 16) & 0xFF;

    // Humidity
    outbound_data[30] = _read_sen_bme280_rhumidity();
    // outbound_data[31] = _read_sen_dht22_rhumidity();

    // Power System:
    outbound_data[32] = (uint8_t) ina260_voltage_measuremnet & 0xFF;
    outbound_data[33] = (uint8_t) (ina260_voltage_measuremnet >> 8);
    
    outbound_data[34] = (uint8_t) ina260_current_measurement & 0xFF;
    outbound_data[35] = (uint8_t) (ina260_current_measurement >> 8) & 0xFF;
    outbound_data[36] = (uint8_t) (ina260_current_measurement >> 16) & 0xFF;
    

    outbound_data[39] = strobe_light_status;
    outbound_data[40] = iridium_mt_queued;

    outbound_data[42] = (uint8_t) ballast_autopilot_drop_time & 0xFF;
    outbound_data[43] = (uint8_t) (ballast_autopilot_drop_time >> 8);

    outbound_data[47] = rx_check_num;
    outbound_data[49] = 0xBB;

    // After Transmission:
    rx_check_num = 0;

}

/**
 * Parses and runs commands contained in the inbound_data buffer.
 */
void process_inbound_data() {

    uint8_t force_transmission_flag = 0;
    
    // Valid packets have the start byte 0xAA at position 0.
    if (inbound_data[0] != 0xAA) return;
    if (inbound_data[(inbound_data[1] + 1)] != 0xBB) return;
    
    for (int i = 2; i < inbound_data[1] + 2; i++) {

        if (inbound_data[i] == 0x01) {
            // Set the Mode
            system_mode = inbound_data[i + 1];

            // Prep for Set Mode
            if (inbound_data[i + 1] == 0x01) {
                // Close GroundLink. It'll no longer be used.
                Serial.end();
            } else if (inbound_data[i + 1] == 0x02) {
                collect_data_for_tx();
                run_iridium_tx_rx_sequence();
                
            }
            
            // Write the mode to the EEPROM in case we restart
            EEPROM.write(EEPROM_SYSTEM_MODE_ADDR, system_mode);

            i += 1;
        } else if (inbound_data[i] == 0x02) {

            // Set Sea-Level Barometer for altitude computations
            sea_level_pressure_pascals = (inbound_data[i + 3] << 16) | (inbound_data[i + 2] << 8) | inbound_data[i + 1];
            i += 3;
            
        } else if (inbound_data[i] == 0x03) {

            // Set Ballast Autopilot State
            ballast_autopilot_status = inbound_data[i + 1];
            i += 1;
            
        } else if (inbound_data[i] == 0x04) {

            // Set Ballast Evaluation Period
            iridium_transmit_interval = (inbound_data[i + 2] << 8) | inbound_data[i + 1];
            i += 2;
            
        } else if (inbound_data[i] == 0x05) {

            // Set Ballast Coefficient
            ballast_autopilot_coefficient = inbound_data[i + 1];
            i += 1;
            
        } else if (inbound_data[i] == 0x06) {

            // Set Lower Altitude Threshold    
            autopilot_lower_alitude_threshold = (inbound_data[i + 2] << 8) | inbound_data[i + 1];
            i += 2;
            
        } else if (inbound_data[i] == 0x07) {

            // Set Upper Altitude Threshold    
            autopilot_upper_altitude_threshold = (inbound_data[i + 2] << 8) | inbound_data[i + 1];
            i += 2;
            
        } else if (inbound_data[i] == 0x08) {

            // Manual Ballast Dispense
            digitalWrite(_HW_PIN_BALLAST_TRIGGER, HIGH);
            delay(inbound_data[i+1] * 1000);
            digitalWrite(_HW_PIN_BALLAST_TRIGGER, LOW);
            i += 1;
            
        } else if (inbound_data[i] == 0x09) {

            set_strobes(inbound_data[i + 1]);
            i += 1;
            
        } else if (inbound_data[i] == 0x0A) {
            
            altitude_zone_constants[0] = inbound_data[i + 1];
            altitude_zone_constants[1] = inbound_data[i + 2];
            altitude_zone_constants[2] = inbound_data[i + 3];
            altitude_zone_constants[3] = inbound_data[i + 4];
            altitude_zone_constants[4] = inbound_data[i + 5];
            altitude_zone_constants[5] = inbound_data[i + 6];
            altitude_zone_constants[6] = inbound_data[i + 7];

            for (int i = 0; i < 7; i++) {
                Serial.println(altitude_zone_constants[i]);
            } 

            i += 7;            
        } else if (inbound_data[i] == 0x0B) {

            // Set RX Check Number
            rx_check_num = inbound_data[i + 1];
            i += 1;
            
        } else if (inbound_data[i] == 0x0C) {
            
            // Set the force transmission flag to 1. The forced transmission will be sent after this function returns.
            force_transmission_flag = 1;

        } else if (inbound_data[i] == 0x0D) {

            sea_level_temperature = (inbound_data[i + 2] << 8) | inbound_data[i + 1];
            i += 2;
        } else if (inbound_data[i] == 0x0E && system_mode == 0x00) {
            // We only can use this command in ground mode, as serial is closed otherwise.
            // Collect the most recent data for dumping
            collect_data_for_tx();
            
            // Dump Flight Data to GroundLink
            for (uint8_t i = 0; i < 50; i++) {
                Serial.write(FLIGHT_DATA::outbound_data[i]);
            }
            
        } else if (inbound_data[i] == 0xF8) { // Emergency Commands Below

            // Restart Flight Computer
            if (inbound_data[i + 1] == 0xA8) {
                    // Welp, guess we're restarting...
                    reset_flight_computer();
                }
            
        } else if (inbound_data[i] == 0xF9) {

            // Trigger Flight Termination System
            if (inbound_data[i + 1] == 0x55) {
                        /*
                         * Unfortunatly, this is it. Trigger the flight termination system,
                         * which should cut down the payload.
                         * We'll run the system for 8 seconds.
                         * Hopefully, this block of code NEVER gets run.
                         */

                        // rip
                        digitalWrite(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_A, HIGH);
                        digitalWrite(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_B, HIGH);
                        delay(8000);            // TODO we should not use delay().
                        digitalWrite(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_A, LOW);
                        digitalWrite(_HW_PIN_FLIGHT_TERMINATION_SYSTEM_CH_B, LOW);
                    }
                    i += 1;
            
        } else if (inbound_data[i] == 0xBB) {            
            break;
        } else {
            // all else (command unrecognized), break
            break;
        }
        
    }

    force_transmission = force_transmission_flag;
    
}

/**
 * Transmits data in the outbound_data buffer to the iridium network.
 * Returns 0 if transmission was unsuccessful, 1 if the transmission was successful and no commands are waiting, 2 if successful and commands are waiting.
 */
uint8_t transmit_outbound() {
    // Start a session with the iridium modem:

    // check connectivity
    if (!check_iridium_ready()) {
        return 0;
    }

    // disable flow control
    if (strcmp(send_modem_command("AT&K0\r", 50, 0), "OK\r") == 0) {
        return 0;
    }

    // request transfer of binary data to ISU
    if (strcmp(send_modem_command("AT+SBDWB=50\r", 50, 0), "READY\r") == 0) {
        return 0;
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
    read_iridium_buffer(0);
    if (!strcmp(iridiumRecieveBufferData, "0OK\r")) {
        return 0;
    }
    
    // Finally, initiate a short burst session:
    iridiumModem.write("AT+SBDIX\r");

    long transmit_start_time = millis();
    while((millis() - transmit_start_time) < MAXIMUM_IRIDIUM_TRANSMIT_DURATION) {
        // hold until we get a response from the ISU, or a timer elapses (ISU potentially frozen)
        
        // check for data in buffer. We are looking for at least 15 definitly recieved bytes.
        if (iridiumModem.available() > 15) {
            break;
        }
        delay(500); // wait a bit to see if the transmission was successful.
    }

    // fetch the data into the buffer
    read_iridium_buffer(0);
    // Serial.println("Data:");
    // Serial.println(iridiumRecieveBufferData);   // debug print
    uint8_t data_variable = 0;
    uint8_t data_position = 0;
    char data[8] = { 0 };

    uint8_t data_waiting = 0;

    for (uint8_t i = 7; i < strlen(iridiumRecieveBufferData) + 1; i++) {
        char cHere = iridiumRecieveBufferData[i];
        if (cHere == ',' || i == strlen(iridiumRecieveBufferData)) {
            
            //See https://www.rock7.com/downloads/ATC_Iridium_ISU_AT_Command_Reference_MAN0009_v5.pdf page 112-114
            if (data_variable == 0 && atoi(data) > 4) {
                return 0; // Transmission failed!

            } else if (data_variable == 2 && atoi(data) == 1) {
                data_waiting = 2;
            } else if (data_variable == 5) {
                iridium_mt_queued = atoi(data);
            }

            data_position = 0;
            memset(data, 0 , 8);
            data_variable++;
            
        } else {
            if (data_position < 8) { // out of bounds memory catch
                data[data_position++] = cHere;
            }
            
        }
    }


    return (data_waiting == 2) ? 2 : 1;
}

void run_iridium_tx_rx_sequence() {

    // Turn on the Iridium Modem
    digitalWrite(_HW_PIN_IRIDIUM_MODEM_SLEEP, HIGH);

    // Collect Data
    collect_data_for_tx();
    // wait for iridium to be ready
    while (!check_iridium_ready()) { delay(500); };
    
    do {
        uint8_t tx_status = 0;
        for (int i = 0; i < 3; i++) {
            tx_status = transmit_outbound();
            Serial.println("Response");
            Serial.println(tx_status);
            if (tx_status != 0) break;
        }
        last_transmission_status = tx_status;

        if (tx_status == 2) {
            Serial.println("DATA AVAILABLE FROM IRIDIUM");
            // read data from modem
            send_modem_command("AT+SBDRB\r", 600, 1);
            uint8_t messageLength = FLIGHT_DATA::iridiumRecieveBufferData[1];

            memcpy(FLIGHT_DATA::inbound_data, &FLIGHT_DATA::iridiumRecieveBufferData[2], messageLength);
            process_inbound_data();
        } else {
            FLIGHT_DATA::force_transmission = 0; // If no data is waiting, then obviously we don't need to transmit again.
            break;
        }

    } while(FLIGHT_DATA::force_transmission);

    // Put Iridium Modem back into Sleep Mode
    digitalWrite(_HW_PIN_IRIDIUM_MODEM_SLEEP, LOW);

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
    return strcmp(send_modem_command("AT\r", 50, 0), "OK") == 0;
}

/**
 * Sends a message to the iridium modem, and stores the response into iridiumRecieveBufferData
 */
char* send_modem_command(char transmission[], int read_timeout, int read_all) {

    // Clear the buffers
    flush_iridium_recieve_buffer();
    flush_iridium_serial_buffer();

    // Write the transmission
    iridiumModem.write(transmission);

    // Wait for the modem to respond
    // TODO there may be a better way to do this.
    delay(read_timeout);

    read_iridium_buffer(read_all);

    return iridiumRecieveBufferData;
    
}

void read_iridium_buffer(int read_all) {

    int writeIdx = 0;
    // Read any data from the iridium modem
    while (iridiumModem.available()) {
        if (writeIdx == sizeof(iridiumRecieveBufferData)) break;   // prevent buffer overflows
        char inChar = iridiumModem.read();
        // only accepting ASCII letters, numbers, etc. (i.e. no control characters like \0, NL, CR, etc.)
        if(read_all) {
            iridiumRecieveBufferData[writeIdx++] = inChar;
        } else {
            if (inChar >= 32 && inChar <= 122)  iridiumRecieveBufferData[writeIdx++] = inChar;
        }
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
