#include "Boiler.h"
#include "Pump.h"
#include "Led.h"

#define WORKING_TEMP 30UL
#define PROTECT_PERIOD 10000UL
#define PROCESSING_PLOT
#define DISPLAY_DELAY 1000UL
#define WATCHDOG
#define DIGITAL_TUBE

#ifdef WATCHDOG
#include <avr/wdt.h>
#endif

#ifdef DIGITAL_TUBE
// 用数字管显示进回水温度
#include "TM1637.h"
#define CLK 11
#define DIO 12
TM1637 tm1637(CLK, DIO);
#endif

// 工作灯
uint8_t pinLed = 13;
// 两个温度传感器
uint8_t pinTempOut = A0, pinTempIn = A1;
// 温控面板
uint8_t pinPanel = 2;
// 壁挂炉采暖控制
uint8_t pinBoiler = 9;
// 循环泵控制
uint8_t pinPump = 8;
uint8_t pinPumpReady = 10;
bool pumpReady;

// 初始化各个类
Led led(pinLed);
Thermistor thermistorIn(pinTempIn);
Thermistor thermistorOut(pinTempOut);
Panel panel(pinPanel);
Boiler boiler;
Pump pump;

// 定义用到的公共变量
unsigned long timeGap;
uint32_t boilerProtectPeriod, pumpProtectPeriod;

void setup() {
  analogReference(EXTERNAL);
#ifdef PROCESSING_PLOT
  Serial.begin(9600);
#endif
  pinMode(pinBoiler, OUTPUT); // 输出模式必须显性指定
  pinMode(pinPump, OUTPUT); // 输出模式必须显性指定
  pinMode(pinLed, OUTPUT);


#ifdef DIGITAL_TUBE
  // 数字管初始化
  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);
#endif

  boiler.setTargetTemperature(WORKING_TEMP);
  boiler.setProtectPeriod(PROTECT_PERIOD);
  pump.setProtectPeriod(PROTECT_PERIOD);

  pumpReady = !digitalRead(pinPumpReady);
  timeGap = 0;
#ifdef WATCHDOG
  wdt_enable(WDTO_500MS); // 500毫秒无响应视为死机,重新启动.
#endif
}

void loop() {
  bool panelState, boilerState, pumpState;

  panel.update(); // 更新面板状态
  panelState = panel.getState(); // 读取面板状态
  if (!panelState) {
    // 如果温控面板关闭了,那么无需壁挂炉启动,泵也应该停止
    led.setBlinkInterval(3000); // 闪灯频率降到每3秒一次.
    boiler.setTargetTemperature(0);
    boilerState = boiler.update();
    if (pumpReady) { // 有泵,需要进行相应处理
      pump.disable();
      pumpState = pump.update();
    } else {
      pumpState = false;
    }
  } else {
    led.setBlinkInterval(1000); // 闪灯频率变为1秒一次
    boiler.setTargetTemperature(WORKING_TEMP);
    boilerState = boiler.update();
    if (pumpReady) { // 有泵,需要进行相应处理
      pump.allow();
      pumpState = pump.update();
    } else {
      pumpState = false;
    }
  }
  // 如果温度不正确,闪灯频率变为0.2秒一次
  if (boiler.getTemperatureIn() == -27 || boiler.getTemperatureOut() == -27) {
    led.setBlinkInterval(200);
  }

  if (millis() - timeGap > DISPLAY_DELAY) {  // 根据定义的显示间隔,输出一次当前检测的温度.

#ifdef PROCESSING_PLOT
    Serial.print(boiler.getTemperatureOut());
    Serial.print(",");
    Serial.print(boiler.getTemperatureIn());
    Serial.print(",");
    Serial.print(pumpState);
    Serial.print(",");
    Serial.println(boilerState);
#endif

#ifdef DIGITAL_TUBE
    tm1637.display(0, round(boiler.getTemperatureOut()) / 10);  // 壁挂炉出水温度十位
    tm1637.display(1, round(boiler.getTemperatureOut()) % 10);  // 壁挂炉出水温度个位
    tm1637.display(2, round(boiler.getTemperatureIn()) / 10);   // 壁挂炉回水温度十位
    tm1637.display(3, round(boiler.getTemperatureIn()) % 10);   // 壁挂炉回水温度各位
    tm1637.point(POINT_ON); // 显示分割符号
#endif

    timeGap = millis();
  }

  led.blink(); // 更新闪灯状态

#ifdef WATCHDOG
  wdt_reset(); // 喂狗,防止重启
#endif
}
