#include "vm.h"
#include "config.h"
#include "string.h"

#define DMM_READ_INTERVAL 250

void vmInit(struct VM *vm)
{
    memset(vm, 0, sizeof(struct VM));
    vm->state = INDICATE;
    vm->dmmNextReadTime = 0;
    vm->isDirty = 1;
}

void vmOnHwReady(struct VM *vm)
{
    dmmInit(vm);
    vm->isDirty = 1;
}

void vmOnButtons(struct VM *vm, uint8_t buttons)
{    
    vm->isDirty = 1;
}

void vmUpdateState(struct VM *vm, uint32_t timestamp)
{
    if (vm->dmmNextReadTime < timestamp)
    {
        dmmRead(vm, &vm->dmmResult, 0);
        vm->dmmNextReadTime = timestamp + DMM_READ_INTERVAL;
        vm->isDirty = 1;
    }
}

void vmOnPwmStart(struct VM *vm, struct PwmConfig *config)
{
    config->work = 1;
    memcpy(&vm->pwm, config, sizeof(struct PwmConfig));
    pwmStart(config);
}

void vmOnPwmUpdate(struct VM *vm, struct PwmConfig *config)
{
    vm->pwm.duty = config->duty;
    pwmSetup(config);
}

void vmOnPwmEnd(struct VM *vm, struct PwmConfig *config)
{
    config->work = 0;
    config->pin = 0;
    config->freq = 0;
    memcpy(&vm->pwm, config, sizeof(struct PwmConfig));
    pwmStop(config);
}

void vmOnPwmGet(struct VM *vm, struct PwmConfig *out) 
{
    memcpy(out, &vm->pwm, sizeof(struct PwmConfig));
}