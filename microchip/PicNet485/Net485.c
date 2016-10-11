#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

#include "Net485.h"
#include "ConfigDevice.h"
#include "Profile.h"

#pragma warning disable 1090
#pragma warning disable 759
#pragma warning disable 520

#define LOBYTE(x) ((unsigned char) ((x)      & 0xff))
#define HIBYTE(x) ((unsigned char) ((x) >> 8 & 0xff))

#define CrcPolynomial   0x1021
#define CrcBits         16
#define CrcBitsPerChar  8

static unsigned short CrcTable[1 << CrcBitsPerChar] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

//-----------------------------------------------------------------------------

long baudRate;
unsigned char address;
unsigned short bufferSize;

unsigned int timeoutRX;

unsigned char lastError;
Status status;

ProcCallbackOnReceive procCallbackOnReceive;
ProcCallbackOnReceiveBroadcast procCallbackOnReceiveBroadcast;

//---------------------------------

unsigned char state;
unsigned short calc;
unsigned short pos;
unsigned short crc;

unsigned char method;
unsigned char addrSource;
unsigned char addrDestination;
unsigned char event;
unsigned short length;
unsigned char response;

char *rs485_buffer;

//---------------------------------

unsigned char readRequest();
unsigned char readResponse();

//---------------------------------

unsigned char writeRequest(
        unsigned char event,
        char *buffer,
        unsigned short length);

unsigned char writeResponse();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void serialOpen(const long int baudrate) {
    unsigned int calcBaudRate;

    //-----------------------------
    // Calculate BaudRate

    // SPBRG for High Baud Rate
    calcBaudRate = (_XTAL_FREQ - baudrate * 16) / (baudrate * 16);
    if (calcBaudRate < 256) { // If High Baud Rate required
        TXSTAbits.BRGH = 1; // Setting High Baud Rate
    } else {
        // SPBRG for Low Baud Rate
        calcBaudRate = (_XTAL_FREQ - baudrate * 64) / (baudrate * 64);
        TXSTAbits.BRGH = 0; // Setting Low Baud Rate
    }

    if (calcBaudRate < 256) {

        //-----------------------------
        // SPBRGH EUSART Baud Rate Generator Register High Byte
        // SPBRG  EUSART Baud Rate Generator Register Low Byte

        SPBRG = calcBaudRate;

        //-----------------------------
        // TRIS/PORT Usart Config

        RS485_RO_TRIS = INPUT;
        RS485_RO_IO = LOW;

        RS485_DE_TRIS = OUTPUT;
        RS485_DE_IO = LOW;

        RS485_DI_TRIS = OUTPUT;
        RS485_DI_IO = LOW;

        //-----------------------------
        // RCSTA: RECEIVE STATUS AND CONTROL REGISTER

        RCSTAbits.SPEN = 1; // Enable serial port pins
        RCSTAbits.CREN = 1; // Enable the receiver 
        RCSTAbits.RX9 = 0; // 8-bit reception 
        RCSTAbits.ADDEN = 0; // Address Detect disabled			

        //-----------------------------
        // TXSTA: TRANSMIT STATUS AND CONTROL REGISTER

        TXSTAbits.SYNC = 0; // Asynchronous 
        TXSTAbits.TXEN = 1; // Enable the transmitter 
        TXSTAbits.TX9 = 0; // 8-bit transmission 

        //-----------------------------
        // PIE1: PERIPHERAL INTERRUPT ENABLE REGISTER 1

        PIE1bits.TXIE = 0; // Disable tx interrupts 
        PIE1bits.RCIE = 1; // Enable rx interrupts 

        //-----------------------------
        // INTCON: INTERRUPT CONTROL REGISTER

        INTCONbits.PEIE = 1; // Enable peripheral interrupts
        INTCONbits.GIEH = 1; // Enable all interrupts        
    }
}

//-----------------------------------------------------------------------------

unsigned char serialAvailable() {
    for (unsigned short counter = 0; counter < timeoutRX; counter++) {
        if (PIR1bits.RCIF) {
            return 1;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------

char serialRead() {
    return RCREG;
}

//-----------------------------------------------------------------------------

void serialWrite(char data) {
    TXREG = data;
    while (!TXSTAbits.TRMT);
}

//-----------------------------------------------------------------------------

void CRC16Buffer(unsigned short *crc, const unsigned char *buffer, unsigned int length) {
    while (length--) {
        *crc = (*crc << CrcBitsPerChar) ^
                CrcTable[(*crc >> (CrcBits - CrcBitsPerChar)) ^
                *buffer++];
    }
}

//-----------------------------------------------------------------------------

void CRC16Byte(unsigned short *crc, const unsigned char data) {
    *crc = (*crc << CrcBitsPerChar) ^
            CrcTable[(*crc >> (CrcBits - CrcBitsPerChar)) ^
            data];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void net485_open(
        unsigned char _address,
        const long int _baudRate,
        char *_buffer,
        unsigned short _bufferSize) {
    state = srSOH;

    baudRate = _baudRate;
    address = _address;
    rs485_buffer = _buffer;
    bufferSize = _bufferSize;

    lastError = speNoError;

    procCallbackOnReceive = NULL;
    procCallbackOnReceiveBroadcast = NULL;

    net485_setTimeoutRX(16384); // 16384 cycles CPU
    serialOpen(baudRate);
}

//-----------------------------------------------------------------------------

void net485_setTimeoutRX(unsigned int _timeoutRX) {
    timeoutRX = _timeoutRX;
}

//-----------------------------------------------------------------------------

unsigned char net485_getLastError() {
    return lastError;
}

//-----------------------------------------------------------------------------

void net485_resetStatus() {
    status.counterBytesRX = 0;
    status.counterBytesTX = 0;
    status.counterInvalidPackRX = 0;
    status.counterInvalidPackTX = 0;
    status.counterTimeoutRX = 0;
    status.counterTimeoutTX = 0;
    status.counterValidPackRX = 0;
    status.counterValidPackTX = 0;
}

//-----------------------------------------------------------------------------

Status net485_getStatus() {
    return status;
}

//-----------------------------------------------------------------------------

void net485_registerCallbackOnReceive(
        ProcCallbackOnReceive _procCallbackOnReceive) {
    procCallbackOnReceive = _procCallbackOnReceive;
}

//-----------------------------------------------------------------------------

void net485_registerCallbackOnReceiveBroadcast(
        ProcCallbackOnReceiveBroadcast _procCallbackOnReceiveBroadcast) {
    procCallbackOnReceiveBroadcast = _procCallbackOnReceiveBroadcast;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#if defined(NET485)

void interrupt SerialRxPinInterrupt(void) {
    // Check if the interrupt is caused by RX pin
    if ((PIR1bits.RCIF == 1) && (PIE1bits.RCIE == 1)) {
        PIE1bits.RCIE = 0; // Disable UART receive interrupt

        if (readRequest() == RetValidPack) {

            if (addrDestination == address) {

                if (method == MethodWriteResponse) {
                    readResponse();
                }

                if (procCallbackOnReceive != NULL) {
                    procCallbackOnReceive(event, rs485_buffer, length);
                }
            } else if (addrDestination == AddrBroadcast) {
                if (procCallbackOnReceiveBroadcast != NULL) {
                    procCallbackOnReceiveBroadcast(event, rs485_buffer, length);
                }
            }

        }

        PIR1bits.RCIF = 0; // Clear Receive Interrupt Flag
        PIE1bits.RCIE = 1; // Enable UART receive interrupt
    }
}
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

unsigned char net485_read(
        unsigned char *_event,
        char *_buffer,
        unsigned short *_length) {
    PIE1bits.RCIE = 0; // Disable UART receive interrupt

    unsigned char ret = readRequest();
    if (ret == RetValidPack) {
        if ((addrDestination == address) &&
                (method == MethodWriteResponse)) {
            ret = readResponse();
        }

        *_event = event;
        _buffer = rs485_buffer;
        *_length = length;
    }

    PIE1bits.RCIE = 1; // Enable UART receive interrupt

    return ret;
}

//------------------------------------------------------------------------------

unsigned char net485_read_only(
        unsigned char *_event,
        char *_buffer,
        unsigned short *_length) {
    PIE1bits.RCIE = 0; // Disable UART receive interrupt

    unsigned char ret = readRequest();
    if (ret == RetValidPack) {
        *_event = event;
        _buffer = rs485_buffer;
        *_length = length;
    }

    PIE1bits.RCIE = 1; // Enable UART receive interrupt

    return ret;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

unsigned char net485_write(
        unsigned char _event,
        char *_buffer,
        unsigned short _length) {
    method = MethodWriteResponse;

    unsigned char ret = writeRequest(_event, _buffer, _length);
    if (ret == RetValidPack) {
        if ((addrDestination != AddrBroadcast) &&
                (method == MethodWriteResponse)) {
            ret = writeResponse();
        }
    }
    return ret;
}

//-----------------------------------------------------------------------------

unsigned char net485_write_only(
        unsigned char _event,
        char *_buffer,
        unsigned short _length) {
    method = MethodWriteOnly;

    unsigned char ret = writeRequest(_event, _buffer, _length);
    return ret;
}

//-----------------------------------------------------------------------------

unsigned char net485_write_addr(
        unsigned char _addrDestination,
        unsigned char _event,
        char *_buffer,
        unsigned short _length) {

    addrSource = address;
    addrDestination = _addrDestination;
    method = MethodWriteResponse;

    unsigned char ret = writeRequest(_event, _buffer, _length);
    if (ret == RetValidPack) {
        if ((addrDestination != AddrBroadcast) &&
                (method == MethodWriteResponse)) {
            ret = writeResponse();
        }
    }
    return ret;
}

//-----------------------------------------------------------------------------

unsigned char net485_write_addr_only(
        unsigned char _addrDestination,
        unsigned char _event,
        char *_buffer,
        unsigned short _length) {

    addrSource = address;
    addrDestination = _addrDestination;
    method = MethodWriteOnly;

    unsigned char ret = writeRequest(_event, _buffer, _length);
    return ret;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

unsigned char readRequest() {

    while (serialAvailable()) {
        unsigned char data = (unsigned char) serialRead();

        status.counterBytesRX++;

        switch (state) {
            case srSOH:
            {
                if (data == ByteSOH) {
                    state = srMethod;
                    calc = 0x00;
                    CRC16Byte(&calc, data);
                    continue;
                } else if (data == ByteSYNC) {
                    continue;
                }
                break;
            }
            case srMethod:
            {
                method = data;
                state = srAddressSource;
                CRC16Byte(&calc, data);
                continue;
            }
            case srAddressSource:
            {
                addrSource = data;
                state = srAddressDestination;
                CRC16Byte(&calc, data);
                continue;
            }
            case srAddressDestination:
            {
                if ((data == address) ||
                        (data == AddrBroadcast)) {
                    addrDestination = data;
                    state = srEvent;
                    CRC16Byte(&calc, data);
                    continue;
                }
                break;
            }
            case srEvent:
            {
                event = data;
                state = srLengthLo;
                CRC16Byte(&calc, data);
                continue;
            }
            case srLengthLo:
            {
                length = (data & 0xff);
                state = srLengthHi;
                CRC16Byte(&calc, data);
                continue;
            }
            case srLengthHi:
            {
                length |= (data & 0xff) << 8;
                if (length > 0) {
                    if (length <= bufferSize) {
                        state = srSTX;
                    } else {
                        break;
                    }
                } else {
                    state = srCrcLo;
                }
                CRC16Byte(&calc, data);
                continue;
            }
            case srSTX:
            {
                if (data == ByteSTX) {
                    state = srBuffer;
                    pos = 0;
                    CRC16Byte(&calc, data);
                    continue;
                }
                break;
            }
            case srBuffer:
            {
                rs485_buffer[pos] = data;
                pos++;
                if (pos == length) {
                    state = srETX;
                }
                CRC16Byte(&calc, data);
                continue;
            }
            case srETX:
            {
                if (data == ByteETX) {
                    state = srCrcLo;
                    CRC16Byte(&calc, data);
                    continue;
                }
                break;
            }
            case srCrcLo:
            {
                crc = (data & 0xff);
                state = srCrcHi;
                continue;
            }
            case srCrcHi:
            {
                crc |= (data & 0xff) << 8;
                if (crc == calc) {
                    state = srEOT;
                } else {
                    state = srINVALID;
                }
                continue;
            }
            case srEOT:
            {
                if (data == ByteEOT) {
                    response = ByteACK;
                    status.counterValidPackRX++;
                } else {
                    response = ByteNAK;
                    status.counterInvalidPackRX++;
                }
                state = srSOH;
                return RetValidPack;
            }
            case srINVALID:
            {
                response = ByteNAK;
                status.counterInvalidPackRX++;
                state = srSOH;
                return RetValidPack;
            }
        }

        state = srSOH;
    }

    status.counterTimeoutRX++;
    return RetTimeoutRX;
}

//-----------------------------------------------------------------------------

unsigned char readResponse() {

    RS485_DE_IO = HIGH;
    Nop();
    Nop();

    //----------------------

    serialWrite(ByteSYNC);
    serialWrite(ByteSOH);
    serialWrite(method);
    serialWrite(addrSource);
    serialWrite(addrDestination);
    serialWrite(event);
    serialWrite(response);

    //----------------------

    status.counterBytesTX += 7;

    Nop();
    Nop();
    RS485_DE_IO = LOW;

    status.counterValidPackTX++;
    return RetValidPack;
}

//-----------------------------------------------------------------------------

unsigned char writeRequest(
        unsigned char _event,
        char *_buffer,
        unsigned short _length) {

    crc = 0x00;
    event = _event;
    length = _length;

    RS485_DE_IO = HIGH;
    Nop();
    Nop();

    //----------------------

    serialWrite(ByteSYNC);

    serialWrite(ByteSOH);
    CRC16Byte(&crc, ByteSOH);

    serialWrite(method);
    CRC16Byte(&crc, method);

    serialWrite(addrSource);
    CRC16Byte(&crc, addrSource);

    serialWrite(addrDestination);
    CRC16Byte(&crc, addrDestination);

    serialWrite(event);
    CRC16Byte(&crc, event);

    serialWrite(LOBYTE(length));
    CRC16Byte(&crc, LOBYTE(length));
    serialWrite(HIBYTE(length));
    CRC16Byte(&crc, HIBYTE(length));

    if (length > 0) {
        serialWrite(ByteSTX);
        CRC16Byte(&crc, ByteSTX);

        for (int x = 0; x < length; x++) {
            serialWrite(_buffer[x]);
            CRC16Byte(&crc, _buffer[x]);
        }

        serialWrite(ByteETX);
        CRC16Byte(&crc, ByteETX);
    }

    serialWrite(LOBYTE(crc));
    serialWrite(HIBYTE(crc));

    serialWrite(ByteEOT);

    //----------------------

    status.counterBytesTX += 7 + length;

    Nop();
    Nop();
    RS485_DE_IO = LOW;

    status.counterValidPackTX++;
    return RetValidPack;
}

//-----------------------------------------------------------------------------

unsigned char writeResponse() {

    unsigned char _state = saSOH;

    while (serialAvailable()) {
        unsigned char data = (unsigned char) serialRead();

        status.counterBytesRX++;

        switch (_state) {
            case saSOH:
            {
                if (data == ByteSOH) {
                    _state = saMethod;
                    continue;
                } else if (data == ByteSYNC) {
                    continue;
                }
                break;
            }
            case saMethod:
            {
                if (data == method) {
                    _state = saAddressSource;
                    continue;
                }
                break;
            }
            case saAddressSource:
            {
                if (data == addrSource) {
                    _state = saAddressDestination;
                    continue;
                }
                break;
            }
            case saAddressDestination:
            {
                if (data == addrDestination) {
                    _state = saEvent;
                    continue;
                }
                break;
            }
            case saEvent:
            {
                if (data == event) {
                    _state = saAnswer;
                    continue;
                }
                break;
            }
            case saAnswer:
            {
                if (data == ByteACK) {
                    status.counterValidPackRX++;
                    return RetValidPack;
                } else if (data == ByteNAK) {
                    status.counterInvalidPackRX++;
                    return RetInvalidTxPack;
                }
                break;
            }
        }

        status.counterInvalidPackRX++;
        return RetInvalidRxPack;
    }

    status.counterTimeoutRX++;
    return RetTimeoutRX;
}

//-----------------------------------------------------------------------------
