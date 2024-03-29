#ifndef BSREAD_WFCONVERTER_H
#define BSREAD_WFCONVERTER_H
#include <QtCore>
#include <QThread>
#include <QThreadPool>

#include <QAtomicInt>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>

#if QT_VERSION > QT_VERSION_CHECK(4, 8, 0)
 #include <QElapsedTimer>
#endif

#include <qdatastream.h>

#include "knobData.h"
#include "mutexKnobData.h"
#include "bsread_channeldata.h"
#include "bsread_wfblockconverter.h"

#ifndef QT_NO_CONCURRENT
#include <QtConcurrentRun>
#include <QFutureSynchronizer>
#endif



template <class T_BSREAD,class T_CAQTDM>
class bsread_wfConverter
{
private:
    knobData* kDataP;
    bsread_channeldata * bsreadPVP;
    QThreadPool *BlockPoolP;
    bool usememcpyP;
    bool set_Precision;
    QDataStream::FloatingPointPrecision precision;
public:
    bsread_wfConverter(knobData* kData,bsread_channeldata * bsreadPV,QThreadPool *BlockPool)
    {
        kDataP=kData;
        bsreadPVP=bsreadPV;
        BlockPoolP=BlockPool;
        usememcpyP=false;
        set_Precision=false;
        precision=QDataStream::SinglePrecision;
    }
    void usememcpy(){
      usememcpyP=true;
    }
    void setPrecision(QDataStream::FloatingPointPrecision prec){
      set_Precision=true;
      precision=prec;
    }

    void ConProcess(int sectorP,int fullP,T_BSREAD* SourceP,size_t sourcecountP ,T_CAQTDM * targetP){
        //QElapsedTimer timer;
        //timer.start();

        QByteArray data = QByteArray::fromRawData((const char *)SourceP,(int) sourcecountP*sizeof(T_BSREAD));
        QDataStream stream(data);
        if (set_Precision) stream.setFloatingPointPrecision(precision);

        stream.skipRawData((int)(sectorP*(sourcecountP*sizeof(T_BSREAD)/fullP)));
        switch(bsreadPVP->endianess){
           case bs_big : {
            stream.setByteOrder(QDataStream::BigEndian);
            break;
        }
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


    void wfconvert()
    {


        //QElapsedTimer timer;
        //timer.start();

        if (bsreadPVP->valid){
        if ((ulong)kDataP->edata.valueCount!=bsreadPVP->bsdata.wf_data_size){
            QMutex *datamutex;
            datamutex = (QMutex*) kDataP->mutex;
            datamutex->lock();
            if (kDataP->edata.dataB==Q_NULLPTR){
                qDebug() << "Realloc"<< bsreadPVP->name << bsreadPVP->bsdata.wf_data_size<< sizeof(T_CAQTDM);
                free(kDataP->edata.dataB);
            }

            kDataP->edata.dataB=malloc(bsreadPVP->bsdata.wf_data_size*sizeof(T_CAQTDM));
            kDataP->edata.dataSize=bsreadPVP->bsdata.wf_data_size*sizeof(T_CAQTDM);
            datamutex->unlock();
        }
        kDataP->edata.valueCount=bsreadPVP->bsdata.wf_data_size;

        if (usememcpyP){
            memcpy(kDataP->edata.dataB,(const char *)bsreadPVP->bsdata.wf_data,kDataP->edata.dataSize);
        }else{
            if (kDataP->edata.valueCount<100000){
                QByteArray data = QByteArray::fromRawData((const char *)bsreadPVP->bsdata.wf_data, bsreadPVP->bsdata.wf_data_size*sizeof(T_BSREAD));
                QDataStream stream(data);
                if (set_Precision) stream.setFloatingPointPrecision(precision);
                ulong counter=0;
                switch(bsreadPVP->endianess){
                   case bs_big : {
                    stream.setByteOrder(QDataStream::BigEndian);
                    break;
                }
                   default     : stream.setByteOrder(QDataStream::LittleEndian);
                }

                while ( !stream.atEnd() ) {
                     T_BSREAD datatemp;
                     stream >> datatemp;

                     ((T_CAQTDM*)kDataP->edata.dataB)[counter]=(T_CAQTDM)datatemp;
                     counter++;
                }
            }else{
                size_t elementcount= (bsreadPVP->bsdata.wf_data_size);
                T_BSREAD* ptr=(T_BSREAD *)(bsreadPVP->bsdata.wf_data);
                T_CAQTDM* target=(T_CAQTDM*)(kDataP->edata.dataB);

   #ifndef QT_NO_CONCURRENT
                int threadcounter=QThread::idealThreadCount();

                QFutureSynchronizer<void> Sectors;
                for (int sector=0;sector<threadcounter;sector++){

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                    Sectors.addFuture(QtConcurrent::run(this,&bsread_wfConverter::ConProcess,sector,threadcounter,ptr,elementcount,target));
#else
                    Sectors.addFuture(QtConcurrent::run(&bsread_wfConverter::ConProcess,this,sector,threadcounter,ptr,elementcount,target));
#endif



                }
                Sectors.waitForFinished();
                //printf("Image timer : %d milliseconds \n",timer.elapsed());


    #else
                ConProcess(1,1,ptr,elementcount,target);
    #endif



            }

            //qDebug() << "convert timer :" <<  timer.elapsed() << "milliseconds";

          }
      }
    }
};






#endif // BSREAD_WFCONVERTER_H

