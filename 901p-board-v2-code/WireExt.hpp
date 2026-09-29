#pragma once

#include <cstdint>
#include <initializer_list>
#include <Wire.h>


inline void wireSend(uint8_t addr, std::initializer_list<uint8_t> data, TwoWire& wire = Wire) {
  wire.write(data.begin(), data.size());
}

inline int wirePacket(uint8_t addr, std::initializer_list<uint8_t> data, TwoWire& wire = Wire) {
  wire.beginTransmission(addr);
  wire.write(data.begin(), data.size());
  return wire.endTransmission();
}