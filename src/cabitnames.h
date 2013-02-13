//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#ifndef caBitnames_H
#define caBitnames_H

#include <qtcontrols_global.h>
#include <eflag.h>

class QTCON_EXPORT caBitnames : public EFlag
{
    Q_OBJECT

    Q_ENUMS(Direction)

    Q_PROPERTY(QString channelEnum READ getEnumPV WRITE setEnumPV)
    Q_PROPERTY(QString channelValue READ getValuePV WRITE setValuePV)

    Q_PROPERTY(int startBit READ getStartBit WRITE setStartBit)
    Q_PROPERTY(int endBit   READ getEndBit   WRITE setEndBit)

    Q_PROPERTY(QColor foreground   READ getTrueColor     WRITE setTrueColor)
    Q_PROPERTY(QColor background  READ getFalseColor    WRITE setFalseColor)
    Q_PROPERTY(EFlag::alignmentHor alignment READ getAlignment WRITE setAlignment)
    Q_PROPERTY(ESimpleLabel::ScaleMode fontScaleMode READ fontScaleMode WRITE setFontScaleMode)


    Q_PROPERTY(int numRows READ readNumRows WRITE setNumRows DESIGNABLE false)
    Q_PROPERTY(int numColumns READ readNumColumns WRITE setNumColumns DESIGNABLE false)
    Q_PROPERTY(QString trueColors   READ trueColors     WRITE setTrueColors   DESIGNABLE false)
    Q_PROPERTY(QString falseColors  READ falseColors    WRITE setFalseColors  DESIGNABLE false)
    Q_PROPERTY(QString trueStrings  READ trueStrings    WRITE setTrueStrings  DESIGNABLE false)
    Q_PROPERTY(QString falseStrings READ falseStrings   WRITE setFalseStrings DESIGNABLE false)
    Q_PROPERTY(QString displayMask  READ getDisplayMask WRITE setDisplayMask  DESIGNABLE false)

public:

    caBitnames(QWidget *parent);

    QString getEnumPV() const;
    void setEnumPV(QString const &newPV);
    QString getValuePV() const;
    void setValuePV(QString const &newPV);

    void setEnumDisplay(int cell, unsigned int value, QString corresponding_str, QColor corresponding_color);

    void setTrueColor(QColor c);
    QColor getTrueColor() const {return thisTrueColor;}

    void setFalseColor(QColor c);
    QColor getFalseColor() const {return thisFalseColor;}

    void setValue(int value);

    int getStartBit()  const {return thisStartBit;}
    void setStartBit(int const &bit);

    int getEndBit()  const {return thisEndBit;}
    void setEndBit(int const &bit);

    bool bitState(long value, int bitNr);
    void setEnumStrings(QString string);
    void setStrings(QString string);

    void setFontScaleMode(ESimpleLabel::ScaleMode m);
    ESimpleLabel::ScaleMode fontScaleMode();

    EFlag::alignmentHor getAlignment() const {return thisAlignment;}
    void setAlignment(EFlag::alignmentHor alignment);

protected:

    virtual void fontChange(const QFont & oldFont);

private:
    int	            numRows;
    int 	        numColumns;

    QColor		    thisFalseColor;
    QColor		    thisTrueColor;
    int             thisStartBit;
    int             thisEndBit;
    QString         thisEnumPV, thisValuePV;
    QString         thisString;

    QList<QVariant> tf;
    QStringList stringlist;
    EFlag::alignmentHor    thisAlignment;
};

#endif  /* caBitnames */
