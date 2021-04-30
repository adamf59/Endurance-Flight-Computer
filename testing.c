#include <stdio.h>
#include <stdint.h>

void process_inbound_data();

// same as 'char' in AVR-GCC compiler
uint8_t inboundTransmission[50] = {
    0xAA, 0x04, 0x02, 0xB0, 0x0B, 0xBB
};

int main() {

    if (inboundTransmission[0] != 0xAA) {
        printf("WARNING: Start byte not found at address 0. Packet is invalid.");
        return 1;
    }
    
    if (inboundTransmission[1] > 49 || inboundTransmission[1] < 1) {
        printf("WARNING: Length at address 1 is not between 1 and 49. Packet is invalid.");
        return 1;
    }

    printf("Inbound Data:\n");
    for (int i = 0; i < 50; i++) {

        if (i == 49) {
            if (inboundTransmission[i] != 0xBB) {
                printf("WARNING: Command buffer is full and no stop byte found. Packet is invalid.");
                return 1;
            }
        }

        printf("%x ", inboundTransmission[i] & 0xff);
        if (inboundTransmission[i] == 0xBB)     break;

    }
    printf("\n");

    process_inbound_data();

    printf("\nDone!");

}


void process_inbound_data() {

    int packet_length = inboundTransmission[1];
    printf("-> Searching for commands in packet of length %i\n", packet_length);
    printf("################################################\n");
    printf("This packet will do the following:\n");

    for (int i = 2; i < packet_length + 2; i++) {
        
        int command_id = inboundTransmission[i];

        switch (command_id) {
            case 0x01: {
                // set mode
                if (inboundTransmission[i + 1] == 0x01) {
                    printf("--> Set Mode to STANDBY\n");
                } else if (inboundTransmission[i + 1] == 0x02) {
                    printf("--> Set Mode to TERMINAL_COUNT\n");
                } else if (inboundTransmission[i + 1] == 0x03) {
                    printf("--> Set Mode to FLIGHT\n");
                } else {
                    printf("!! Invalid Mode Change !! Sets to %i?\n", inboundTransmission[i + 1]);
                }

                i += 1;
                break;
            }
            case 0x02: {
                uint8_t _byte_0 = inboundTransmission[i + 1]; // LSB
                uint8_t _byte_1 = inboundTransmission[i + 2]; // MSB

                uint16_t baro = (_byte_1 << 8) | _byte_0;

                printf("--> Set Barometer to %f\n", baro / 100.0);

                i += 2;
                break;

            }
            case 0x03: {
                // set ballast autopilot state
                if (inboundTransmission[i + 1] == 0xFF) {
                    printf("--> Enable Ballast Autopilot\n");
                } else {
                    printf("--> Disable Ballast Autopilot\n");
                }

                i += 1;
                break;
            }
            case 0x04: {
                uint8_t _byte_0 = inboundTransmission[i + 1];
                uint8_t _byte_1 = inboundTransmission[i + 2];
                uint16_t ballast_eval_period = (_byte_1 << 8) | _byte_0;
                printf("--> Set Ballast Eval Period to %i seconds\n", ballast_eval_period);
                i += 2;
                break;   
            }
            case 0x05: {
                printf("--> Set Ballast Coefficient to %i\n", inboundTransmission[i + 1]);
                i += 1;
                break;   
            }
            case 0x06: {
                uint8_t _byte_0 = inboundTransmission[i + 1];
                uint8_t _byte_0 = inboundTransmission[i + 1];

                break;   
            }
            case 0x07: {
                break;   
            }
            case 0x08: {
                break;   
            }
            case 0x09: {
                break;   
            }
            case 0x0A: {
                break;   
            }
            case 0x0B: {
                break;   
            }
            case 0x0C: {
                break;   
            }

            case 0xBB: {
                printf("################################################\n");
                break;
            }
        }
        
        

    }

}