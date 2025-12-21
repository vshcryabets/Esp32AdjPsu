#include "vm.h"
#include "string.h"

#define DMM_READ_INTERVAL 250

void VM::init(Dmm *dmmSource, PwmControler *pwm) 
{
    this->dmmSource = dmmSource;
    this->pwm = pwm;
    memset(&neural, 0, sizeof(neural));
    neuralLoadModel(&neural);
    notifyStateChanged();
}

void VM::onHwReady()
{
    this->state.dmmConnected = dmmSource->connect();
    notifyStateChanged();
}

void VM::onButtons(uint8_t buttons)
{
    notifyStateChanged();
}

void VM::updateState(uint32_t timestamp)
{
    if (state.dmmNextReadTime < timestamp)
    {
        state.dmmResult = dmmSource->read(0);
        state.dmmNextReadTime = timestamp + DMM_READ_INTERVAL;
        notifyStateChanged();
    }
}

void VM::onPwmStart(const PwmConfig& config)
{
    pwm->onPwmStart(config);
}

void VM::onPwmUpdate(uint32_t duty)
{
    pwm->onPwmUpdate(duty);
}

void VM::onPwmEnd()
{
    pwm->onPwmEnd();
}

const PwmConfig& VM::getPwm()
{
    return pwm->getPwm();
}

void VM::onCalibration()
{
    state.state = VMState::CALIBRATE;
    notifyStateChanged();
}

bool VM::subscribe(VmStateReceiver* receiver) {
    for (size_t i = 0; i < MAX_RECEIVERS; i++) {
        if (stateReceivers[i] == receiver) {
            // Receiver already subscribed; avoid duplicate entries.
            return false;
        }
    }
    bool added = false;
    for (size_t i = 0; i < MAX_RECEIVERS; i++) {
        if (stateReceivers[i] == nullptr) {
            stateReceivers[i] = receiver;
            added = true;
            break;
        }
    }
    return added;
}

void VM::unsubscribe(const VmStateReceiver* receiver) {
    for (size_t i = 0; i < MAX_RECEIVERS; i++) {
        if (stateReceivers[i] == receiver) {
            stateReceivers[i] = nullptr;
            break;
        }
    }
}

void VM::notifyStateChanged() {
    state.isDirty = 1;
    for (size_t i = 0; i < MAX_RECEIVERS; i++) {
        if (stateReceivers[i] != nullptr) {
            stateReceivers[i]->onStateChanged(state);
        }
    }
}