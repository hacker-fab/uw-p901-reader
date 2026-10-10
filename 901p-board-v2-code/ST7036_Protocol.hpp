#pragma once

#include <cstdint>

/******************
This file provides various functions for working with the ST7036's I2C protocol.
*******************/

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

// Clear the display and reset the cursor position to line 0, column 0.
// Must wait 2ms before issuing a new command.
constexpr uint8_t clearDisplay() { return 0x01; }
// Reset the cursor position to line 0, column 0.
// Must wait 2ms before issuing a new command.
constexpr uint8_t returnHome() { return 0x02; }
// ID = [0: cursor moves left, 1: cursor moves right], S = enable shift-on-write
constexpr uint8_t entryModeSet(bool ID, bool S = false) {
  return 0x04 | (uint8_t(ID) << 1) | uint8_t(S);
}
// D = display on/off, C = cursor on/off, B = blink on/off
constexpr uint8_t displayOnOff(bool D, bool C = false, bool B = false) {
  return 0x08 | (uint8_t(D) << 2) | (uint8_t(C) << 1) | uint8_t(B);
}
// IS[1:0] = instruction table (see below), N = [0: 1 line, 1: >1 line], DH = enable double-height text
constexpr uint8_t functionSet(uint8_t IS, bool N = true, bool DH = false) { 
  return 0x30 | (uint8_t(N) << 3) | (uint8_t(DH) << 2) | (IS & 0x03); 
}
// AC[6:0] = DDRAM address.  
constexpr uint8_t setDDAddr(uint8_t AC) {
  return 0x80 | (AC & 0x7F);
}

// Wrapper for setDDAddr using a line/column number.
constexpr uint8_t setCursorAddr(uint8_t lines, uint8_t line, uint8_t col) {
  assert(line < lines);
  switch (lines) {
  case 1:
    return col;
  case 2:
    return (line << 6) | col;
  case 3:
    return (line << 4) | col;
  }
}

// INSTRUCTION TABLE 0
// ===================

// LR = [0: cursor moves left, 1: cursor moves right]
constexpr uint8_t shiftCursor(bool LR) {
  return 0x10 | (uint8_t(LR) << 2);
}
// LR = [0: display moves left, 1: display moves right]
constexpr uint8_t shiftDisplay(bool LR) {
  return 0x18 | (uint8_t(LR) << 2);
}
// AC[5:0] = CGRAM address.
constexpr uint8_t setCGAddr(uint8_t AC) {
  return 0x40 | (AC & 0x3F);
}

// INSTRUCTION TABLE 1
// ===================

// BS = [0: 1/4 bias, 1: 1/5 bias], FX = [0: < 3 lines, 1: 3 lines]
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

// MISC FUNCTIONS
// =================

// Finds the start of a line in DDRAM given the number of lines and the line number.
constexpr uint8_t findLineStartDD(uint8_t lines, uint8_t line) {
  assert(lines >= 1 && lines <= 3 && line < lines);
  switch (lines) {
  case 1:
    return 0;
  case 2:
    return (line << 6);
  case 3:
    return (line << 4);
  }
}


}