#include <QSysInfo>
#include "bsread_wfhandling.h"
#include "bsread_wfconverter.h"

void bsread_wfhandling::process()
{
    wfconvert();
}

void bsread_wfhandling::wfconvert()
{
    switch (bsreadPVP->type){
        case bs_float64:{
            bsread_wfConverter<double,double> *converter=new bsread_wfConverter<double,double>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_float32:{
            bsread_wfConverter<float,double> *converter=new bsread_wfConverter<float,double>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_int64:{
            bsread_wfConverter<qint64,double> *converter=new bsread_wfConverter<qint64,double>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_int32:{
            bsread_wfConverter<qint32,long> *converter=new bsread_wfConverter<qint32,long>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_uint64:{
            bsread_wfConverter<quint64,double> *converter=new bsread_wfConverter<quint64,double>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_uint32:{
            bsread_wfConverter<quint32,double> *converter=new bsread_wfConverter<quint32,double>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_int16:{
            bsread_wfConverter<qint16,int> *converter=new bsread_wfConverter<qint16,int>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_int8:{
            bsread_wfConverter<qint8,int> *converter=new bsread_wfConverter<qint8,int>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_uint16:{
            //qDebug() << "<quint16,int>";
            bsread_wfConverter<quint16,unsigned short> *converter=new bsread_wfConverter<quint16,unsigned short>(kDataP,bsreadPVP,BlockPoolP);
            if ((bsreadPVP->endianess==bs_little)&&(QSysInfo::ByteOrder==QSysInfo::LittleEndian)){
                converter->usememcpy();
            }
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_uint8:{
            bsread_wfConverter<quint8,int> *converter=new bsread_wfConverter<quint8,int>(kDataP,bsreadPVP,BlockPoolP);
            converter->wfconvert();
            delete converter;
            break;
        }
        case bs_string:
        case bs_bool:{
          printf("bool and string not yet handled as waveform");
          break;
        }



     }

}

bsread_wfhandling::bsread_wfhandling(knobData *kData, bsread_channeldata *bsreadPV, QThreadPool *BlockPool)
{
    kDataP=kData;
    bsreadPVP=bsreadPV;
    BlockPoolP=BlockPool;
    this->setAutoDelete(true);
}

