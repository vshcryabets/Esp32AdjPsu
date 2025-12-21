#pragma once

#include <stdint.h>
#include "pwm.h"
#include "Dmm.h"
#include "neural.h"

#define MAX_RECEIVERS 4

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
    bool dmmConnected {false};
    uint32_t dmmNextReadTime {0};
    DmmResult dmmResult;
};


class VmStateReceiver {
public:
    virtual void onStateChanged(const State& state) = 0;
};

class VmStateProvider {
public:
    virtual const State& getState() = 0;
    virtual bool subscribe(VmStateReceiver* receiver) = 0;
    virtual void unsubscribe(const VmStateReceiver* receiver) = 0;
};

class VM: public PwmControler, public VmStateProvider {
private:
    Dmm *dmmSource;
    PwmControler *pwm;
    VmStateReceiver* stateReceivers[MAX_RECEIVERS] = {nullptr, nullptr, nullptr, nullptr};
public:
    NeuralNetwork neural;
    State state;
public:
    VM() = default;
    void init(Dmm *dmmSource, PwmControler *pwm);
    void onHwReady();
    void onButtons(uint8_t buttons);
    void updateState(uint32_t timestamp);
    void onCalibration();
    void onPwmStart(const PwmConfig& config) override;
    void onPwmUpdate(uint32_t duty) override;
    void onPwmEnd() override;
    const PwmConfig& getPwm() override;
    const State& getState() override {
        return state;
    }
    bool subscribe(VmStateReceiver* receiver) override;
    void unsubscribe(const VmStateReceiver* receiver) override;
private:
    void notifyStateChanged();
};
