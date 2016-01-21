#ifndef PANEL_IN_
#define PANEL_IN_

#include <stdint.h>

class Panel {
  private:
    bool state;
    uint8_t digital_pin;

  public:
    Panel(uint8_t digital_pin);

    bool getState() const {
      return !state;
    }

    bool update();
};

#endif // PANEL_IN_
