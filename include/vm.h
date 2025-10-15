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

class VmPwm {
public:
    virtual void onPwmStart(const PwmConfig& config) = 0;
    virtual void onPwmUpdate(uint32_t duty) = 0;
    virtual void onPwmEnd() = 0;
    virtual const PwmConfig& getPwm() = 0;
};

class VM: public VmPwm {
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
    void onPwmStart(const PwmConfig& config) override;
    void onPwmUpdate(uint32_t duty) override;
    void onPwmEnd() override;
    const PwmConfig& getPwm() override;
};
