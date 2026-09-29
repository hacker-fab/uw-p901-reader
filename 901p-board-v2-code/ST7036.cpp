#include "ST7036.hpp"
#include "WireExt.hpp"

bool ST7036::begin() {
  if (_init)
    return true;
  // Init sequence described here:
  // https://www.lcd-module.com/eng/pdf/zubehoer/st7036.pdf
  // Serial interface & IIC interface (p. 43)
  using namespace ST7036_Protocol;

  // wait 40ms to ensure initialization
  delay(40);
  // set all initial settings
  int result = wirePacket(_addr, {
    ctrlCmds(),
    functionSet(1),
    biasSet(),
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

size_t ST7036::write(uint8_t data) {
  using namespace ST7036_Protocol;
  _wire.beginTransmission(_addr);
  _wire.write(ctrlData());
  _wire.write(data);
  return _wire.endTransmission() == 0;
}
size_t ST7036::write(const uint8_t* data, size_t size) {
  using namespace ST7036_Protocol;
  _wire.beginTransmission(_addr);
  _wire.write(ctrlData());
  _wire.write(data, size);
  return (_wire.endTransmission() == 0)? size : 0;
}