#include "Adafruit_MPR121_C.h"

Adafruit_MPR121_C::Adafruit_MPR121_C() {
}

boolean Adafruit_MPR121_C::begin(uint8_t i2caddr_C) {
  Wire.begin();
    
  _i2caddr_C = i2caddr_C;

  // soft reset
  writeRegister(MPR121_C_SOFTRESET, 0x63);
  delay(1);
  for (uint8_t i=0; i<0x7F; i++) {
  //  Serial.print("$"); Serial.print(i, HEX); 
  //  Serial.print(": 0x"); Serial.println(readRegister8(i));
  }
  

  writeRegister(MPR121_C_ECR, 0x0);

  uint8_t c_C = readRegister8(MPR121_C_CONFIG2);
  
  if (c_C != 0x24) return false;


  setThreshholds(2,6);
  writeRegister(MPR121_C_MHDR, 0x01);
  writeRegister(MPR121_C_NHDR, 0x01);
  writeRegister(MPR121_C_NCLR, 0x0E);
  writeRegister(MPR121_C_FDLR, 0x00);

  writeRegister(MPR121_C_MHDF, 0x01);
  writeRegister(MPR121_C_NHDF, 0x05);
  writeRegister(MPR121_C_NCLF, 0x01);
  writeRegister(MPR121_C_FDLF, 0x00);

  writeRegister(MPR121_C_NHDT, 0x00);
  writeRegister(MPR121_C_NCLT, 0x00);
  writeRegister(MPR121_C_FDLT, 0x00);

  writeRegister(MPR121_C_DEBOUNCE, 0x11);
  writeRegister(MPR121_C_CONFIG1, 0x16); // default, 16uA charge current
  writeRegister(MPR121_C_CONFIG2, 0x20); // 0.5uS encoding, 1ms period

//  writeRegister(MPR121_C_AUTOCONFIG0, 0x8F);

//  writeRegister(MPR121_C_UPLIMIT, 150);
//  writeRegister(MPR121_C_TARGETLIMIT, 100); // should be ~400 (100 shifted)
//  writeRegister(MPR121_C_LOWLIMIT, 50);
  // enable all electrodes
  writeRegister(MPR121_C_ECR, 0x8F);  // start with first 5 bits of baseline tracking

  return true;
}

void Adafruit_MPR121_C::setThreshholds(uint8_t touch, uint8_t release) {

  setThresholds(touch, release);
  }

void Adafruit_MPR121_C::setThresholds(uint8_t touch, uint8_t release) {
  for (uint8_t i=0; i<12; i++) {
    writeRegister(MPR121_C_TOUCHTH_0 + 2*i, touch);
    writeRegister(MPR121_C_RELEASETH_0 + 2*i, release);
  }
}

uint16_t  Adafruit_MPR121_C::filteredData(uint8_t t) {
  if (t > 12) return 0;
  return readRegister16(MPR121_C_FILTDATA_0L + t*2);
}

uint16_t  Adafruit_MPR121_C::baselineData(uint8_t t) {
  if (t > 12) return 0;
  uint16_t bl = readRegister8(MPR121_C_BASELINE_0 + t);
  return (bl << 2);
}

uint16_t  Adafruit_MPR121_C::touched(void) {
  uint16_t t = readRegister16(MPR121_C_TOUCHSTATUS_L);
  return t & 0x0FFF;
}

/*********************************************************************/


uint8_t Adafruit_MPR121_C::readRegister8(uint8_t reg) {
    Wire.beginTransmission(_i2caddr_C);
    Wire.write(reg);
    Wire.endTransmission(false);
    while (Wire.requestFrom(_i2caddr_C, 1) != 1);
    return ( Wire.read());
}

uint16_t Adafruit_MPR121_C::readRegister16(uint8_t reg) {
    Wire.beginTransmission(_i2caddr_C);
    Wire.write(reg);
    Wire.endTransmission(false);
    while (Wire.requestFrom(_i2caddr_C, 2) != 2);
    uint16_t v = Wire.read();
    v |=  ((uint16_t) Wire.read()) << 8;
    return v;
}

/**************************************************************************/
/*!
    @brief  Writes 8-bits to the specified destination register
*/
/**************************************************************************/
void Adafruit_MPR121_C::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_i2caddr_C);
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)(value));
    Wire.endTransmission();
}

