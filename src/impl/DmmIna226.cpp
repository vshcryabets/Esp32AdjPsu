#include "impl/DmmIna226.h"

DmmIna226::DmmIna226(uint16_t address): dmmModule(address) {
}

bool DmmIna226::connect() {
    if (!dmmModule.begin()) {
        Serial.println("Can't connect INA226.");
        return false;
    }
    dmmModule.setMaxCurrentShunt(0.785f, 0.1016);
    auto mode = dmmModule.getMode();
    // Serial.print("Mode = ");
    // Serial.println(mode);
    return true;
}

const DmmResult DmmIna226::read(uint8_t channel) {
    DmmResult result;
    result.voltage = dmmModule.getBusVoltage();
    result.current = dmmModule.getCurrent_mA()/1000.0f;
    result.timestamp = millis();
    // Serial.print("Current = ");
    // Serial.print("Voltage = ");
    // Serial.println(result->voltage);
    // Serial.println(result->current);
    return result;
}
