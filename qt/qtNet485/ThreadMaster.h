#ifndef THREADMASTER_H
#define THREADMASTER_H

#include <QThread>

#include "Net485.h"

//-----------------------------------------------------------------------------

class ThreadMaster : public QThread
{
   Q_OBJECT

public:
   explicit ThreadMaster(QObject *parent = 0);

protected:
   void run();

private slots:
   void error(SerialPortError error);

};

//-----------------------------------------------------------------------------

#endif // THREADMASTER_H
