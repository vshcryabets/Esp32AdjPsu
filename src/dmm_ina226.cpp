#include "dmm.h"
#include <Arduino.h>
#include "INA226.h"
#include "vm.h"


class DmmIna226
{
    private:
    INA226 dmmModule;
    public:
    DmmIna226(uint16_t address): dmmModule(address) {
        if (!dmmModule.begin()) {
            Serial.println("Can't connect INA226.");
        }
        dmmModule.setMaxCurrentShunt(0.785f, 0.1016);
        auto mode = dmmModule.getMode();
        Serial.print("Mode = ");
        Serial.println(mode);
    }
    void read(DmmResult *result, uint8_t channel) {
        result->voltage = dmmModule.getBusVoltage();
        result->current = dmmModule.getCurrent_mA()/1000.0f;
        result->timestamp = millis();
        Serial.print("Voltage = ");
        Serial.println(result->voltage);
        Serial.print("Current = ");
        Serial.println(result->current);
    }
};

extern "C" void dmmInit(struct VM *vm)
{
    Serial.print("A1");
    vm->dmmData = new DmmIna226(0x40);
    Serial.print("A2");
}

extern "C" void dmmRead(struct VM *vm, struct DmmResult *result, uint8_t channel)
{
    ((DmmIna226*)vm->dmmData)->read(result, channel);
}
