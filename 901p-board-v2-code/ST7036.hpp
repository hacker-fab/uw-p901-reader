#pragma once

#include <cstdint>
#include <Print.h>
#include <Wire.h>

namespace ST7036_Protocol {
// CONTROL BYTES
// ===================

// control byte: Next data byte(s) are commands.
constexpr uint8_t ctrlCmds(bool more = false) {
  return uint8_t(more) << 7;
}
// control byte: Next data byte(s) go to RAM.
constexpr uint8_t ctrlData(bool more = false) {
  return (uint8_t(more) << 7) | 0x40;
}

// INSTRUCTIONS
// ===================
constexpr uint8_t clearDisplay() { return 0x01; }
constexpr uint8_t returnHome() { return 0x02; }
// ID = [0: cursor moves left, 1: cursor moves right], S = enable text shifting
constexpr uint8_t entryModeSet(bool ID, bool S = false) {
  return 0x04 | (uint8_t(ID) << 1) | uint8_t(S);
}
// D = display on/off, C = cursor on/off, B = blink on/off
constexpr uint8_t displayOnOff(bool D, bool C = false, bool B = false) {
  return 0x08 | (uint8_t(D) << 2) | (uint8_t(C) << 1) | uint8_t(B);
}
// IS[1:0] = instruction table (see below), N = [0: 1 line, 1: 2 line], DH = enable double-height text
constexpr uint8_t functionSet(uint8_t IS, bool N = true, bool DH = false) { 
  return 0x30 | (uint8_t(N) << 3) | (uint8_t(DH) << 2) | (IS & 0x03); 
}

// INSTRUCTION TABLE 1
// ===================
// BS = [0: 1/4 bias, 1: 1/5 bias], FX = [0: 1 or 2 lines, 1: 3 lines]
constexpr uint8_t biasSet(bool BS = false, bool FX = false) {
  return 0x14 | (uint8_t(BS) << 3) | uint8_t(FX);
}
// Ion = enable ICON display, Bon = enable booster, C[5:4] = high nibble of contrast
constexpr uint8_t contrastSetHi(uint8_t C, bool Ion = false, bool Bon = true) {
  return 0x50 | (uint8_t(Ion) << 3) | (uint8_t(Bon) << 2) | ((C >> 4) & 0x03);
}
// C[3:0] = low nibble of contrast
constexpr uint8_t contrastSetLo(uint8_t C) {
  return 0x70 | (C & 0x0F);
}
// Rab[2:0] = follower gain, Fon = enable internal follower
constexpr uint8_t followerCtrl(uint8_t Rab, bool Fon = true) {
  return 0x60 | (uint8_t(Fon) << 3) | (Rab & 0x07);
}

}

class ST7036 : public Print {
public:
  ST7036(TwoWire& wire, uint8_t addr) : _wire(wire), _addr(addr), _init(false) {}

  // Perform all initialization. Returns true if successful.
  bool begin();

  size_t write(uint8_t data) override;
  size_t write(const uint8_t* data, size_t size) override;
private:
  TwoWire& _wire;
  uint8_t _addr;
  bool _init;
};