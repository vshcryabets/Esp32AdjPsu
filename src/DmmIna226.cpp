#include "DmmIna226.h"

DmmIna226::DmmIna226(uint16_t address): dmmModule(address) {
    if (!dmmModule.begin()) {
        Serial.println("Can't connect INA226.");
    }
    dmmModule.setMaxCurrentShunt(0.785f, 0.1016);
    auto mode = dmmModule.getMode();
    // Serial.print("Mode = ");
    // Serial.println(mode);
}

void DmmIna226::read(DmmResult &result, uint8_t channel) {
    result.voltage = dmmModule.getBusVoltage();
    result.current = dmmModule.getCurrent_mA()/1000.0f;
    result.timestamp = millis();
    // Serial.print("Voltage = ");
    // Serial.println(result->voltage);
    // Serial.print("Current = ");
    // Serial.println(result->current);
}
