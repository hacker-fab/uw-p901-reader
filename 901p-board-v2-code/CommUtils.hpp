#pragma once

#include <cstdint>
#include <initializer_list>
#include <Arduino.h>
#include <Wire.h>

// Pings a device on an I2C bus, returning true if the device responded.
inline bool wirePing(uint8_t addr, TwoWire& wire = Wire) {
  wire.beginTransmission(addr);
  return wire.endTransmission() == 0;
}

// Shorthand for sending an init-list of bytes over I2C.
inline int wirePacket(uint8_t addr, std::initializer_list<uint8_t> data, TwoWire& wire = Wire) {
  wire.beginTransmission(addr);
  wire.write(data.begin(), data.size());
  return wire.endTransmission();
}

// Shorthand for reading an I2C register.
inline uint8_t wireRegisterReadByte(uint8_t addr, uint8_t reg, TwoWire& wire = Wire) {
  wire.beginTransmission(addr);
  wire.write(reg);
  wire.endTransmission(false);

  wire.requestFrom(addr, 1);
  return (uint8_t) wire.read();
}

// Shorthand for writing an I2C register.
inline void wireRegisterWriteByte(uint8_t addr, uint8_t reg, uint8_t data, TwoWire& wire = Wire) {
  wire.beginTransmission(addr);
  wire.write(reg);
  wire.write(data);
  wire.endTransmission();
}