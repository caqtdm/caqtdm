#include "bsread_channeldata.h"

bsread_channeldata::bsread_channeldata(QObject *parent) : QObject(parent)
{
    name="";
    type=bs_double;
    shape.append(1);
    offset=0;
    modulo=1;
    endianess=bs_little;

}

