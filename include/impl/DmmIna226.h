#pragma once

#include "Dmm.h"
#include "INA226.h"

class DmmIna226: public Dmm
{
private:
    INA226 dmmModule;
public:
    DmmIna226(uint16_t address);
    bool connect() override;
    void read(DmmResult &result, uint8_t channel) override;
};