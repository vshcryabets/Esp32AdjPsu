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

class PwmControler {
public:
    virtual ~PwmControler() = default;
    virtual void onPwmStart(const PwmConfig& config) = 0;
    virtual void onPwmUpdate(uint32_t duty) = 0;
    virtual void onPwmEnd() = 0;
    virtual const PwmConfig& getPwm() = 0;
};

class PwmEsp32 : public PwmControler {
private:
    PwmConfig config;
    int8_t channel;    
public:
    PwmEsp32() = default;
    ~PwmEsp32() override = default;
    void onPwmStart(const PwmConfig& config) override;
    void onPwmUpdate(uint32_t duty) override;
    void onPwmEnd() override;
    const PwmConfig& getPwm() override { return config; };
};