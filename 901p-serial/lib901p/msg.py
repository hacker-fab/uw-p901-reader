from dataclasses import dataclass
from enum import Enum
import re
from serial import Serial
from typing import Any, Literal

from lib901p.enums import *

# MAIN PROTOCOL STUFF
# =======================================

class ProtocolError(ValueError):
    '''
    Error during encoding/decoding of protocol messages.
    '''
    def __init__(self, *args):
        super().__init__(self, *args)

class ResponseError(Exception):
    '''
    Error due to NAK code.
    '''
    def __init__(self, *args):
        super().__init__(self, *args)

@dataclass
class Query:
    key: str
    addr: int = 254

    def encode(self) -> bytes:
        if self.addr not in range(256):
            raise ProtocolError("Address must range between 0 and 255")
        return f"@{self.addr:03}{self.key}?;FF".encode(encoding="ascii")

@dataclass
class Command:
    key: str
    param: str
    addr: int = 254

    def encode(self) -> bytes:
        if self.addr not in range(256):
            raise ProtocolError("Address must range between 0 and 255")
        return f"@{self.addr:03}{self.key}!{self.param};FF".encode(encoding="ascii")

@dataclass
class Ack:
    data: str
    addr: int

    def ensure_ack(self) -> Ack:
        return self

@dataclass
class Nak:
    err_code: NakCode
    addr: int

    def ensure_ack(self) -> Ack:
        raise ResponseError(self.err_code)

def read_response(src: Serial) -> Ack | Nak:
    '''
    Reads a response from the serial port.
    
    :param src: The serial port to read.
    :type src: serial.Serial
    '''

    src.timeout = 0.1
    src.read
    
    # skip bytes until header
    src.read_until(b"@")

    # read header (address and response type)
    header = src.read(6)
    addr = int(header[0:3], base=10)
    resp = header[3:6]

    result = None

    try:
        if addr not in range(256):
            raise ValueError(f"{addr} isn't a valid address")

        # read data
        if resp == b"ACK":
            data = src.read_until(b";")[:-1].decode("ascii")
            result = Ack(data, addr)
            pass
        elif resp == b"NAK":
            code = int(src.read_until(b";")[:-1], base=10)
            result = Nak(NakCode(code), addr)
            pass
        else:
            raise ValueError(f"{resp} isn't a valid response code")
        
        # terminator sequence
        if src.read(2) != b"FF":
            raise ValueError("Failed to read terminator")
            
    except ValueError as err:
        raise ProtocolError("Failed to read response") from err
    
    return result

def send_message(src: Serial, msg: Query | Command) -> Ack | Nak:
    src.write(msg.encode())
    return read_response(src)

# RESULT DATA TYPES
# =======================================
