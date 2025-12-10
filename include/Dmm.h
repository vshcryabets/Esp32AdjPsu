#pragma once

#include <stdint.h>

struct DmmResult {
    float voltage;
    float current;
    uint32_t timestamp;

    bool isValid() const {
        return timestamp != 0;
    }
};

class Dmm {
    public:
        virtual bool connect() = 0;
        virtual const DmmResult read(uint8_t channel) = 0;
};
