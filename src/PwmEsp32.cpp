#include "pwm.h"
#include <Arduino.h>

void PwmEsp32::onPwmStart(const PwmConfig& config) {
    this->config = config;
    this->config.isActive = 1;
    ledcSetup(config.channel, config.freq, config.resolution);
    ledcAttachPin(config.pin, config.channel);
    ledcWrite(config.channel, config.duty);
}

void PwmEsp32::onPwmUpdate(uint32_t duty) {
    this->config.duty = duty;
    ledcWrite(this->config.channel, duty);
}
void PwmEsp32::onPwmEnd() {
    this->config.duty = 0;
    this->config.isActive = 0;
    ledcWrite(this->config.channel, 0);
}
