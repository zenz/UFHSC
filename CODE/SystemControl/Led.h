#include <stdint.h>

class Led {
  private:
    uint32_t lastTime, interval;
    uint8_t digital_pin;
    bool state;

  public:
    Led(uint8_t digital_pin_out);
    void setBlinkInterval(uint32_t time);
    bool blink();
};
