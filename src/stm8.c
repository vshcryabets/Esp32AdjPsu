#include "config.h"

#ifdef STM8
void toneCustom(uint16_t freq) {
  if (freq == 0) {
    return;
  }
  uint16_t arr = 1000000 / freq;
  TIM2_OC1Init(TIM2_OCMODE_PWM1,
               TIM2_OUTPUTSTATE_ENABLE, arr / 2,
               TIM2_OCPOLARITY_HIGH);
  TIM2_TimeBaseInit(TIM2_PRESCALER_16, arr);
  TIM2_Cmd(ENABLE);  
}

void noToneCustom() {
  TIM2_Cmd(DISABLE);
}

void stm8Init()) {
#ifdef EN_BUZZER
  // prepare buzzer
  GPIO_DeInit(GPIOD);
  TIM2_DeInit();
  GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);
#endif
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
}

void playLoseTune() {
  for (uint8_t note = 0; note < 3; note++) {
    toneCustom(400 - note * 40);
    delay(1100);
    noToneCustom();
  }
}
#endif