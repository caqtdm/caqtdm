#include "fastuiloader.h"

#include <sys/timeb.h>
#include <sys/time.h>

static double rTime()
{
    struct timeval tt;
    gettimeofday(&tt, (struct timezone *) 0);
    return (double) 1000000.0 * (double) tt.tv_sec + (double) tt.tv_usec;
}

UiLoader::UiLoader(QObject *parent) : QUiLoader(parent)
{
    qDebug() << "Uiloader class create";
    map.clear();
}

QWidget* UiLoader::createWidget(const QString &className, QWidget *parent, const   QString &name)
{
   int occurrence;
    QMap<QString, int>::iterator i = statisticList.find(className);
    bool found = false;
    // when key found increment its value
    while (i != statisticList.end() && i.key() ==  className) {
        found = true;
        occurrence = i.value()+1;
        statisticList.insert(className, occurrence);
        break;
    }
    if(!found) {
        // when not found, insert in statisticlist
        statisticList.insert(className, 1);
        occurrence = 1;
    }

    double last = rTime();
    QWidget* widget = QUiLoader::createWidget(className, parent, name);
    double now = rTime();
    if(qRound ((now - last) /1000.0) > 0) qDebug() << "loaded" << className << " in " << qRound ((now - last) /1000.0) << " ms";
    return widget;
}

QWidget* UiLoader::fastload(QString fileName, QWidget *parentWidget)
{
    //qDebug() << "load file" << fileName;
    QWidget *w;

    QMap<QString, QByteArray *>::const_iterator i = map.find(fileName);
    while (i != map.end() && i.key() == fileName) {
        QBuffer buffer(i.value());
        //qDebug() << "got buffer from Qmap for " << fileName;
        w = load(&buffer, parentWidget);
        buffer.deleteLater();
        return w;
    }

    QFile *file = new QFile;
    QByteArray *byteArray = new QByteArray;
    QBuffer buffer(byteArray);
    file->setFileName(fileName);
    file->open(QIODevice::ReadOnly);

    *byteArray = file->readAll();

    map.insert(fileName, byteArray);

    w =  load(&buffer, parentWidget);

    buffer.deleteLater();
    file->close();
    delete file;
    return w;
}

void UiLoader::cleanup()
{
    QMap<QString, QByteArray *>::const_iterator i = map.constBegin();
    while (i != map.constEnd()) {
        QByteArray *byteArray = i.value();
        byteArray->clear();
        ++i;
    }
    map.clear();

    QMap<QString, int>::const_iterator j = statisticList.constBegin();
    while (j !=statisticList.end()) {
        qDebug() <<  j.key() <<  j.value();
         ++j;
    }

}
