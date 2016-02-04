#include "bsread_wfhandling.h"
#include "bsread_wfconverter.h"

void bsread_wfhandling::wfconvert()
{
    switch (bsreadPVP->type){
        case bs_float64:{
            bsread_wfConverter<double,double> *converter=new bsread_wfConverter<double,double>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_float32:{
            bsread_wfConverter<float,double> *converter=new bsread_wfConverter<float,double>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_int64:{
            bsread_wfConverter<qint64,double> *converter=new bsread_wfConverter<qint64,double>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_int32:{
            bsread_wfConverter<qint32,long> *converter=new bsread_wfConverter<qint32,long>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_uint64:{
            bsread_wfConverter<quint64,double> *converter=new bsread_wfConverter<quint64,double>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_uint32:{
            bsread_wfConverter<quint32,double> *converter=new bsread_wfConverter<quint32,double>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_int16:{
            bsread_wfConverter<qint16,int> *converter=new bsread_wfConverter<qint16,int>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_int8:{
            bsread_wfConverter<qint8,int> *converter=new bsread_wfConverter<qint8,int>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_uint16:{
            bsread_wfConverter<quint16,long> *converter=new bsread_wfConverter<quint16,long>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_uint8:{
            bsread_wfConverter<quint8,int> *converter=new bsread_wfConverter<quint8,int>(kDataP,bsreadPVP);
            converter->wfconvert();
            delete converter;
            break;
        }
     }

}

bsread_wfhandling::bsread_wfhandling(knobData *kData, bsread_channeldata *bsreadPV)
{
    kDataP=kData;
    bsreadPVP=bsreadPV;
    this->setAutoDelete(true);
}

