#include "Net485.h"

#define Address     AddrSlave01
#define Baudrate    B115200
#define TxPin       2
#define BufferSize  512

Net485 net485;

char buffer[BufferSize];

//-----------------------------------------------------------------------------

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  net485.registerCallbackOnReceive(&CallbackOnReceive);
  net485.registerCallbackOnReceiveBroadcast(&CallbackOnReceiveBroadcast);
  net485.open(Address, Baudrate, TxPin, buffer, BufferSize);
}

//-----------------------------------------------------------------------------

void loop() {

}

//-----------------------------------------------------------------------------

void serialEvent() {
  net485.serialEvent();
}

//-----------------------------------------------------------------------------

void CallbackOnReceive(
  unsigned char event,
  char *buffer,
  unsigned short length)
{
  switch ( event )
  {
    case 0x00:
      {
        digitalWrite(LED_BUILTIN, HIGH);
        break;
      }
    case 0x01:
      {
        digitalWrite(LED_BUILTIN, LOW);
        break;
      }
    case 0x02:
      {
        if ( length > 0 ) {
          buffer[0] = '[';
          buffer[length - 1] = ']';
        }

        if ( net485.write(event, buffer, length) == 0 ) {
          digitalWrite(LED_BUILTIN, HIGH);
        }
        break;
      }
    case 0x03:
      {
        if ( length > 0 ) {
          buffer[0] = '(';
          buffer[length - 1] = ')';
        }

        net485.writeOnly(event, buffer, length);
        digitalWrite(LED_BUILTIN, HIGH);
        break;
      }
  }
}

//-----------------------------------------------------------------------------

void CallbackOnReceiveBroadcast(
  unsigned char event,
  char *buffer,
  unsigned short length)
{
  switch ( event )
  {
    case 0x00:
      {
        digitalWrite(LED_BUILTIN, HIGH);
        break;
      }
    case 0x01:
      {
        digitalWrite(LED_BUILTIN, LOW);
        break;
      }
  }
}

//-----------------------------------------------------------------------------



