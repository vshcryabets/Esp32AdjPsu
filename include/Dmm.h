#pragma once

#include <stdint.h>

struct DmmResult {
    float voltage;
    float current;
    uint32_t timestamp;
};

class Dmm {
    public:
        virtual bool connect() = 0;
        virtual void read(DmmResult &result, uint8_t channel) = 0;
};
