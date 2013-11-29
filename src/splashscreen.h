#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H


#include <QSplashScreen>
#include <QApplication>

class SplashScreen : public QSplashScreen
{
    Q_OBJECT
public:
    explicit SplashScreen( QWidget *parent = 0);
    int m_progress;
    void setMaximum(int max);


public slots:
    void setProgress(int value)
    {
      m_progress = value;
      if (m_progress > m_maximum) m_progress = m_maximum;
      if (m_progress < 0)         m_progress = 0;
      update();
      QApplication::processEvents();
    }

protected:
    void drawContents(QPainter *painter);

private:
        QPixmap pixmap;
        int m_maximum;

};

#endif
