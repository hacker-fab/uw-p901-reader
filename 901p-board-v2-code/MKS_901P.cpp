#include <string_view>
#include <algorithm>
#include "MKS_901P.hpp"

#include <cstring>
#include <cstdlib>

namespace {

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
  _serial.write(_sendBuf.data(), sendLen);
  
  size_t recvLen = readResponse();
  if (recvLen == 0)
    return false;
  
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

ssize_t MKS_901P::rawQuery(const char* cmd, char* buf, size_t len) {
  assert(_init);
  // encode and send the query, fail if it's too long
  int sendLen = snprintf(_sendBuf.data(), _sendBuf.size(), "@%03u%s?;FF", _addr, cmd);
  if (sendLen > _sendBuf.size() - 1)
    return -1;
  _serial.write(_sendBuf.data(), sendLen);

  // read the response
  const size_t recvLen = readResponse();
  if (recvLen == 0)
    return -1;
  const auto recvStatus = responseStatus();
  switch (recvStatus) {
  case RespBroken:
    return -1;
  case RespNak:
    return parseNak();
  case RespAck: {
    strncpy(buf, _recvBuf.data() + 6, len);
    return recvLen - 6;
  }
  }


}

ssize_t MKS_901P::rawCommand(const char* cmd, const char* param) {
  assert(_init);
  // encode and send the query, fail if it's too long
  int sendLen = snprintf(_sendBuf.data(), _sendBuf.size(), "@%03u%s!%s;FF", _addr, cmd, param);
  if (sendLen > _sendBuf.size() - 1)
    return -1;
  _serial.write(_sendBuf.data(), sendLen);

  // read the response
  const size_t recvLen = readResponse();
  if (recvLen == 0)
    return -1;
  const auto recvStatus = responseStatus();
  switch (recvStatus) {
  case RespBroken:
    return -1;
  case RespNak:
    return parseNak();
  case RespAck:
    return 0;
  }
}

size_t MKS_901P::readResponse() {
  // wait for the response to come back
  while (!(Serial.available() && (char) Serial.read() == '@')) {}
  // read off the full response to the semicolon
  // this should never fill the buffer
  size_t readLen = Serial.readBytesUntil(';', _recvBuf.data(), _recvBuf.size() - 1);
  // read terminating characters and check they match
  int term1 = Serial.read();
  int term2 = Serial.read();
  if (term1 != 'F' || term2 != 'F')
    return 0;
  // check that we read enough for an address + ACK/NAK
  if (readLen < 6)
    return 0;

  // buffer should be OK
  return readLen;
}

MKS_901P::ResponseType MKS_901P::responseStatus() {
  std::string_view ackPart {_recvBuf.data() + 3, 3};
  if (ackPart == "ACK")
    return RespAck;
  if (ackPart == "NAK")
    return RespNak;
  return RespBroken;
}

ssize_t MKS_901P::parseNak() {
  return atol(_recvBuf.data() + 6);
}