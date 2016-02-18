#ifndef BSREAD_WFCONVERTER_H
#define BSREAD_WFCONVERTER_H
#include <QtCore>
#include <QThread>

#include <QAtomicInt>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QElapsedTimer>
#include <qdatastream.h>

#include "knobData.h"
#include "mutexKnobData.h"
#include "bsread_channeldata.h"
#include "bsread_wfblockconverter.h"

template <class T_BSREAD,class T_CAQTDM>
class bsread_wfConverter
{
private:
    knobData* kDataP;
    bsread_channeldata * bsreadPVP;
    QThreadPool *BlockPoolP;
public:
    bsread_wfConverter(knobData* kData,bsread_channeldata * bsreadPV,QThreadPool *BlockPool)
    {
        kDataP=kData;
        bsreadPVP=bsreadPV;
        BlockPoolP=BlockPool;
    }

    void wfconvert()
    {


        QElapsedTimer timer;
        timer.start();


        if (kDataP->edata.valueCount!=bsreadPVP->bsdata.wf_data_size){
            QMutex *datamutex;
            datamutex = (QMutex*) kDataP->mutex;
            datamutex->lock();
            if (kDataP->edata.dataB==NULL){
                qDebug() << "Realloc";
                free(kDataP->edata.dataB);
            }

            kDataP->edata.dataB=malloc(bsreadPVP->bsdata.wf_data_size*sizeof(T_CAQTDM));
            kDataP->edata.dataSize=bsreadPVP->bsdata.wf_data_size*sizeof(T_CAQTDM);
            datamutex->unlock();
        }
        kDataP->edata.valueCount=bsreadPVP->bsdata.wf_data_size;
        if (kDataP->edata.valueCount<100000){
            QByteArray data = QByteArray::fromRawData((const char *)bsreadPVP->bsdata.wf_data, bsreadPVP->bsdata.wf_data_size*sizeof(T_BSREAD));
            QDataStream stream(data);
            ulong counter=0;

            switch(bsreadPVP->endianess){
               case bs_big : stream.setByteOrder(QDataStream::BigEndian);
               default     : stream.setByteOrder(QDataStream::LittleEndian);
            }

            while ( !stream.atEnd() ) {
                 T_BSREAD datatemp;
                 stream >> datatemp;

                 ((T_CAQTDM*)kDataP->edata.dataB)[counter]=(T_CAQTDM)datatemp;
                 counter++;
            }
        }else{
            int threadcounter=QThread::idealThreadCount();
            size_t elementcount= (bsreadPVP->bsdata.wf_data_size);
            for (int sector=0;sector<threadcounter;sector++){
              T_BSREAD* ptr=(T_BSREAD *)(bsreadPVP->bsdata.wf_data);
              T_CAQTDM* target=(T_CAQTDM*)(kDataP->edata.dataB);
              bsread_wfblockconverter<T_BSREAD,T_CAQTDM> *blockconverter=new bsread_wfblockconverter<T_BSREAD,T_CAQTDM>(sector,threadcounter,
                                                                                                                        ptr
                                                                                                                   ,elementcount
                                                                                                                 ,target,bsreadPVP->endianess);
              blockconverter->setAutoDelete(true);
              BlockPoolP->start(blockconverter);
            }
            BlockPoolP->waitForDone();

        }

        qDebug() << "convert timer :" <<  timer.elapsed() << "milliseconds";

      }

};






#endif // BSREAD_WFCONVERTER_H

