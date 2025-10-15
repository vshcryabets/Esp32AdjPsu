#pragma once

#include <stdint.h>
#include "pwm.h"
#include "dmm.h"
#include "neural.h"

enum class VMState: uint8_t {
    INDICATE = 1,
    CONFIGURE_V,
    CALIBRATE,
    LAST
};

class State {
public:
    VMState state = VMState::INDICATE;
    uint8_t isDirty {1};
    uint16_t configuredVoltage {0};
    uint16_t configuredCurrent {0};
};

class VM {
private:
    Dmm *dmmSource;
    Pwm *pwm;
public:
    uint32_t dmmNextReadTime;
    DmmResult dmmResult;
    NeuralNetwork neural;
    State state;
public:
    VM() = default;
    void init(Dmm *dmmSource, Pwm *pwm);
    void onHwReady();
    void onButtons(uint8_t buttons);
    void updateState(uint32_t timestamp);
    void onCalibration();
    void onPwmStart(const PwmConfig& config);
    void onPwmUpdate(uint32_t duty);
    void onPwmEnd();
    const PwmConfig& getPwm();
};
