/*
  对于壁挂炉而言, 是没有PID算法可用的, 而且为了避免壁挂炉的频繁启动, 壁挂炉关闭后至少需要3分钟时间后再打开,以避免烟气清理问题以及旁通问题.

  针对壁挂炉的算法暂时考量如下:
  壁挂炉应该考量出水温度和回水温度.
  1. 回水温度达到30度时,可以认为需要采暖的区域的地暖管内水温已经达到30度以上, 在后续的散热过程中, 应该能满足需要. 因此壁挂炉应当停止加热.
  2. 当出水温度超过60度时,可以认为壁挂炉的设定温度过高或者采暖温度传感器失效, 应停止加热, 确保安全(保护地暖管及壁挂炉本身).
  3. 壁挂炉关闭后下一次打开应至少隔3分钟时间,以保证炉膛内的废气被正确清理, 旁通应正确关闭

  对于后续的版本,如果我们弄通了OpenTherm协议,我们可以通过控制支持OpenTherm的壁挂炉采暖热水温度来实现更好的控制.
*/
#include "Boiler.h"
#include "Arduino.h"

#define MAX_TEMP_OUTPUT 60

extern uint8_t pinBoiler;
extern Thermistor thermistorIn;
extern Thermistor thermistorOut;

Boiler::Boiler()
{
  timeGap = 0;
  targetTemp = 0;
  protectPeriod = 180000UL;
}

void Boiler::setTargetTemperature(float temp) {
  targetTemp = temp;
}

void Boiler::setProtectPeriod(uint32_t period) {
  protectPeriod = period;
}

bool Boiler::hasReachedTargetTemperature() {
  if (tempIn >= getTargetTemperature()) { // 温度达到设定值
    if (hitCount > 3) { // 而且连续3个循环均符合条件
      return true; // 确认温度已经达到
    } else {
      hitCount ++;
      return false;
    }
  } else {
    hitCount = 0;
    return false;
  }
}

bool Boiler::update() {
  boolean boilerState = true;

  thermistorIn.update();
  tempIn = thermistorOut.getTemperature(); // 获取回水温度
  thermistorOut.update();
  tempOut = thermistorIn.getTemperature(); // 获取进水温度

  // 如果返回-27,代表温感探头有问题,应该停机
  if (tempIn == -27 || tempOut == -27) {
    boilerState = false;
  }
  // 壁挂炉炉温度超过60度,应当停机
  if (tempOut >= MAX_TEMP_OUTPUT) {
    boilerState = false;
  }
  // 温度已经达到要求,应该停机
  if (hasReachedTargetTemperature()) {
    boilerState = false;
  }
  // 检查待机保护时间是否达到
  if (!boilerState) {
    timeGap = millis();
  } else {
    if (millis() - timeGap < protectPeriod) {
      boilerState = false;
    }
  }

  digitalWrite(pinBoiler, boilerState);
  return boilerState;
}

