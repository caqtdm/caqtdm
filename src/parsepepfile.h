#ifndef PARSEPEPFILE_H
#define PARSEPEPFILE_H

#include <qtcontrols_global.h>
#include <QMap>
#include <QBuffer>
#include <QDataStream>
#include <QSharedMemory>
#include <QWidget>
#include <QtUiTools/QUiLoader>


class QTCON_EXPORT ParsePepFile
{

public:
    ParsePepFile(QString filename);
    void Initialize();
    QWidget *load(QWidget *parent);

protected:


private slots:

private:

    void displayItem(int row, int column, QString widgetType, QString text, QString pv, int span, QString *formats,
                     QByteArray *array);
    void writeOpenProperty(QString property, QByteArray *array);
    void writeCloseProperty(QByteArray *array);
    void writeTaggedString(QString tag, QString value, QByteArray *array);
    void setColor(QString property, int r, int g, int b, int alpha, QByteArray *array);
    void writeOpenTag(QString tag,  QByteArray *array);
    void writeCloseTag(QString tag,  QByteArray *array);
    void writeSimpleProperty(QString prop, QString tag, QString value, QByteArray *array);

    void writeLayoutHeader(QByteArray *array);
    void writeLayoutFooter(QByteArray *array);
    void writeLineEdit(QString format, QString text, QString minsize[2], QString maxsize[2], QString pv, QString pointsize,
                       QString alignment, QString colormode, QString calc, QString visibility, int rgba[4], QByteArray *array);
    void writeLabel(QString text, QString minsize[2], QString maxsize[2], QString pointsize, QString alignment,QString colormode,
                    QString calcpv, QString calc, QString visibility, int rgba[], QByteArray *array);
    //void writeLabel(QString text, QByteArray *array);
    void writeWheelswitch(QString format, QString pv, QByteArray *array);
    void writeLineEdit(QString format, QString pv, QByteArray *array);

    enum StringValue {evChannel, evComment, evSeparator, evEnd};
    QMap<QString, StringValue> mapStringValues;
    QSharedMemory sharedMemory;
    QBuffer *buffer;
};

#endif
