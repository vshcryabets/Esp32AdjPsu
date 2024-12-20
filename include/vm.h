#ifndef VIEWMODEL_H
#define VIEWMODEL_H

#include <stdint.h>
#include "pwm.h"

#ifdef __cplusplus
extern "C" { 
#endif

enum VMState {
    INDICATE = 1,
    CONFIGURE_V,
    LAST
};

struct VM {
    enum VMState state;
    uint8_t isDirty;
    uint16_t innVoltage;
    uint16_t innCurrent;
    uint16_t configuredVoltage;
    uint16_t configuredCurrent;
    uint16_t configuredPwm;
};

void vmInit(struct VM *vm);
void vmOnButtons(struct VM *vm, uint8_t buttons);
void vmUpdateState(struct VM *vm);

void vmOnPwmStart(struct PwmConfig *config);
void vmOnPwmUpdate(struct PwmConfig *config);
void vmOnPwmEnd(struct PwmConfig *config);


#ifdef __cplusplus
}
#endif

#endif