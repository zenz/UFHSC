#include "Boiler.h"
#include "Pump.h"
#include "Led.h"

#define WORKING_TEMP 30UL
#define PROTECT_PERIOD 10000UL
#define PROCESSING_PLOT
#define DISPLAY_DELAY 60000UL
#define WATCHDOG
//#define DIGITAL_TUBE
#define ESPWIFI // 如果开启了ESPWIFI,则会自动关闭PROCESSING_PLOT和WATCHDOG

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

#ifdef ESPWIFI
#undef PROCESSING_PLOT
#undef WATCHDOG
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#include "WiFiEsp.h"

#define ROUTER_SSID "ROUTER" // 路由
#define ROUTER_PASS "PASSWORD" // 密码
#define SERVER "api.yeelink.net"
#define YEELINKAPI  "YOURYEELINKAPI"
#define DEVICEID  344501
#define SENSORID1 382953  // 回水温度
#define SENSORID2 382954  // 进水温度
#define SENSORID3 382955  // 壁挂炉
#define SENSORID4 382956  // 循环泵
#define SENSORID5 382957  // 温控器
int status = WL_IDLE_STATUS;
WiFiEspClient wificlient;
boolean wifiReady = true;
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
#if (defined PROCESSING_PLOT) || (defined ESPWIFI)
  Serial.begin(9600);
#endif
  pinMode(pinBoiler, OUTPUT); // 输出模式必须显性指定
  pinMode(pinPump, OUTPUT); // 输出模式必须显性指定
  pinMode(pinLed, OUTPUT);

#ifdef ESPWIFI
  // 初始化ESP模块
  Serial1.begin(9600);
  WiFi.init(&Serial1);

  if (WiFi.status() == WL_NO_SHIELD) { // 没有发现ESPwifi模块
    Serial.println("ESP Wifi shield not present");
    wifiReady = false;
  }

  if (wifiReady) {
    while (status != WL_CONNECTED) {
      status = WiFi.begin(ROUTER_SSID, ROUTER_PASS);
    }
    Serial.println("Connected to the network");
    printWifiStatus();
  }

#endif

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
  wdt_enable(WDTO_8S); // 500毫秒无响应视为死机,重新启动.
#endif
}


void loop() {
  bool panelState = false, boilerState = false, pumpState = false;

#ifdef ESPWIFI
  if (wificlient.available()) {
    char c = wificlient.read();
    Serial.print(c);
  }
#endif

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

#ifdef ESPWIFI
    Serial.println("update panelState");
    postData(panelState, YEELINKAPI, DEVICEID, SENSORID5);
    Serial.println("update pumpState");
    postData(pumpState, YEELINKAPI, DEVICEID, SENSORID4);
    Serial.println("update boilerState");
    postData(boilerState, YEELINKAPI, DEVICEID, SENSORID3);

    // 写入温度
    Serial.print("Send Output temperature: ");
    Serial.println(boiler.getTemperatureOut());
    postData(round(boiler.getTemperatureOut()), YEELINKAPI, DEVICEID, SENSORID1);
    Serial.print("Send Input temperature: ");
    Serial.println(boiler.getTemperatureIn());
    postData(round(boiler.getTemperatureIn()), YEELINKAPI, DEVICEID, SENSORID2);
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

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void postData(int data, String apikey, unsigned long deviceid, unsigned long sensorid)
{
  if (wificlient.connect(SERVER, 80)) {
    int thisLength = 10 + getLength(data);
    Serial.println("Connected to server!");
    Serial.println("Send data now.");
    wificlient.print("POST /v1.0/device/");
    wificlient.print(deviceid);
    wificlient.print("/sensor/");
    wificlient.print(sensorid);
    wificlient.print("/datapoints");
    wificlient.println(" HTTP/1.1");
    wificlient.println("Host: api.yeelink.net");
    wificlient.print("Accept: *");
    wificlient.print("/");
    wificlient.println("*");
    wificlient.print("U-ApiKey: ");
    wificlient.println(apikey);
    wificlient.print("Content-Length: ");
    wificlient.println(thisLength);
    wificlient.println("Content-Type: application/x-www-form-urlencoded");
    wificlient.println("Connection: close");
    wificlient.println();
    wificlient.print("{\"value\":");
    wificlient.print(data);
    wificlient.print("}");
    wificlient.stop();
  } else {
    Serial.println("Connecting failure");
    Serial.println("Send data abort.");
    wificlient.stop();
  }
}

int getLength(int someValue) {
  // there's at least one byte:
  int digits = 1;
  // continually divide the value by ten,
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = someValue / 10;
  while (dividend > 0) {
    dividend = dividend / 10;
    digits++;
  }
  // return the number of digits:
  return digits;
}
