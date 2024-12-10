#ifndef ESP32_H
#define ESP32_H

#ifdef ESP32
    #ifdef __cplusplus
    extern "C" { 
    #endif

    void esp32Init(struct VM *state);
    void esp32Loop(struct VM *state);

    #ifdef __cplusplus
    }
    #endif
#endif

#endif //ESP32_H