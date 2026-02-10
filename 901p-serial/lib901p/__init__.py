from serial import Serial, SerialTimeoutException
import pathlib
import os
from . import msg
from .enums import *

class ResponseError(Exception):
    '''
    Error due to NACK code.
    '''
    def __init__(self, *args):
        super().__init__(self, *args)

def _try_open_with_baud(path: str | bytes | os.PathLike, baud: int) -> Serial | None:
    # open connection with baud rate
    conn = Serial(
        os.fsdecode(path), 
        baudrate=baud, 
        bytesize=8, 
        parity="N", 
        stopbits=1, 
        xonxoff=False,
        timeout=200/baud
    )

    # attempt to confirm baud rate
    resp = None
    try:
        resp = msg.send_message(conn, msg.Query("BR"))
    except msg.ProtocolError:
        return None

    if isinstance(resp, msg.Nak):
        raise ResponseError(resp.err_code)
    
    return conn

_BAUD_RATES = [9600, 115200, 4800, 19200, 38400, 57600, 230400]

class PressureSensor:
    '''
    An MKS 901P vacuum pressure transducer, connected over RS-232 serial.

    This class may be used as a context manager:
    ```
    with lib901p.PressureSensor("/dev/ttyUSB0", 9600) as sensor:
        # do stuff here
        pass
    ```

    ## Reading notes
    - As per manufacturer specifications, measurement accuracy is limited at low pressures:
        - From 1.0 × 10<sup>-5</sup> to 1.0 × 10<sup>-4</sup> torr: 1 significant figure 
        - From 1.0 × 10<sup>-4</sup> to 1.0 × 10<sup>-3</sup> torr: 2 significant figures
        - From 1.0 × 10<sup>-3</sup> to 900 torr: 3–4 significant figures
    '''

    _conn: Serial

    def __init__(self, path: str | bytes | os.PathLike, baud: int | None = None):
        '''
        Constructs a new `PressureSensor`.
        
        :param path: Path to a file.
        :type path: str | bytes | os.PathLike
        :param baud: (optional) the baud rate. If unspecified, tries all possible baud rates until one works.
        :type baud: int | None
        '''

        if baud != None:
            conn = _try_open_with_baud(path, baud)
            if conn == None:
                raise Exception(f"Failed to init @ {baud} baud!")
            self._conn = conn
        else:
            for test_rate in _BAUD_RATES:
                conn = _try_open_with_baud(path, test_rate)
                if conn != None:
                    self._conn = conn
                    break
            else:
                raise Exception("Failed to init after testing all baud rates!")
        pass
        
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._conn.__exit__(exc_type, exc_val, exc_tb)

    # RAW REQUEST HANDLING
    # ===============================================

    def query_raw(self, key: str) -> str:
        '''
        Sends a raw query-type message (e.g. `@254[key]?;FF`)
        
        :param key: The command's key.
        :type key: str
        :return: The response data.
        :rtype: str
        '''
        resp = msg.send_message(self._conn, msg.Query(key)).ensure_ack()
        return resp.data
    
    def command_raw(self, key: str, param: str) -> str:
        '''
        Sends a raw command-type message (e.g. `@254[key]![param];FF`)
        
        :param key: The command's key.
        :type key: str
        :param data: The command's data.
        :type data: str
        :return: The response data.
        :rtype: str
        '''
        resp = msg.send_message(self._conn, msg.Command(key, param)).ensure_ack()
        return resp.data
    
    # PRESSURE READING
    # ===============================================

    def read_pirani(self) -> float:
        '''
        Reads off the measurement from the Pirani sensor.
        
        :return: The measured pressure. Correct to 3 significant digits.
        :rtype: float
        '''
        return float(self.query_raw("PR1"))

    def read_piezo(self) -> float:
        '''
        Reads off the (differential) measurement from the piezo sensor.
        The answer is precise to 3 significant digits.
        
        :return: The measured pressure. Correct to 3 significant digits.
        :rtype: float
        '''
        return float(self.query_raw("PR2"))
    
    def read_combined(self) -> float:
        '''
        Obtains a combined reading using both sensors.
        
        :return: The measured pressure. Correct to 4 significant digits.
        :rtype: float
        '''
        return float(self.query_raw("PR4"))

    # OUTPUT CONFIG
    # ===============================================

    def analog_src(self) -> ReadSource:
        return ReadSource(self.query_raw("AO1")[0])

    def set_analog1_src(self, source: ReadSource) -> None:
        # param[0]: [1/2/3], reading source
        # param[1:]: 0, use 1 VDC/decade output
        self.command_raw("AO1", str(source) + "0")

    def calibration_gas(self) -> Gas:
        return Gas(self.query_raw("GT"))
    
    def set_calibration_gas(self, gas: Gas) -> None:
        self.command_raw("GT", str(gas))
    
    # OTHER QUERIES
    # ===============================================

    def status(self) -> StatusCode:
        '''
        Checks the status.
        
        :return: The status code.
        :rtype: StatusCode
        '''
        return StatusCode(self.query_raw("T"))
    
    