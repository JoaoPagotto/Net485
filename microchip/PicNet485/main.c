#include <xc.h>
#include "ConfigDevice.h"
#include "Profile.h"
#include "Net485.h"

#define Address     AddrSlave05
#define BaudRate    115200
#define BufferSize  512

char buffer[BufferSize];

void callbackOnReceive(
        unsigned char event,
        char *buffer,
        unsigned short length);

void callbackOnReceiveBroadcast(
        unsigned char event,
        char *buffer,
        unsigned short length);

//------------------------------------------------------------------------------

void setup();
void loop();
void delayMs(unsigned int msecs);

//------------------------------------------------------------------------------

void main(void) {
    setup();
    while (1) {
        loop();
    }
}

//------------------------------------------------------------------------------

void setup() {
    // Default all pins to digital
    ADCON1 |= 0x0F;

    LED_TRIS = OUTPUT;
    LED = LOW;

    net485_open(Address, BaudRate, buffer, BufferSize);
    net485_registerCallbackOnReceive(callbackOnReceive);
    net485_registerCallbackOnReceiveBroadcast(callbackOnReceiveBroadcast);
}

//------------------------------------------------------------------------------

void loop() {

}

//------------------------------------------------------------------------------

void delayMs(unsigned int msecs) {
    for (unsigned int i = 0; i != msecs; i++) {
        __delay_ms(1);
    }
}

//-----------------------------------------------------------------------------

void callbackOnReceive(
        unsigned char event,
        char *buffer,
        unsigned short length) {

    switch (event) {
        case 0x00:
        {
            LED = HIGH;
            break;
        }
        case 0x01:
        {
            LED = LOW;
            break;
        }
        case 0x02:
        {
            if (length > 0) {
                buffer[0] = '[';
                buffer[length - 1] = ']';
            }

            if (net485_write(event, buffer, length) == 0) {
                LED = HIGH;
            }

            break;
        }
        case 0x03:
        {
            if (length > 0) {
                buffer[0] = '(';
                buffer[length - 1] = ')';
            }

            net485_write_only(event, buffer, length);
            LED = HIGH;
            break;
        }
    }
}

//-----------------------------------------------------------------------------

void callbackOnReceiveBroadcast(
        unsigned char event,
        char *buffer,
        unsigned short length) {

    switch (event) {
        case 0x00:
        {
            LED = HIGH;
            break;
        }
        case 0x01:
        {
            LED = LOW;
            break;
        }
    }
}

//-----------------------------------------------------------------------------