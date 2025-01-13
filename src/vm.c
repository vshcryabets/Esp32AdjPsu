#include "vm.h"
#include "config.h"

void vmInit(struct VM *vm)
{
    vm->state = INDICATE;
    vm->isDirty = 1;
}

void vmOnButtons(struct VM *vm, uint8_t buttons)
{
    
    vm->isDirty = 1;
}

void vmUpdateState(struct VM *vm)
{
}

void vmOnPwmStart(struct VM *vm, struct PwmConfig *config)
{
    config->work = 1;
    pwmStart(config);
}

void vmOnPwmUpdate(struct VM *vm, struct PwmConfig *config)
{
    pwmSetup(config);
}

void vmOnPwmEnd(struct VM *vm, struct PwmConfig *config)
{
    config->work = 0;
    pwmStop(config);
}