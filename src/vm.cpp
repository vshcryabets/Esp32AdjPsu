#include "vm.h"
#include "string.h"

#define DMM_READ_INTERVAL 250

void VM::init(Dmm *dmmSource, Pwm *pwm) 
{
    dmmNextReadTime = 0;
    this->dmmSource = dmmSource;
    this->pwm = pwm;
    memset(&dmmResult, 0, sizeof(dmmResult));
    memset(&neural, 0, sizeof(neural));
    neuralLoadModel(&neural);
}

void VM::onHwReady()
{
}

void VM::onButtons(uint8_t buttons)
{
    state.isDirty = 1;
}

void VM::updateState(uint32_t timestamp)
{
    if (dmmNextReadTime < timestamp)
    {
        dmmSource->read(dmmResult, 0);
        dmmNextReadTime = timestamp + DMM_READ_INTERVAL;
        state.isDirty = 1;
    }
}

void VM::onPwmStart(const PwmConfig& config)
{
    pwm->start(config);
}

void VM::onPwmUpdate(uint32_t duty)
{
    pwm->setup(duty);
}

void VM::onPwmEnd()
{
    pwm->stop();
}

const PwmConfig& VM::getPwm()
{
    return pwm->getConfig();
}

void VM::onCalibration()
{
    state.state = VMState::CALIBRATE;
    state.isDirty = 1;
}