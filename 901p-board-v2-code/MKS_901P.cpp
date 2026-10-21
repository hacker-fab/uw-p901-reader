#include "esp32-hal.h"
#include "MKS_901P.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string_view>

#include <Arduino.h>

namespace {
const char* unit_strs[] = {
  "TORR",
  "MBAR",
  "PASCAL"
};
const char* gas_strs[] = {
  "NITROGEN",
  "AIR",
  "ARGON",
  "HELIUM",
  "HYDROGEN",
  "H2O",
  "NEON",
  "CO2",
  "XENON",
};

template <size_t N>
int revLookup(const char* key, const char* (&table)[N]) {
  for (size_t i = 0; i < N; i++) {
    if (strcmp(key, table[i]) == 0)
      return i;
  }
  return -1;
}
}

MKS_901P::MKS_901P(Stream& serial, uint8_t addr) : _serial(serial), _addr(addr) {
  // normalize address 255 to 254
  if (_addr == 255)
    _addr == NoAddress;
}

bool MKS_901P::begin() {
  if (_init)
    return true;
  
  // send test message. if NoAddress, this works fine anyways.
  int sendLen = snprintf(_sendBuf.data(), _sendBuf.size(), "@%03uT?;FF", _addr);
  int testRes = transact(sendLen);

  if (testRes != ErrNone) {
    Serial.println(testRes);
    return false;
  }
  
  // the response should be valid either way, so save the address from this 
  // request if we don't have one
  if (_addr == NoAddress) {
    char parseBuffer[4] = {};
    memcpy(parseBuffer, _recvBuf.data(), 3);
    _addr = atoi(parseBuffer);
  }

  _init = true;
  return true;
}

MKS_901P::Error MKS_901P::rawQuery(const char* cmd, std::string_view& result) {
  assert(_init);
  // encode the query, fail if it's too long
  int sendLen = snprintf(_sendBuf.data(), _sendBuf.size(), "@%03u%s?;FF", _addr, cmd);
  if (sendLen > _sendBuf.size() - 1)
    return ErrSendFailed;

  return transact(sendLen, &result);
}

MKS_901P::Error MKS_901P::rawCommand(const char* cmd, const char* param) {
  assert(_init);
  // encode the command, fail if it's too long
  int sendLen = snprintf(_sendBuf.data(), _sendBuf.size(), "@%03u%s!%s;FF", _addr, cmd, param);
  if (sendLen > _sendBuf.size() - 1)
    return ErrSendFailed;

  return transact(sendLen);
}

MKS_901P::Error MKS_901P::transact(size_t sendLen, std::string_view* result) {
  using namespace std::literals;

  // send the query
  _serial.write(_sendBuf.data(), sendLen);

  // wait for the response to come back
  while (_serial.read() != '@') {}

  // read off the full response to the semicolon
  // this should never fill the buffer
  size_t readLen = _serial.readBytesUntil(';', _recvBuf.data(), _recvBuf.size() - 1);
  // read terminating characters
  while (!_serial.available()) {}
  char term1 = _serial.read();
  while (!_serial.available()) {}
  char term2 = _serial.read();

  // cap off received characters with null for safety
  _recvBuf[readLen] = '\0';
  // needed between requests to avoid overloading the sensor.
  delayMicroseconds(400);

  // check if terminators match
  if (term1 != 'F' || term2 != 'F')
    return ErrParseFailed;
  // check that we read enough for an address + ACK/NAK
  if (readLen < 6)
    return ErrParseFailed;

  // check if response is ACK, NACK, or neither
  std::string_view ackPart {_recvBuf.data() + 3, 3};
  if (ackPart == "ACK"sv) {
    if (result)
      *result = std::string_view {_recvBuf.data() + 6, readLen - 6};
    return ErrNone;
  }
  if (ackPart == "NAK"sv) {
    return (Error) atol(_recvBuf.data() + 6);
  }


  return ErrParseFailed;
}

// PARSED PROTOCOL OPERATIONS
// ========================================

MKS_901P::Error MKS_901P::readPR4(std::pair<int, int>& reading) {
  std::string_view line;
  if (auto res = rawQuery("PR4", line); res != ErrNone)
    return res;
  // X.XXXE+X
  unsigned int leading = 0;
  unsigned int trailing = 0;
  int exponent = 0;

  int matches = sscanf(line.data(), "%1u.%3uE%d", &leading, &trailing, &exponent);
  if (matches < 3)
    return ErrParseFailed;

  reading = {leading * 1000 + trailing, exponent};
  return ErrNone;
}

// Gets the current units being used by the sensor.
MKS_901P::Error MKS_901P::readUnits(Units& units) {
  std::string_view line;
  if (auto res = rawQuery("U", line); res != ErrNone)
    return res;

  int entry = revLookup(line.data(), unit_strs);
  if (entry < 0)
    return ErrParseFailed;

  units = (Units) entry;
  return ErrNone;
}
// Sets the sensor's current units.
MKS_901P::Error MKS_901P::writeUnits(Units units) {
  return rawCommand("U", unit_strs[(size_t) units]);
}

// Gets the sensor's current gas type.
MKS_901P::Error MKS_901P::readGasType(GasType& gas) {
  std::string_view line;
  if (auto res = rawQuery("GT", line); res != ErrNone)
    return res;

  int entry = revLookup(line.data(), gas_strs);
  if (entry < 0)
    return ErrParseFailed;

  gas = (GasType) entry;
  return ErrNone;
}
// Sets the gas type.
MKS_901P::Error MKS_901P::writeGasType(GasType gas) {
  return rawCommand("GT", gas_strs[(size_t) gas]);
}

