#include "Pump.h"
#include "Arduino.h"

extern uint8_t pinPump;

Pump::Pump()
{
  state = false;
  timeGap = 0;
  protectPeriod = 180000UL;
}

void Pump::setProtectPeriod(uint32_t period){
  protectPeriod = period;
}

void Pump::allow()
{
  if (millis() - timeGap > protectPeriod) {
    state = true;
  }
}

void Pump::disable()
{
  state = false;
  timeGap = millis();
}

bool Pump::update()
{
  digitalWrite(pinPump, state);
  return state;
}

