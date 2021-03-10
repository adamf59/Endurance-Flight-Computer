#ifndef JSFC_H
#define JSFC_H

    int main();
    void system_health_check();
    void flight_loop();
    void fillArray(char* arr, size_t arr_size, int val);

    enum system_state {
        STARTUP, POLL_WAIT_IRIDIUM, TX_RX, LOW_POWER_SLEEP
    };



    
#endif