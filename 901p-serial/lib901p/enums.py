from enum import Enum, StrEnum

class NakCode(Enum):
    '''
    Error code for NAK responses from the sensor.
    '''

    VAC_FAIL = 8
    'The VAC command (calibrating zero for the Pirani element) failed.'
    ATM_FAIL = 9
    'The ATM command (calibrating atmospheric for the Pirani element) failed.'
    INVALID_MSG = 160
    'Unrecognized message.'
    INVALID_ARG = 169
    'Invalid argument.'
    OUT_OF_RANGE = 172
    'Argument out of range.'
    SETUP_LOCKED = 180
    'Setup locked by manufacturer.'

class ReadSource(StrEnum):
    '''
    Source to read pressure info from.
    '''

    PIRANI = "1"
    'Corresponds to PR1 (the Pirani sensor).'
    PIEZO = "2"
    'Corresponds to PR1 (the piezo sensor).'
    COMBINED = "3"
    'Corresponds to PR3 (the combined reading).'

class Gas(StrEnum):
    '''
    The gas whose pressure is being measured.
    '''

    NITROGEN = "NITROGEN"
    AIR = "AIR"
    ARGON = "ARGON"
    HELIUM = "HELIUM"
    HYDROGEN = "HYDROGEN"
    H2O = "H2O"
    NEON = "NEON"
    CO2 = "CO2"
    XENON = "XENON"


class StatusCode(StrEnum):
    '''
    Status code returned by the "T" query.
    '''

    OK = "O"
    'No errors have been detected.'
    PIRANI_FAIL = "M"
    'The Pirani sensor failed.'
    PIEZO_FAIL = "Z"
    'The piezo sensor failed.'

