#ifndef ADAFRUIT_MPR121_R_H
#define ADAFRUIT_MPR121_R_H
 
#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include <Wire.h>

// The default I2C address
#define MPR121_R_I2CADDR_DEFAULT 0x5A

#define MPR121_R_TOUCHSTATUS_L 0x00
#define MPR121_R_TOUCHSTATUS_H 0x01
#define MPR121_R_FILTDATA_0L  0x04
#define MPR121_R_FILTDATA_0H  0x05
#define MPR121_R_BASELINE_0   0x1E
#define MPR121_R_MHDR         0x2B
#define MPR121_R_NHDR         0x2C
#define MPR121_R_NCLR         0x2D
#define MPR121_R_FDLR         0x2E
#define MPR121_R_MHDF         0x2F
#define MPR121_R_NHDF         0x30
#define MPR121_R_NCLF         0x31
#define MPR121_R_FDLF         0x32
#define MPR121_R_NHDT         0x33
#define MPR121_R_NCLT         0x34
#define MPR121_R_FDLT         0x35

#define MPR121_R_TOUCHTH_0    0x41
#define MPR121_R_RELEASETH_0    0x43
#define MPR121_R_DEBOUNCE 0x5B
#define MPR121_R_CONFIG1 0x5C
#define MPR121_R_CONFIG2 0x5D
#define MPR121_R_CHARGECURR_0 0x5F
#define MPR121_R_CHARGETIME_1 0x6C
#define MPR121_R_ECR 0x5E
#define MPR121_R_AUTOCONFIG0 0x7B
#define MPR121_R_AUTOCONFIG1 0x7C
#define MPR121_R_UPLIMIT   0x7D
#define MPR121_R_LOWLIMIT  0x7E
#define MPR121_R_TARGETLIMIT  0x7F

#define MPR121_R_GPIODIR  0x76
#define MPR121_R_GPIOEN  0x77
#define MPR121_R_GPIOSET  0x78
#define MPR121_R_GPIOCLR  0x79
#define MPR121_R_GPIOTOGGLE  0x7A

#define MPR121_R_SOFTRESET 0x80

//.. thru to 0x1C/0x1D
class Adafruit_MPR121_R {
 public:
  // Hardware I2C
  Adafruit_MPR121_R(void);

  boolean begin(uint8_t i2caddr_R = MPR121_R_I2CADDR_DEFAULT);

  uint16_t filteredData(uint8_t t);
  uint16_t  baselineData(uint8_t t);

  uint8_t readRegister8(uint8_t reg);
  uint16_t readRegister16(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);
  uint16_t touched(void);
  // Add deprecated attribute so that the compiler shows a warning
  __attribute__((deprecated)) void setThreshholds(uint8_t touch, uint8_t release);
  void setThresholds(uint8_t touch, uint8_t release);

 private:
  int8_t _i2caddr_R;
};

#endif // ADAFRUIT_MPR121_R_H
