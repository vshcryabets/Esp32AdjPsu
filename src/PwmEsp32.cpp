#include "pwm.h"
#include <Arduino.h>

void PwmEsp32::onPwmStart(const PwmConfig& config) {
    this->config = config;
    this->config.isActive = 1;
    auto err = ledcSetup(config.channel, config.freq, config.resolution);
    Serial.print("ledcSetup=");
    Serial.println(err);
    ledcAttachPin(config.pin, config.channel);
    ledcWrite(config.channel, config.duty);
    this->channel = config.channel;
}

void PwmEsp32::onPwmUpdate(uint32_t duty) {
    this->config.duty = duty;
    ledcWrite(channel, duty);
}
void PwmEsp32::onPwmEnd() {
    this->config.duty = 0;
    this->config.isActive = 0;
    ledcWrite(channel, 0);
}
