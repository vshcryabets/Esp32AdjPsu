#pragma once
#include <cstdint>

struct PwmConfig
{
    uint8_t isActive;
    int16_t duty;
    int8_t channel;
    int32_t freq;
    int8_t resolution;
    int8_t pin;
};

class Pwm {
protected:
    PwmConfig config;
public:
    Pwm() = default;
    virtual ~Pwm() = default;
    virtual void start(const PwmConfig& config) = 0;
    virtual void setup(uint32_t duty) = 0;
    virtual void stop() = 0;
    virtual const PwmConfig& getConfig() const { return config; }
};

class PwmEsp32 : public Pwm {
private:
    int8_t channel;    
public:
    PwmEsp32() = default;
    ~PwmEsp32() override = default;
    void start(const PwmConfig& config) override;
    void setup(uint32_t duty) override;
    void stop() override;
};