#include "Led.h"
#include <Arduino.h>

Led::Led(uint8_t digital_pin_out):
  digital_pin(digital_pin_out)
{
  interval = 2000; // 默认2秒闪一下
  lastTime = 0;
  state = false;
}

void Led::setBlinkInterval(uint32_t time) {
  interval = time;
}

bool Led::blink() {
  if ( millis() - lastTime >= interval) {
    lastTime = millis();
    state = !state;
    digitalWrite(digital_pin, state);
  }
}

