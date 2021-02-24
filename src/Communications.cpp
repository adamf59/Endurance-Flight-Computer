#include "Communications.h"
#include "MCUHardwareMap.h"
#include "FlightData.h"

#include <SoftwareSerial.h>

SoftwareSerial iridiumModem(_HW_PIN_IRIDIUM_MODEM_RECEIVE, _HW_PIN_IRDIUM_MODEM_TRANSMIT);

void processInboundData() {
    
} 