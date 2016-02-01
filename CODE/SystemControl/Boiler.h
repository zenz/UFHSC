#include "Thermistor.h"
#include "PID_v1.h"

class Boiler {
  private:
    float tempIn, tempOut, targetTemp; // 壁挂炉侧的回水温度,出水温度,目标回水温度
    uint32_t timeGap, protectPeriod;
    double Kp = 20, Ki = 0.1, Kd = 1; // 根据现场实测调整
    double Setpoint, Input, Output;
    PID* boilerPID;

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
    bool update();
};

