#include "Thermistor.h"
#include "Arduino.h"

#define FAST_TABLE
#define FILTER // 是否需要过滤电压波动所产生的纹波

#ifndef FAST_TABLE // 查表法
#define CALC_RESIST  // 阻值计算法
#endif

#ifdef FILTER
void bubbleSort( int sort[], unsigned char len )
{
  char i, j;
  int temp;

  len -= 2;

  for ( i = len; i >= 0; i--)
  {
    for ( j = 0; j <= i; j++)
    {
      if ( sort[j + 1] < sort[j])
      {
        temp = sort[j];
        sort[j] = sort[j + 1];
        sort[j + 1] = temp;
      }
    }
  }
}
#endif

#ifdef CALC_RESIST // 阻值计算法
#define THERMISTORNOMINAL 100000  // NTC阻值 100K
#define TEMPERATURENOMINAL 25     // 标准温度25C
#define BCOEFFICIENT 3950         // NTC B值
#define SERIESRESISTOR 4700       // 串联电阻阻值 4.7K

// 获取温度值函数
bool Thermistor::update() {
  uint8_t i;
  float media = 0;

  for (i = 0; i < OVERSAMPLENR; i++) {
    heatCount[i] = analogRead(analog_pin);
    delay(10);
  }

#ifdef FILTER
  bubbleSort(heatCount, OVERSAMPLENR);
  for (i = 2; i < OVERSAMPLENR - 2; i ++) {
    media += heatCount[i];
  }
  media = media / (OVERSAMPLENR - 4) * OVERSAMPLENR;
#else
  for (i = 0; i < OVERSAMPLENR; i++) {
    media += heatCount[i];
  }
#endif

  media /= OVERSAMPLENR;
  media = 1023 / media - 1;
  media = SERIESRESISTOR / media;
  float temperature;
  temperature = media / THERMISTORNOMINAL;     // (R/Ro)
  temperature = log(temperature);                  // ln(R/Ro)
  temperature /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  temperature += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  temperature = 1.0 / temperature;                 // Invert
  temperature -= 273.15;

  currentTemp = temperature;
  return true;
}
#endif

#ifdef FAST_TABLE // 快速查表法
const short temptable_1[][2] PROGMEM = {
  {711 * OVERSAMPLENR,      85},
  {751 * OVERSAMPLENR,      79},
  {791 * OVERSAMPLENR,      72},
  {811 * OVERSAMPLENR,      69},
  {831 * OVERSAMPLENR,      65},
  {871 * OVERSAMPLENR,      57},
  {881 * OVERSAMPLENR,      55},
  {901 * OVERSAMPLENR,      51},
  {921 * OVERSAMPLENR,      45},
  {941 * OVERSAMPLENR,      39},
  {971 * OVERSAMPLENR,      28},
  {981 * OVERSAMPLENR,      23},
  {991 * OVERSAMPLENR,      17},
  {1001 * OVERSAMPLENR,     9},
  {1021 * OVERSAMPLENR,     -27}
};

//这堆东西都是从Marlin借过来方便运算和减少工作量的
#define _TT_NAME(_N) temptable_ ## _N
#define TT_NAME(_N) _TT_NAME(_N)
#define HEATER_TEMPTABLE TT_NAME(1)
#define HEATER_TEMPTABLE_LEN (sizeof(HEATER_TEMPTABLE)/sizeof(*HEATER_TEMPTABLE))

static void *heater_ttbl_map = (void *)HEATER_TEMPTABLE;
static uint8_t heater_ttbllen_map = HEATER_TEMPTABLE_LEN;

//从阻值温度表中取得对应的温度
#define PGM_RD_W(x)   (short)pgm_read_word(&x)
float analog2temp(int raw) {
  float celsius = 0;
  uint8_t i;
  short (*tt)[][2] = (short (*)[][2])(heater_ttbl_map);

  for (i = 1; i < heater_ttbllen_map; i++)
  {
    if (PGM_RD_W((*tt)[i][0]) > raw)
    {
      celsius = PGM_RD_W((*tt)[i - 1][1]) +
                (raw - PGM_RD_W((*tt)[i - 1][0])) *
                (float)(PGM_RD_W((*tt)[i][1]) - PGM_RD_W((*tt)[i - 1][1])) /
                (float)(PGM_RD_W((*tt)[i][0]) - PGM_RD_W((*tt)[i - 1][0]));
      break;
    }
  }

  // Overflow: Set to last value in the table
  if (i == heater_ttbllen_map) celsius = PGM_RD_W((*tt)[i - 1][1]);

  return celsius;
}

bool Thermistor::update() {
  float media = 0;
  int i;

  for (i = 0; i < OVERSAMPLENR; i++) {
    heatCount[i] = analogRead(analog_pin);
    delay(10); // 10ms的等待是必须的,否则温度读取会出错.
  }

#ifdef FILTER
  bubbleSort(heatCount, OVERSAMPLENR);
  for (i = 2; i < OVERSAMPLENR - 2; i ++) {
    media += heatCount[i];
  }
  media = media / (OVERSAMPLENR - 4) * OVERSAMPLENR;
#else
  for (i = 0; i < OVERSAMPLENR; i++) {
    media += heatCount[i];
  }
#endif

  currentTemp = analog2temp(media);
  return true;
}

#endif

Thermistor::Thermistor(uint8_t analog_pin_in) :
  analog_pin(analog_pin_in) {
  for (int i = 0; i < OVERSAMPLENR; i++) {
    heatCount[i] = 0;
  }
}
