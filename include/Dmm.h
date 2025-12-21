#pragma once

#include <stdint.h>

struct DmmResult {
    float voltage {0.0f};
    float current {0.0f};
    uint32_t timestamp {0};

    bool isValid() const {
        return timestamp != 0;
    }
};

class Dmm {
    public:
        virtual bool connect() = 0;
        virtual const DmmResult read(uint8_t channel) = 0;
};
