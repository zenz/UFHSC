#include "Thermistor.h"
#include "Panel.h"

class Boiler {
  private:
    uint8_t hitCount;
    float tempIn, tempOut, targetTemp; // 壁挂炉侧的回水温度,出水温度,目标回水温度
    uint32_t timeGap, protectPeriod;

  public:
    Boiler();

    void setTargetTemperature(float temp);
    void setProtectPeriod(uint32_t period);
    uint32_t getProtectPeriod() const {
      return protectPeriod;
    }
    float getTargetTemperature() const {
      return targetTemp;
    }
    float getTemperatureIn() const {
      return tempIn;
    }
    float getTemperatureOut() const {
      return tempOut;
    }
    bool hasReachedTargetTemperature();
    bool update();
};

