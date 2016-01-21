#include <stdint.h>

class Pump {
  private:
    bool state;
    uint32_t timeGap, protectPeriod;

  public:
    Pump();

    void allow();
    void setProtectPeriod(uint32_t period);
    uint32_t getProtectPeriod() const {
      return protectPeriod;
    }
    void disable();
    bool update();
    bool getState() const {
      return state;
    }
};

