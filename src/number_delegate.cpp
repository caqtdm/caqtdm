#include <QObject>
#include <math.h>
#include "number_delegate.h"
#include "dec_int_from_format.h"
#include <stdio.h>

#define qstoc(x) ""

NumberDelegate::NumberDelegate()
{
    d_intDigits = 3;
    d_decDigits = 2;
}

bool NumberDelegate::configureNumber(const QString& format)
{
    bool decodeOk;
    /* first apply format, if - correctly - specified */
    DecIntFromFormat diff(format);
    decodeOk = diff.decode();
    if(decodeOk)
    {
        if(!diff.integerDefaults()) /* found a configuration format, not the default coming from DecIntFromFormat */
            d_intDigits = diff.numIntDigits();
        if(!diff.decimalDefaults())
            d_decDigits = diff.numDecDigits();
    }
    else
        printf("NumberDelegate: error decoding format \"%s\"", qstoc(format));
    return decodeOk;
}

void NumberDelegate::configureNumber(const double min, const double max, const QString& format)
{
    int maxFromId = 3;
    //bool decodeOk = configureNumber(format);
    configureNumber(format);

    /* integer digits is overwritten if there is a maximum and minimum */
    maxFromId = 1 + (int) log10(qMax(max, -min));
    if(maxFromId != d_intDigits)
    {
        printf("maximum value %f has changed integer digits from %d (format) to %d",
               max, d_intDigits, maxFromId);
        d_intDigits = maxFromId;
    }
    else
        printf("NumberDelegate configureNumber: integer digits unchanged");


}

