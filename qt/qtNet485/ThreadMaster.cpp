#include "ThreadMaster.h"

#include <QDebug>
#include <QTime>
#include <QDateTime>

#define BufferSize 512

#define MaxNodes 6

typedef struct {
    unsigned char active;
    unsigned int packValid;
    unsigned int packInvalid;
} Node;

//-----------------------------------------------------------------------------

ThreadMaster::ThreadMaster(QObject *parent) : QThread(parent)
{

}

//-----------------------------------------------------------------------------

void ThreadMaster::run()
{
   Net485 net485;

   connect(&net485, SIGNAL(onError(SerialPortError)),
           this, SLOT(error(SerialPortError)));

   QString port = "/dev/ttyUSB0";

   unsigned char ret;
   unsigned char event;
   char bufferTX[BufferSize];
   char bufferRX[BufferSize];
   unsigned short length;

   QTime time;
   int counter_pps = 0;

   QDateTime base = QDateTime::currentDateTime();

   if ( net485.open(AddrMaster, port, 115200, bufferRX, BufferSize) ) {

      printf("Starting . . .\n");
      msleep(2000);
      printf("\nOK\n\n");

      time.start();

      unsigned char aux = 0;
      unsigned short size = 0;

      Node nodes[MaxNodes];
      memset(nodes, 0, sizeof(Node) * MaxNodes);

      nodes[0].active = false;
      nodes[1].active = true;
      nodes[2].active = true;
      nodes[3].active = true;
      nodes[4].active = true;
      nodes[5].active = true;

      net485.writeOnly(AddrBroadcast, 0x01, NULL, 0);

      forever
      {
         size = qrand() % 8;
         for ( int x=0; x < size; x++ ) {
            bufferTX[x] = qrand() % 255;
         }

         //-------------------------

         for ( unsigned char x=0; x < MaxNodes; x++ )
         {
            unsigned char address = x + 1; 

            if ( ! nodes[x].active ) continue;

            // Apenas escrita
            ret = net485.writeOnly(address, 0x00, bufferTX, size);
            if ( ret == RetValidPack ) {

               // Escrita e leitura sem confirmacao
               ret = net485.writeOnly(address, 0x03, bufferTX, size);
               if ( ret == RetValidPack ) {
                  ret = net485.readOnly(&event, bufferRX, &length);
                  if ( ret == RetValidPack ) {

                     // Escrita e leitura com confirmacao
                     ret = net485.write(address, 0x02, bufferTX, size);
                     if ( ret == RetValidPack ) {
                        ret = net485.read(&event, bufferRX, &length);
                        if ( ret == RetValidPack ) {

                           // Escrita em broadcast sem confirmacao
                           memset(bufferTX, aux + 1, size);
                           ret = net485.write(AddrBroadcast, 0x01, bufferTX, size);
                           if ( ret == RetValidPack ) {

                              counter_pps += 6;

                              nodes[x].packValid += 6;
                              continue;

                           }
                        }
                     }
                  }
               }
            }

            nodes[x].packInvalid++;
         }

         //-------------------------

         Status status = net485.getStatus();

         qint64 msecs = base.msecsTo(QDateTime::currentDateTime());
         int seconds = msecs / 1000;
         int minutes = seconds / 60;
         int hours = minutes / 60;
         int days = hours / 24;
         seconds %= 60;
         minutes %= 60;
         hours %= 24;

         if ( time.elapsed() > 1000 ) {

            printf("\033[2J\033[H");
            printf("USB to RS485 on \x1b[33m/dev/ttyUSB0\x1b[0m\n");
            printf("\x1b[36mNet485\x1b[0m Protocol, Rev: 2016.10.10\n\n");

            printf("PPS: %4d    IO:%10.3f MB    Elapsed: %d days %02d:%02d:%02d    Rand: %4d\n\n",
                   counter_pps,
                   (status.counterBytesRX + status.counterBytesTX) / 1024.0  / 1024.0,
                   days, hours, minutes, seconds,
                   size);

            for ( unsigned char x=0; x < MaxNodes; x++ ) {
               unsigned char address = x + 1;
               if ( nodes[x].active ) {
                  printf("Node%02d, ok:%8d, error:%8d\n",
                     address,
                     nodes[x].packValid,
                     nodes[x].packInvalid);
               } else {
                  printf("Node%02d, not connected . . .\n",
                     address);

               }
            }

            counter_pps = 0;
            time.restart();
         }
      }
   }

   qDebug() << "ThreadMaster # Error Port" << port;
}

//-----------------------------------------------------------------------------

void ThreadMaster::error(SerialPortError error)
{
   switch ( error )
   {
      case speNoError                    : { qDebug() << "ThreadMaster::error speNoError"; break; }
      case speDeviceNotFoundError        : { qDebug() << "ThreadMaster::error speDeviceNotFoundError"; break; }
      case speNotOpenError               : { qDebug() << "ThreadMaster::error speNotOpenError"; break; }
      case speOpenError                  : { qDebug() << "ThreadMaster::error speOpenError"; break; }
      case spePermissionError            : { qDebug() << "ThreadMaster::error spePermissionError"; break; }
      case speReadError                  : { qDebug() << "ThreadMaster::error speReadError"; break; }
      case speResourceError              : { qDebug() << "ThreadMaster::error speResourceError"; break; }
      case speTimeoutError               : { qDebug() << "ThreadMaster::error speTimeoutError"; break; }
      case speUnknownError               : { qDebug() << "ThreadMaster::error speUnknownError"; break; }
      case speUnsupportedOperationError  : { qDebug() << "ThreadMaster::error speUnsupportedOperationError"; break; }
      case speWriteError                 : { qDebug() << "ThreadMaster::error speWriteError"; break; }
   }
}

//-----------------------------------------------------------------------------
