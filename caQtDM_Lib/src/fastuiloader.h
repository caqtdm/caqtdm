#include <QUiLoader>
#include <QIODevice>
#include <QWidget>
#include <QFile>
#include <QBuffer>
#include <QDebug>

class UiLoader : public QUiLoader
{
    Q_OBJECT
    
public:
    UiLoader(QObject *parent = 0);
    QWidget* fastload(QString fileName, QWidget *parentWidget = 0);
    void cleanup();

protected:
virtual QWidget* createWidget(const QString &className, QWidget *parent =0, const   QString &name = QString());

private:
    QMap<QString, QByteArray *> map;
    QMap<QString, int> statisticList;
};
