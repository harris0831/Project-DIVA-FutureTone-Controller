#include "Adafruit_MPR121_R.h"

Adafruit_MPR121_R::Adafruit_MPR121_R() {
}

boolean Adafruit_MPR121_R::begin(uint8_t i2caddr_R) {
  Wire.begin();
    
  _i2caddr_R = i2caddr_R;

  // soft reset
  writeRegister(MPR121_R_SOFTRESET, 0x63);
  delay(1);
  for (uint8_t i=0; i<0x7F; i++) {
  //  Serial.print("$"); Serial.print(i, HEX); 
  //  Serial.print(": 0x"); Serial.println(readRegister8(i));
  }
  

  writeRegister(MPR121_R_ECR, 0x0);

  uint8_t c_R = readRegister8(MPR121_R_CONFIG2);
  
  if (c_R != 0x24) return false;


  setThreshholds(2,6);
  writeRegister(MPR121_R_MHDR, 0x01);
  writeRegister(MPR121_R_NHDR, 0x01);
  writeRegister(MPR121_R_NCLR, 0x0E);
  writeRegister(MPR121_R_FDLR, 0x00);

  writeRegister(MPR121_R_MHDF, 0x01);
  writeRegister(MPR121_R_NHDF, 0x05);
  writeRegister(MPR121_R_NCLF, 0x01);
  writeRegister(MPR121_R_FDLF, 0x00);

  writeRegister(MPR121_R_NHDT, 0x00);
  writeRegister(MPR121_R_NCLT, 0x00);
  writeRegister(MPR121_R_FDLT, 0x00);

  writeRegister(MPR121_R_DEBOUNCE, 0x11);
  writeRegister(MPR121_R_CONFIG1, 0x16); // default, 16uA charge current
  writeRegister(MPR121_R_CONFIG2, 0x20); // 0.5uS encoding, 1ms period

//  writeRegister(MPR121_R_AUTOCONFIG0, 0x8F);

//  writeRegister(MPR121_R_UPLIMIT, 150);
//  writeRegister(MPR121_R_TARGETLIMIT, 100); // should be ~400 (100 shifted)
//  writeRegister(MPR121_R_LOWLIMIT, 50);
  // enable all electrodes
  writeRegister(MPR121_R_ECR, 0x8F);  // start with first 5 bits of baseline tracking

  return true;
}

void Adafruit_MPR121_R::setThreshholds(uint8_t touch, uint8_t release) {

  setThresholds(touch, release);
  }

void Adafruit_MPR121_R::setThresholds(uint8_t touch, uint8_t release) {
  for (uint8_t i=0; i<12; i++) {
    writeRegister(MPR121_R_TOUCHTH_0 + 2*i, touch);
    writeRegister(MPR121_R_RELEASETH_0 + 2*i, release);
  }
}

uint16_t  Adafruit_MPR121_R::filteredData(uint8_t t) {
  if (t > 12) return 0;
  return readRegister16(MPR121_R_FILTDATA_0L + t*2);
}

uint16_t  Adafruit_MPR121_R::baselineData(uint8_t t) {
  if (t > 12) return 0;
  uint16_t bl = readRegister8(MPR121_R_BASELINE_0 + t);
  return (bl << 2);
}

uint16_t  Adafruit_MPR121_R::touched(void) {
  uint16_t t = readRegister16(MPR121_R_TOUCHSTATUS_L);
  return t & 0x0FFF;
}

/*********************************************************************/


uint8_t Adafruit_MPR121_R::readRegister8(uint8_t reg) {
    Wire.beginTransmission(_i2caddr_R);
    Wire.write(reg);
    Wire.endTransmission(false);
    while (Wire.requestFrom(_i2caddr_R, 1) != 1);
    return ( Wire.read());
}

uint16_t Adafruit_MPR121_R::readRegister16(uint8_t reg) {
    Wire.beginTransmission(_i2caddr_R);
    Wire.write(reg);
    Wire.endTransmission(false);
    while (Wire.requestFrom(_i2caddr_R, 2) != 2);
    uint16_t v = Wire.read();
    v |=  ((uint16_t) Wire.read()) << 8;
    return v;
}

/**************************************************************************/
/*!
    @brief  Writes 8-bits to the specified destination register
*/
/**************************************************************************/
void Adafruit_MPR121_R::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_i2caddr_R);
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)(value));
    Wire.endTransmission();
}

