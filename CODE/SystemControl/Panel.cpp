#include "Panel.h"
#include <Arduino.h>

Panel::Panel(uint8_t digital_pin_in):
  digital_pin(digital_pin_in) {
  state = false;
}

bool Panel::update() {
  state = digitalRead(digital_pin);
  return true;
}

