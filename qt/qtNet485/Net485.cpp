#include "Net485.h"
#include <QCoreApplication>

#define LOBYTE(x) ( ((x)      & 0xff))
#define HIBYTE(x) ( ((x) >> 8 & 0xff))

#define CrcPolynomial	0x1021
#define CrcBits			16
#define CrcBitsPerChar	8

static unsigned short CrcTable[1 << CrcBitsPerChar] =
{
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

void CRC16Buffer(unsigned short *crc, const unsigned char *buffer, unsigned int length)
{
   while ( length-- ) {
      *crc = (*crc << CrcBitsPerChar) ^
            CrcTable[(*crc >> (CrcBits - CrcBitsPerChar)) ^
            *buffer++];
   }
}

//-----------------------------------------------------------------------------

void CRC16Byte(unsigned short *crc, const unsigned char data)
{
   *crc = (*crc << CrcBitsPerChar) ^
         CrcTable[(*crc >> (CrcBits - CrcBitsPerChar)) ^
         data];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Net485::Net485(QObject *parent) : QObject(parent)
{
   qRegisterMetaType<SerialPortError>("SerialPortError");

   this->address    = AddrSlave01;
   this->bufferSize = 0;

   this->timeoutTX = 500;
   this->timeoutRX = 500;

   this->buffer = NULL;

   this->lastError = speNoError;
   resetStatus();
}

//-----------------------------------------------------------------------------

Net485::~Net485()
{
   close();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool Net485::open(
      unsigned char address,
      const QString &port,
      qint32 baudRate,
      char *buffer,
      unsigned short bufferSize)
{
   this->address = address;
   this->buffer = buffer;
   this->bufferSize = bufferSize;

   resetStatus();

   serialPort.close();
   serialPort.setBaudRate(baudRate);
   serialPort.setDataBits(QSerialPort::Data8);
   serialPort.setParity(QSerialPort::NoParity);
   serialPort.setStopBits(QSerialPort::OneStop);
   serialPort.setFlowControl(QSerialPort::NoFlowControl);
   serialPort.setPortName(port);

   if ( this->address != AddrMaster ) {
      connect(&serialPort, SIGNAL(readyRead()),
              this, SLOT(readyRead()));
   }

   connect(&serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
           this, SLOT(error(QSerialPort::SerialPortError)));

   return serialPort.open(QIODevice::ReadWrite);
}

//-----------------------------------------------------------------------------

void Net485::close()
{
   if ( this->address != AddrMaster ) {
      disconnect(&serialPort, SIGNAL(readyRead()));
      disconnect(&serialPort, SIGNAL(error(QSerialPort::SerialPortError)));
   }

   serialPort.close();
}

//-----------------------------------------------------------------------------

void Net485::setTimeoutRX(unsigned int timeoutRX)
{
   this->timeoutRX = timeoutRX;
}

//-----------------------------------------------------------------------------

void Net485::setTimeoutTX(unsigned int timeoutTX)
{
   this->timeoutTX = timeoutTX;
}

//-----------------------------------------------------------------------------

SerialPortError Net485::getLastError()
{
   return lastError;
}

//-----------------------------------------------------------------------------

void Net485::resetStatus()
{
   memset(&this->status, 0, sizeof(Status));
}

//-----------------------------------------------------------------------------

Status Net485::getStatus()
{
   return status;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

unsigned char Net485::read(
      unsigned char *event,
      char *buffer,
      unsigned short *length)
{
   unsigned char ret = readRequest();
   if ( ret == RetValidPack ) {
      if ((this->addrDestination == this->address) &&
          (this->method == MethodWriteResponse)) {
         ret = readResponse();
      }

      *event  = this->event;
      buffer = this->buffer;
      *length = this->length;
   }

   if ( ret != RetValidPack ) {
      serialPort.clear();
   }

   return ret;
}

//-----------------------------------------------------------------------------

unsigned char Net485::readOnly(
      unsigned char *event,
      char *buffer,
      unsigned short *length)
{
   unsigned char ret = readRequest();
   if ( ret == RetValidPack ) {
      *event  = this->event;
      buffer = this->buffer;
      *length = this->length;
   }

   if ( ret != RetValidPack ) {
      serialPort.clear();
   }

   return ret;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

unsigned char Net485::write(
      unsigned char event,
      char *buffer,
      unsigned short length)
{
   this->method = MethodWriteResponse;

   unsigned char ret = writeRequest(event, buffer, length);
   if ( ret == RetValidPack ) {
      if ((this->addrDestination != AddrBroadcast) &&
          (this->method == MethodWriteResponse)) {
         ret = writeResponse();
      }
   }

   if ( ret != RetValidPack ) {
      serialPort.clear();
   }

   return ret;
}

//-----------------------------------------------------------------------------

unsigned char Net485::writeOnly(
      unsigned char event,
      char *buffer,
      unsigned short length)
{
   this->method = MethodWriteOnly;

   unsigned char ret = writeRequest(event, buffer, length);
   if ( ret != RetValidPack ) {
      serialPort.clear();
   }
   return ret;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

unsigned char Net485::write(
      unsigned char addrDestination,
      unsigned char event,
      char *buffer,
      unsigned short length)
{
   this->addrSource = address;
   this->addrDestination = addrDestination;
   this->method = MethodWriteResponse;

   unsigned char ret = writeRequest(event, buffer, length);
   if ( ret == RetValidPack ) {
      if ((this->addrDestination != AddrBroadcast) &&
          (this->method == MethodWriteResponse)) {
         ret = writeResponse();
      }
   }

   if ( ret != RetValidPack ) {
      serialPort.clear();
   }

   return ret;
}

//-----------------------------------------------------------------------------

unsigned char Net485::writeOnly(
      unsigned char addrDestination,
      unsigned char event,
      char *buffer,
      unsigned short length)
{
   this->addrSource = address;
   this->addrDestination = addrDestination;
   this->method = MethodWriteOnly;

   unsigned char ret = writeRequest(event, buffer, length);
   if ( ret != RetValidPack ) {
      serialPort.clear();
   }
   return ret;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void Net485::readyRead()
{
   if ( readRequest() == RetValidPack ) {
      if (this->addrDestination == address) {
         if (this->method == MethodWriteResponse) {
            readResponse();
         }

         emit onReceive(event, buffer, length);
      } else if (addrDestination == AddrBroadcast ) {
         emit onReceiveBroadcast(event, buffer, length);
      }
   } else {
      serialPort.clear();
   }
}

//-----------------------------------------------------------------------------

void Net485::error(QSerialPort::SerialPortError error)
{
   switch ( error ) {
      case QSerialPort::NoError							: { lastError = speNoError; break; }
      case QSerialPort::DeviceNotFoundError			: { lastError = speDeviceNotFoundError; break; }
      case QSerialPort::NotOpenError					: { lastError = speNotOpenError; break; }
      case QSerialPort::OpenError						: { lastError = speOpenError; break; }
      case QSerialPort::PermissionError				: { lastError = spePermissionError; break; }
      case QSerialPort::ReadError						: { lastError = speReadError; break; }
      case QSerialPort::ResourceError					: { lastError = speResourceError; break; }
      case QSerialPort::TimeoutError					: { lastError = speTimeoutError; break; }
      case QSerialPort::UnknownError					: { lastError = speUnknownError; break; }
      case QSerialPort::UnsupportedOperationError	: { lastError = speUnsupportedOperationError; break; }
      case QSerialPort::WriteError						: { lastError = speWriteError; break; }
   }
   emit onError(lastError);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

unsigned char Net485::readRequest()
{
   state = srSOH;

   QCoreApplication::processEvents();

   do {
      QByteArray rx = serialPort.readAll();

      this->status.counterBytesRX += rx.size();

      foreach(const unsigned char data, rx)
      {

         switch ( state )
         {
            case srSOH: {
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
            case srMethod: {
               method = data;
               state = srAddressSource;
               CRC16Byte(&calc, data);
               continue;
            }
            case srAddressSource: {
               addrSource = data;
               state = srAddressDestination;
               CRC16Byte(&calc, data);
               continue;
            }
            case srAddressDestination: {
               addrDestination = data;
               state = srEvent;
               CRC16Byte(&calc, data);
               continue;
               break;
            }
            case srEvent: {
               event = data;
               state = srLengthLo;
               CRC16Byte(&calc, data);
               continue;
            }
            case srLengthLo: {
               length = (data & 0xff);
               state = srLengthHi;
               CRC16Byte(&calc, data);
               continue;
            }
            case srLengthHi: {
               length |= (data & 0xff) << 8;
               if ( length > 0 ) {
                  if ( length <= bufferSize ) {
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
            case srSTX: {
               if (data == ByteSTX) {
                  state = srBuffer;
                  pos = 0;
                  CRC16Byte(&calc, data);
                  continue;
               }
               break;
            }
            case srBuffer: {
               buffer[pos] = data;
               pos++;
               if (pos == length) {
                  state = srETX;
               }
               CRC16Byte(&calc, data);
               continue;
            }
            case srETX: {
               if (data == ByteETX) {
                  state = srCrcLo;
                  CRC16Byte(&calc, data);
                  continue;
               }
               break;
            }
            case srCrcLo: {
               crc = (data & 0xff);
               state = srCrcHi;
               continue;
            }
            case srCrcHi: {
               crc |= (data & 0xff) << 8;
               if ( crc == calc ) {
                  state = srEOT;
               } else {
                  state = srINVALID;
               }
               continue;
            }
            case srEOT: {
               if (data == ByteEOT) {
                  this->response = ByteACK;
                  this->status.counterValidPackRX++;
               } else {
                  this->response = ByteNAK;
                  this->status.counterInvalidPackRX++;
               }
               this->state = srSOH;
               return RetValidPack;
               break;
            }
            case srINVALID: {
               this->response = ByteNAK;
               this->status.counterInvalidPackRX++;
               this->state = srSOH;
               return RetInvalidRxPack;
               break;
            }
         }

         this->status.counterInvalidPackRX++;
         return RetInvalidRxPack;
      }
   } while ( serialPort.waitForReadyRead(timeoutRX) );

   this->status.counterTimeoutRX++;
   return RetTimeoutRX;
}

//-----------------------------------------------------------------------------

unsigned char Net485::readResponse()
{
   QByteArray tx;

   tx.append(ByteSYNC);
   tx.append(ByteSOH);
   tx.append(this->method);
   tx.append(this->addrSource);
   tx.append(this->addrDestination);
   tx.append(this->event);
   tx.append(this->response);

   this->status.counterBytesTX += tx.size();

   serialPort.write(tx);
   if ( serialPort.waitForBytesWritten(timeoutTX) ) {
      this->status.counterValidPackTX++;
      return RetValidPack;
   }

   this->status.counterTimeoutTX++;
   return RetTimeoutTX;
}

//-----------------------------------------------------------------------------

unsigned char Net485::writeRequest(
      unsigned char event,
      char *buffer,
      unsigned short length)
{
   QByteArray tx;

   QCoreApplication::processEvents();

   this->crc = 0x00;
   this->event = event;
   this->length = length;

   //----------------------------------

   tx.append(ByteSYNC);

   tx.append(ByteSOH);
   CRC16Byte(&crc, ByteSOH);

   tx.append(this->method);
   CRC16Byte(&crc, this->method);

   tx.append(this->addrSource);
   CRC16Byte(&crc, this->addrSource);

   tx.append(this->addrDestination);
   CRC16Byte(&crc, this->addrDestination);

   tx.append(this->event);
   CRC16Byte(&crc, this->event);

   tx.append(LOBYTE(this->length));
   CRC16Byte(&crc, LOBYTE(this->length));
   tx.append(HIBYTE(this->length));
   CRC16Byte(&crc, HIBYTE(this->length));

   if ( this->length > 0 )
   {
      tx.append(ByteSTX);
      CRC16Byte(&crc, ByteSTX);

      tx.append(buffer, this->length);
      CRC16Buffer(&crc, (const unsigned char *)buffer, length);

      tx.append(ByteETX);
      CRC16Byte(&crc, ByteETX);
   }

   tx.append(LOBYTE(crc));
   tx.append(HIBYTE(crc));

   tx.append(ByteEOT);

   //----------------------------------

   this->status.counterBytesTX += tx.size();

   serialPort.write(tx);
   if ( serialPort.waitForBytesWritten(timeoutTX) ) {
      this->status.counterValidPackTX++;
      return RetValidPack;
   }

   this->status.counterTimeoutRX++;
   return RetTimeoutTX;
}

//-----------------------------------------------------------------------------

unsigned char Net485::writeResponse()
{
   StateAnswer state = saSOH;

   unsigned char buffer[7];

   do {

      qint64 received = serialPort.read((char*)buffer, 7);
      this->status.counterBytesRX += received;

      if ( received == 7 )
      {

         for ( int x=0; x < 7; x++ )
         {
            unsigned char data = buffer[x];

            switch ( state )
            {
               case saSOH: {
                  if (data == ByteSOH) {
                     state = saMethod;
                     continue;
                  } else if (data == ByteSYNC) {
                     continue;
                  }
                  break;
               }
               case saMethod: {
                  if (data == this->method ) {
                     state = saAddressSource;
                     continue;
                  }
                  break;
               }
               case saAddressSource: {
                  if (data == this->addrSource) {
                     state = saAddressDestination;
                     continue;
                  }
                  break;
               }
               case saAddressDestination: {
                  if (data == this->addrDestination) {
                     state = saEvent;
                     continue;
                  }
                  break;
               }
               case saEvent: {
                  if (data == this->event) {
                     state = saAnswer;
                     continue;
                  }
                  break;
               }
               case saAnswer: {
                  if (data == ByteACK) {
                     this->status.counterValidPackRX++;
                     return RetValidPack;
                  } else if (data == ByteNAK) {
                     this->status.counterInvalidPackRX++;
                     return RetInvalidTxPack;
                  }
                  break;
               }
            }
            this->status.counterInvalidPackRX++;
            return RetInvalidRxPack;
         }
      }
   } while ( serialPort.waitForReadyRead(timeoutRX) );

   this->status.counterTimeoutRX++;
   return RetTimeoutRX;
}

//-----------------------------------------------------------------------------
