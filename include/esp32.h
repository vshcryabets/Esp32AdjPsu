#ifndef ESP32_H
#define ESP32_H
#include <stdint.h>

#ifdef ESP32
    #ifdef __cplusplus
    struct EspState {
        uint32_t dmmSendTime;
    };
    extern "C" { 
    #endif

    void esp32Init(struct VM *state);
    void esp32Loop(struct VM *state);

    #ifdef __cplusplus
    }
    #endif
#endif

#endif //ESP32_H