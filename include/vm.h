#pragma once

#include <stdint.h>
#include "pwm.h"
#include "dmm.h"
#include "neural.h"

enum VMState {
    INDICATE = 1,
    CONFIGURE_V,
    CALIBRATE,
    LAST
};

class VM {
private:
    Dmm *dmmSource;    
public:
    enum VMState state;
    uint8_t isDirty;
    uint16_t configuredVoltage;
    uint16_t configuredCurrent;
    uint32_t dmmNextReadTime;
    struct DmmResult dmmResult;
    struct PwmConfig pwm;
    struct NeuralNetwork neural;
public:
    VM(Dmm *dmmSource);

    void vmOnHwReady();
    void vmOnButtons(uint8_t buttons);
    void vmUpdateState(uint32_t timestamp);
    void vmOnCalibration();
    void vmOnPwmStart(struct PwmConfig *config);
    void vmOnPwmUpdate(struct PwmConfig *config);
    void vmOnPwmEnd(struct PwmConfig *config);
    void vmOnPwmGet(struct PwmConfig *out);
};
