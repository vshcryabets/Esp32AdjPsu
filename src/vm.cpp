#include "vm.h"
#include "string.h"

#define DMM_READ_INTERVAL 250

VM::VM(Dmm *dmmSource) : state(INDICATE),
                         isDirty(1),
                         configuredVoltage(0),
                         configuredCurrent(0),
                         dmmNextReadTime(0),
                         dmmSource(dmmSource)
{
    memset(&dmmResult, 0, sizeof(dmmResult));
    memset(&pwm, 0, sizeof(pwm));
    memset(&neural, 0, sizeof(neural));
    neuralLoadModel(&neural);
}

void VM::vmOnHwReady()
{
}

void VM::vmOnButtons(uint8_t buttons)
{
    isDirty = 1;
}

void VM::vmUpdateState(uint32_t timestamp)
{
    if (dmmNextReadTime < timestamp)
    {
        dmmSource->read(dmmResult, 0);
        dmmNextReadTime = timestamp + DMM_READ_INTERVAL;
        isDirty = 1;
    }
}

void VM::vmOnPwmStart(struct PwmConfig *config)
{
    config->work = 1;
    memcpy(&pwm, config, sizeof(struct PwmConfig));
    pwmStart(config);
}

void VM::vmOnPwmUpdate(struct PwmConfig *config)
{
    pwm.duty = config->duty;
    pwmSetup(config);
}

void VM::vmOnPwmEnd(struct PwmConfig *config)
{
    config->work = 0;
    config->pin = 0;
    config->freq = 0;
    memcpy(&pwm, config, sizeof(struct PwmConfig));
    pwmStop(config);
}

void VM::vmOnPwmGet(struct PwmConfig *out)
{
    memcpy(out, &pwm, sizeof(struct PwmConfig));
}

void VM::vmOnCalibration()
{
    state = CALIBRATE;
    isDirty = 1;
}