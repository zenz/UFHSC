#include "Boiler.h"
#include "Pump.h"
#include "Panel.h"
#include "Led.h"

#define WORKING_TEMP 30UL // 回水不30度
#define PROTECT_PERIOD 180000UL // 保护时间为3分钟
#define BLINK_INTERVAL_NORMAL 1000UL  // 供热时1秒一闪
#define BLINK_INTERVAL_SHORT 200UL  // 温度出错时1/5秒一闪
#define BLINK_INTERVAL_LONG 3000UL  // 非供热时3秒一闪
#define DISPLAY_INTERVAL 1000UL // 显示间隔1秒
#define UPDATE_INTERVAL 300000UL  // 更新服务间隔5分钟
#define PROCESSING_PLOT // 输出Processing程序监控数据
#define WATCHDOG  // 使用看门狗
#define DIGITAL_TUBE  // 使用4位数字管
#define ESPWIFI // 如果开启了ESPWIFI,则会自动关闭PROCESSING_PLOT及WATCHDOG
//#define DEBUG // 允许串口输出调试

#if (defined DIGITAL_TUBE) | (defined ESPWIFI) | (defined PROCESSING_PLOT)
#include "Thread.h"
#include "ThreadController.h"
ThreadController controll = ThreadController();
#endif

#ifdef WATCHDOG
#include <avr/wdt.h>
#endif

#ifdef DIGITAL_TUBE
// 用数字管显示进回水温度
#include "TM1637.h"
#define CLK 11
#define DIO 12
TM1637 tm1637(CLK, DIO);
boolean tmPowerOn = false;
Thread* displayThread = new Thread();
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
WiFiEspClient wifiClient;
boolean wifiReady = true;
Thread* updateThread = new Thread();
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
uint32_t boilerProtectPeriod, pumpProtectPeriod;
bool panelState = false, boilerState = false, pumpState = false;
void(* resetFunc) (void) = 0;

//--------------------------------------------------------------------
// 系统初始化
//--------------------------------------------------------------------

void setup() {
#if (defined PROCESSING_PLOT) || (defined ESPWIFI) || (defined DEBUG)
  Serial.begin(9600);
#endif
  analogReference(EXTERNAL);
  pinMode(pinBoiler, OUTPUT); // 输出模式必须显性指定
  pinMode(pinPump, OUTPUT); // 输出模式必须显性指定
  pinMode(pinLed, OUTPUT);

  boiler.setTargetTemperature(WORKING_TEMP);
  boiler.setProtectPeriod(PROTECT_PERIOD);
  pump.setProtectPeriod(PROTECT_PERIOD);
  pumpReady = !digitalRead(pinPumpReady);

#ifdef DIGITAL_TUBE
  tm1637.init(); // 数字管初始化
  tm1637.set(BRIGHT_TYPICAL); // 普通亮度
  tm1637.display(0, 0); // 未进入循环处理程序前,4位全部显示0
  tm1637.display(1, 0);
  tm1637.display(2, 0);
  tm1637.display(3, 0);
  displayThread->onRun(displayCallback);
  displayThread->setInterval(DISPLAY_INTERVAL);
  controll.add(displayThread);
#endif

#ifdef ESPWIFI
  // 初始化ESP模块
  Serial1.begin(9600);
  WiFi.init(&Serial1);

  if (WiFi.status() == WL_NO_SHIELD) { // 没有发现ESPwifi模块
    Serial.println("ESP Wifi shield not present");
    resetFunc();
    wifiReady = false;
  }

  if (wifiReady) {
    while (status != WL_CONNECTED) {
      status = WiFi.begin(ROUTER_SSID, ROUTER_PASS);
    }
    Serial.println("Connected to the network");
    printWifiStatus();
  }
  updateThread->onRun(updateCallback);
  updateThread->setInterval(UPDATE_INTERVAL);
  controll.add(updateThread);
#endif

#ifdef WATCHDOG
  wdt_enable(WDTO_8S); // 500毫秒无响应视为死机,重新启动. 如果用了Yeelink服务,则应该有更长时间.
#endif
}

//--------------------------------------------------------------------
// 主体循环
//--------------------------------------------------------------------

void loop() {
  panel.update(); // 更新面板状态
  panelState = panel.getState(); // 读取面板状态
  if (!panelState) {
    // 如果温控面板关闭了,那么无需壁挂炉启动,泵也应该停止
    led.setBlinkInterval(BLINK_INTERVAL_LONG); // 闪灯频率降到每3秒一次.
    boiler.setTargetTemperature(0);
    boilerState = boiler.update();
    if (pumpReady) { // 有泵,需要进行相应处理
      pump.disable();
      pumpState = pump.update();
    } else {
      pumpState = false;
    }
  } else { // 如果温控面板没有关闭
    led.setBlinkInterval(BLINK_INTERVAL_NORMAL); // 闪灯频率变为1秒一次
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
    led.setBlinkInterval(BLINK_INTERVAL_SHORT);
  }

  led.blink(); // 更新闪灯状态

#if (defined DIGITAL_TUBE) | (defined ESPWIFI) | (defined PROCESSING_PLOT)
  controll.run();
#endif

#ifdef WATCHDOG
  wdt_reset(); // 喂狗,防止重启
#endif
}

//--------------------------------------------------------------------
// 显示功能
//--------------------------------------------------------------------

#if (defined DIGITAL_TUBE) | (defined PROCESSING_PLOT)
void displayCallback() {
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
  tm1637.display(0, byte(round(boiler.getTemperatureOut())) / 10);  // 壁挂炉出水温度十位
  tm1637.display(1, byte(round(boiler.getTemperatureOut())) % 10);  // 壁挂炉出水温度个位
  tm1637.display(2, byte(round(boiler.getTemperatureIn())) / 10);   // 壁挂炉回水温度十位
  tm1637.display(3, byte(round(boiler.getTemperatureIn())) % 10);   // 壁挂炉回水温度各位
  tm1637.point(tmPowerOn); // 显示分割符号
  tmPowerOn = !tmPowerOn; // 实现分隔符闪烁
#endif
}
#endif

//--------------------------------------------------------------------
// 网络数据更新功能
//--------------------------------------------------------------------

#ifdef ESPWIFI
void updateCallback() {
  // 面板状态
  Serial.println("update panelState");
  postData(panelState, YEELINKAPI, DEVICEID, SENSORID5);

  // 循环本状态
  Serial.println("update pumpState");
  postData(pumpState, YEELINKAPI, DEVICEID, SENSORID4);

  // 壁挂炉状态
  Serial.println("update boilerState");
  postData(boilerState, YEELINKAPI, DEVICEID, SENSORID3);

  // 出水温度
  Serial.print("Send Output temperature: ");
  Serial.println(boiler.getTemperatureOut());
  postData(round(boiler.getTemperatureOut()), YEELINKAPI, DEVICEID, SENSORID1);

  // 回水温度
  Serial.print("Send Input temperature: ");
  Serial.println(boiler.getTemperatureIn());
  postData(round(boiler.getTemperatureIn()), YEELINKAPI, DEVICEID, SENSORID2);

  Serial.println("Update finished!");
}

void printWifiStatus()
{
  // 显示连接到的SSID
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // 显示获取的IP地址
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // 显示接受信号强度
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void postData(int data, String apikey, unsigned long deviceid, unsigned long sensorid)
{
  if (wifiClient.connect(SERVER, 80)) {
    Serial.println("Connected to server!");
    Serial.println("Send data now.");
    int thisLength = 10 + getLength(data);
    wifiClient.print("POST /v1.0/device/");
    wifiClient.print(deviceid);
    wifiClient.print("/sensor/");
    wifiClient.print(sensorid);
    wifiClient.print("/datapoints HTTP/1.1\r\nHost: api.yeelink.net\r\n");
    wifiClient.print("Accept: */*\r\nU-ApiKey:");
    wifiClient.print(apikey);
    wifiClient.print("\r\nContent-Length: ");
    wifiClient.print(thisLength);
    wifiClient.print("\r\nContent-Type: application/x-www-form-urlencoded\r\n");
    wifiClient.print("Connection: close\r\n\r\n");
    wifiClient.print("{\"value\":");
    wifiClient.print(data);
    wifiClient.print("}");
    wifiClient.stop();
  } else {
    Serial.println("Connecting failure");
    Serial.println("Send data abort.");
    wifiClient.stop();
  }
}

int getLength(int someValue) {
  int digits = 1;
  int dividend = someValue / 10;

  while (dividend > 0) {
    dividend = dividend / 10;
    digits++;
  }
  return digits;
}
#endif
