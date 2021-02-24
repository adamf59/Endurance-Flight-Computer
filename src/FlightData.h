#ifndef FLIGHT_DATA_H
#define FLIGHT_DATA_H

    namespace FLIGHT_DATA {

        // data to be sent:
        static char outboundData[50] = {0};  // we are limiting these to 50 bytes per the iridium credit limit
        
        // data we get from iridium network OR GroundLink
        static char inboundData[50] = {0};
        
        // check number we get from an RX-message (here) to send in the next TX
        static int checkNum = 0;

        // use 1013.2074 for standard atmospheric model, usually effective above FL180 (equivalent to 29.92 inHg)     
        static float obd_sea_level_pressure_hpa = 1018.6097;

    }

#endif
