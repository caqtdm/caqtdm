#ifndef BSREAD_WFBLOCKCONVERTER
#define BSREAD_WFBLOCKCONVERTER
#include <QThread>
#include <QDebug>
#include <QRunnable>
#include <QSysInfo>

template <class T_BSREAD,class T_CAQTDM>
class bsread_wfblockconverter :public QObject, public QRunnable
{
    void run()
     {
        Process();
     }
private:
    int sectorP,fullP;
    T_BSREAD* SourceP;
    size_t sourcecountP;
    T_CAQTDM * targetP;
    bsread_endian EndianessP;

public:
    bsread_wfblockconverter(int sector,int full,T_BSREAD* Source,size_t sourcecount ,T_CAQTDM * target,bsread_endian Endianess){
      sectorP=sector;
      fullP=full;
      SourceP=Source;
      sourcecountP=sourcecount;
      targetP=target;
      EndianessP=Endianess;
    }
    void Process(int dummy){
        //QElapsedTimer timer;
        //timer.start();
        Q_UNUSED(dummy);
        QByteArray data = QByteArray::fromRawData((const char *)SourceP,(int) sourcecountP*sizeof(T_BSREAD));
        QDataStream stream(data);
        stream.skipRawData((int)(sectorP*(sourcecountP*sizeof(T_BSREAD)/fullP)));
        switch(EndianessP){
           case bs_big : stream.setByteOrder(QDataStream::BigEndian);
           default     : stream.setByteOrder(QDataStream::LittleEndian);
        }
        size_t counter=sectorP*sourcecountP/fullP;
        size_t counterEnd=counter+sourcecountP/fullP;
        while ( !stream.atEnd()&& (counter<counterEnd) ) {
             T_BSREAD datatemp;
             stream >> datatemp;
             (targetP)[counter]=(T_CAQTDM)datatemp;
             counter++;
        }
        //qDebug() <<"Sec2:" << sectorP <<  "convert timer :" <<  timer.elapsed() << "milliseconds";

    }
};
#endif // BSREAD_WFBLOCKCONVERTER

