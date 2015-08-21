#ifndef BSREAD_CHANNELDATA_H
#define BSREAD_CHANNELDATA_H

#include <QObject>
#include <QList>


enum bsread_types{
    bs_double,bs_string,bs_integer,bs_long,bs_short
};

enum bsread_endian{
    bs_little,bs_big
};
typedef struct _bs_data{
   double bs_double;
   QString bs_string;
   int bs_integer;
   long bs_long;
   short bs_short;
}bs_data;

class bsread_channeldata : public QObject
{
    Q_OBJECT
public:
    explicit bsread_channeldata(QObject *parent = 0);
    QString name;
    bsread_types type;
    QList<int> shape;
    int offset;
    int modulo;
    bsread_endian endianess;
    double timestamp;
    bs_data bsdata;
signals:

public slots:
};

#endif // BSREAD_CHANNELDATA_H
