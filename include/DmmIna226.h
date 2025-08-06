#pragma once

#include "dmm.h"
#include "INA226.h"

class DmmIna226: public Dmm
{
private:
    INA226 dmmModule;
public:
    DmmIna226(uint16_t address);
    void read(DmmResult &result, uint8_t channel) override;
};