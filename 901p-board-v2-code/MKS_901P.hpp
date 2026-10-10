#pragma once

#include <array>
#include <string_view>
#include <Arduino.h>

class MKS_901P {
public:
  // Indicates the lack of an address.
  static constexpr uint8_t NoAddress = 254;

  MKS_901P(Stream& serial = Serial, uint8_t addr = NoAddress);

  // Returns the currently-active transmission address
  uint8_t addr() const { return _addr; }

  // Performs all initialization, including resolving an address if one was not specified.
  // The provided serial bus *must* be initialized prior to calling this function.
  bool begin();

  // Issues a raw query, copying up to `len` characters of the result to the provided buffer.
  // If succeeded, returns the expected length of the buffer, excluding the null terminator.
  // If failed, returns the *negative* of the sensor's NAK code. 
  ssize_t rawQuery(const char* cmd, char* buf, size_t len);

  // Issues a raw command with the provided parameter.
  // If succeeded, returns a non-negative value.
  // If failed, returns the *negative* of the sensor's NAK code. 
  ssize_t rawCommand(const char* cmd, const char* param);
private:
  enum ResponseType {
    RespBroken = 0,
    RespAck,
    RespNak
  };

  // Reads a response from the serial to _recvBuf.
  size_t readResponse();
  // Checks whether the last response was ACK or NAK.
  ResponseType responseStatus();
  // Returns the negative response code for a NAK.
  ssize_t parseNak();

  Stream& _serial;
  uint8_t _addr;
  bool _init;
  std::array<char, 32> _sendBuf;
  std::array<char, 32> _recvBuf;
};