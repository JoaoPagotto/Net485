#include <QCoreApplication>

#include "ThreadMaster.h"

//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   
   printf("- ThreadMaster (start)\n");
   ThreadMaster threadMaster;
   threadMaster.start();

   return a.exec();
}

//-----------------------------------------------------------------------------
