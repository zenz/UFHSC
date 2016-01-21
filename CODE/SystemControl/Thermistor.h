#ifndef THERMISTOR_IN_
#define THERMISTOR_IN_

#include <stdint.h>

#define OVERSAMPLENR 19 // 累计取样次数,必须大于7.大于19的话,看门狗的复位时间要加长,否则会触发.

class Thermistor {
  private:
    float currentTemp;
    uint8_t analog_pin;
    int16_t heatCount[OVERSAMPLENR];

  public:
    Thermistor(uint8_t analog_pin);

    float getTemperature() const {
      return currentTemp;
    }
    // True if update initiated, false otherwise
    bool update();
};

#endif //THERMISTOR_IN_
