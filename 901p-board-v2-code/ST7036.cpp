#include <cassert>

#include <Arduino.h>

#include "ST7036.hpp"
#include "ST7036_Protocol.hpp"
#include "WireExt.hpp"

ST7036::ST7036(TwoWire& wire, uint8_t addr, uint8_t lines, uint8_t cols) : 
  _wire(wire), 
  _addr(addr), 
  _lines(lines), 
  _cols(cols) {
  assert((lines <= 2 && cols <= 20) || (lines == 3 && cols <= 16));
}

bool ST7036::begin() {
  if (_init)
    return true;
  // Init sequence described here:
  // https://www.lcd-module.com/eng/pdf/zubehoer/st7036.pdf
  // Serial interface & IIC interface (p. 43)
  using namespace ST7036_Protocol;

  // wait 50 ms from power on to ensure the LCD is on
  // ideally, begin is not called for the first time 50 days after reboot
  while (millis() < 50) {}
  // set all initial settings
  int result = wirePacket(_addr, {
    ctrlCmds(),
    functionSet(1, _lines > 1),
    biasSet(false, _lines > 2),
    contrastSetLo(0x23),
    contrastSetHi(0x23),
    followerCtrl(6),
    displayOnOff(true),
    entryModeSet(true),
    clearDisplay(),
  }, _wire);
  if (result != 0)
    return false;
  // init completed, wait 2ms for clear to take effect
  _init = true;
  delay(2);
  return true;
}


// COMMANDS
// ===============================

void ST7036::clear() {
  using namespace ST7036_Protocol;
  assert(_init);

  // Command takes 1.08 ms to complete, hence delay at end
  wirePacket(_addr, {
    ctrlCmds(),
    clearDisplay()
  }, _wire);
  delay(2);
}

void ST7036::home() {
  using namespace ST7036_Protocol;
  assert(_init);

  // Command takes 1.08 ms to complete, hence delay at end
  wirePacket(_addr, {
    ctrlCmds(),
    returnHome()
  }, _wire);
  delay(2);
}

void ST7036::setCursor(uint8_t col, uint8_t row) {
  using namespace ST7036_Protocol;
  assert(_init && col < _cols && row < _lines);

  uint8_t ddAddr = findLineStartDD(_lines, row) + col;

  wirePacket(_addr, {
    ctrlCmds(),
    setDDAddr(ddAddr)
  });
}

// Shifts the entire display and cursor one cell to the left.
void ST7036::scrollDisplayLeft() {
  using namespace ST7036_Protocol;
  assert(_init);

  wirePacket(_addr, {
    ctrlCmds(),
    functionSet(0, _lines > 1),
    shiftDisplay(false),
  });
}
// Shifts the entire display and cursor one cell to the right.
void ST7036::scrollDisplayRight() {
  using namespace ST7036_Protocol;
  assert(_init);

  wirePacket(_addr, {
    ctrlCmds(),
    functionSet(0, _lines > 1),
    shiftDisplay(true),
  });

}

// DATA WRITES
// ===============================

size_t ST7036::write(uint8_t data) {
  using namespace ST7036_Protocol;
  assert(_init);

  _wire.beginTransmission(_addr);
  _wire.write(ctrlData());
  _wire.write(data);
  return _wire.endTransmission() == 0;
}
size_t ST7036::write(const uint8_t* data, size_t size) {
  using namespace ST7036_Protocol;
  assert(_init);

  _wire.beginTransmission(_addr);
  _wire.write(ctrlData());
  _wire.write(data, size);
  return (_wire.endTransmission() == 0)? size : 0;
}

void ST7036::syncOnOff() {
  using namespace ST7036_Protocol;

  wirePacket(_addr, {
    ctrlCmds(),
    displayOnOff(_display, _cursor, _blink)
  });
}

void ST7036::syncEntryMode() {
  using namespace ST7036_Protocol;
  wirePacket(_addr, {
    ctrlCmds(),
    entryModeSet(_isDirRight, _autoscroll)
  });
}

