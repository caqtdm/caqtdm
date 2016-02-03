#ifndef BSREAD_WFCONVERTER_H
#define BSREAD_WFCONVERTER_H

#include <QAtomicInt>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>

#include "knobData.h"
#include "mutexKnobData.h"
#include "bsread_channeldata.h"

template <class T_BSREAD,class T_CAQTDM>
class bsread_wfConverter
{
private:
    knobData* kDataP;
    bsread_channeldata * bsreadPVP;
public:
    bsread_wfConverter(knobData* kData,bsread_channeldata * bsreadPV)
    {
        kDataP=kData;
        bsreadPVP=bsreadPV;
    }
    void wfconvert()
    {
        if (kDataP->edata.dataSize/sizeof(T_CAQTDM)!=bsreadPVP->bsdata.wf_data_size/sizeof(T_BSREAD)){
            QMutex *datamutex;
            datamutex = (QMutex*) kDataP->mutex;
            datamutex->lock();
            if (kDataP->edata.dataB==NULL){
                free(kDataP->edata.dataB);
            }

            kDataP->edata.dataB=malloc((bsreadPVP->bsdata.wf_data_size*sizeof(T_BSREAD))/sizeof(T_CAQTDM));
            kDataP->edata.dataSize=(bsreadPVP->bsdata.wf_data_size*sizeof(T_BSREAD))/sizeof(T_CAQTDM);
            datamutex->unlock();
        }

        QByteArray data = QByteArray::fromRawData((const char *)bsreadPVP->bsdata.wf_data, bsreadPVP->bsdata.wf_data_size*sizeof(T_BSREAD));
        QDataStream stream(data);
        QVector<T_CAQTDM> data_stream(bsreadPVP->bsdata.wf_data_size);
        stream.setByteOrder(QDataStream::LittleEndian);
        long counter=0;
        while ( !stream.atEnd() ) {
             T_BSREAD datatemp;
             stream >> datatemp;
             data_stream.replace(counter,datatemp);
             counter++;
        }


        memcpy(kDataP->edata.dataB,data_stream.data(),data_stream.count()*sizeof(T_CAQTDM));
        kDataP->edata.valueCount=data_stream.count()-100;
        qDebug() << "counts :" << bsreadPVP->bsdata.wf_data_size << sizeof(T_BSREAD)<< sizeof(T_CAQTDM) << data_stream.count() << kDataP->edata.dataSize ;
    }

};







#endif // BSREAD_WFCONVERTER_H

