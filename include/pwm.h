#ifndef PWM_H
#define PWM_H

#include "vm.h"

struct PwmConfig
{
    int8_t work;
    int16_t duty;
#ifdef STM8
#elif defined(ESP32)
    int8_t channel;
    int32_t freq;
    int8_t resolution;
    int8_t pin;
#endif    
};


#ifdef __cplusplus
extern "C" { 
#endif
    void pwmStart(struct PwmConfig* config);
    void pwmSetup(struct PwmConfig* config);
    void pwmStop(struct PwmConfig* config);
#ifdef __cplusplus
}
#endif

#endif //ESP32_H