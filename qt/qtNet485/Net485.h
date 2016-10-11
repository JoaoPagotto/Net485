/******************************************************************************
**
** Copyright (C)2004-2016 SimCORE www.simcore.com.br
** Author: Joao Pagotto
** Contact: contato@simcore.com.br
**
** This file is part of the project Net485.
**
** Version: 2016.10.10
**
** Commercial License Usage
** Licensees holding valid commercial SimCORE licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The SimCORE Company.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3. The licenses are as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL2 and
** LICENSE.GPL3 included in the packaging of this file. Please review the
** following information to ensure the GNU General Public License
** requirements will be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
******************************************************************************/

#ifndef NET485_H
#define NET485_H

#include <QObject>
#include <QSerialPort>

//-----------------------------------------------------------------------------

#define Net485_Version      0x01

#define AddrMaster          0x00
#define AddrSlave01         0x01
#define AddrSlave02         0x02
#define AddrSlave03         0x03
#define AddrSlave04         0x04
#define AddrSlave05         0x05
#define AddrSlave06         0x06
#define AddrSlave07         0x07
#define AddrSlave08         0x08
#define AddrSlave09         0x09
#define AddrSlave10         0x0A
#define AddrSlave11         0x0B
#define AddrSlave12         0x0C
#define AddrSlave13         0x0D
#define AddrSlave14         0x0E
#define AddrSlave15         0x0F
#define AddrSlave16         0x10
#define AddrSlave17         0x11
#define AddrSlave18         0x12
#define AddrSlave19         0x13
#define AddrSlave20         0x14
#define AddrSlave21         0x15
#define AddrSlave22         0x16
#define AddrSlave23         0x17
#define AddrSlave24         0x18
#define AddrSlave25         0x19
#define AddrSlave26         0x1A
#define AddrSlave27         0x1B
#define AddrSlave28         0x1C
#define AddrSlave29         0x1D
#define AddrSlave30         0x1E
#define AddrSlave31         0x1F
#define AddrBroadcast       0xFF

#define AddrSlaveMIN        AddrSlave01
#define AddrSlaveMAX        AddrSlave31

#define ByteSYNC            0x55 // Byte Synchronization
#define ByteSOH             0x01 // Byte Start of Header
#define ByteSTX             0x02 // Byte Start of Text
#define ByteETX             0x03 // Byte End of Text
#define ByteEOT             0x04 // Byte End of Transmission
#define ByteACK             0x06 // Byte Acknowledge
#define ByteNAK             0x15 // Byte Negative-Acknowledge

#define RetValidPack        0x00 // Return Valid Pack
#define RetInvalidTxPack    0x01 // Return Invalid TX Pack
#define RetInvalidRxPack    0x02 // Return Invalid RX Pack
#define RetTimeoutTX        0x03 // Return Timeout to TX
#define RetTimeoutRX        0x04 // Return Timeout to RX

#define MethodNone          0x00
#define MethodWriteOnly     0x01
#define MethodWriteResponse 0x02
#define MethodReadOnly      0x03
#define MethodReadResponse  0x04

#define B1200               1200
#define B2400               2400
#define B4800               4800
#define B9600               9600
#define B19200              19200
#define B38400              38400
#define B57600              57600
#define B115200             115200

enum SerialPortError {
   speNoError,
   speDeviceNotFoundError,
   speNotOpenError,
   speOpenError,
   spePermissionError,
   speReadError,
   speResourceError,
   speTimeoutError,
   speUnknownError,
   speUnsupportedOperationError,
   speWriteError
};

enum StateAnswer {
   saSYNC,
   saSOH,
   saAddressSource,
   saAddressDestination,
   saMethod,
   saEvent,
   saAnswer
};

enum StateReceiver {
   srSYNC,
   srSOH,
   srMethod,
   srAddressSource,
   srAddressDestination,
   srEvent,
   srLengthLo,
   srLengthHi,
   srSTX,
   srBuffer,
   srETX,
   srCrcLo,
   srCrcHi,
   srEOT,
   srINVALID
};

typedef struct {
   unsigned long long counterBytesRX;
   unsigned long long counterBytesTX;
   unsigned long long counterValidPackRX;
   unsigned long long counterValidPackTX;
   unsigned long long counterInvalidPackRX;
   unsigned long long counterInvalidPackTX;
   unsigned long long counterTimeoutRX;
   unsigned long long counterTimeoutTX;
} Status;

//-----------------------------------------------------------------------------

class Net485 : public QObject
{
   Q_OBJECT

public:
   explicit Net485(QObject *parent = 0);
   ~Net485();

   bool open(
      unsigned char address,
         const QString &port,
         qint32 baudRate,
         char *buffer,
         unsigned short bufferSize);

   void close();

   void setTimeoutRX(unsigned int timeoutRX);
   void setTimeoutTX(unsigned int timeoutTX);

   SerialPortError getLastError();

   void resetStatus();
   Status getStatus();

   unsigned char read(
     unsigned char *event,
     char *buffer,
     unsigned short *length);

   unsigned char readOnly(
     unsigned char *event,
     char *buffer,
     unsigned short *length);

   unsigned char write(
     unsigned char event,
     char *buffer,
     unsigned short length);

   unsigned char writeOnly(
     unsigned char event,
     char *buffer,
     unsigned short length);

   unsigned char write(
     unsigned char addrDestination,
     unsigned char event,
     char *buffer,
     unsigned short length);

   unsigned char writeOnly(
     unsigned char addrDestination,
     unsigned char event,
     char *buffer,
     unsigned short length);

private:
   QSerialPort serialPort;

   unsigned char address;
   unsigned short timeoutTX;
   unsigned short timeoutRX;

   SerialPortError lastError;

   Status status;

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

   unsigned short bufferSize;
   char *buffer;

   //---------------------------------

   unsigned char readRequest();
   unsigned char readResponse();

   //---------------------------------

   unsigned char writeRequest(
     unsigned char event,
     char *buffer,
     unsigned short length);

   unsigned char writeResponse();

private slots:
   void readyRead();
   void error(QSerialPort::SerialPortError error);

signals:
   void onReceive(
     unsigned char event,
     char *buffer,
     unsigned short length);

   void onReceiveBroadcast(
     unsigned char event,
     char *buffer,
     unsigned short length);

   void onError(SerialPortError error);

};

//-----------------------------------------------------------------------------

#endif // NET485_H
