#ifndef ImageWIDGET_H
#define ImageWIDGET_H

#include <QPixmap>
#include <QWidget>
#include <QGridLayout>
#include <caLineEdit>

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    ImageWidget(QWidget *parent = 0);
    ~ImageWidget(){}
    void updateImage(bool zoom, const QImage &image, bool valuesPresent[], int values[]);
    QImage scaleImage(const QImage &image);

protected:
    void paintEvent(QPaintEvent *event);

private slots:

private:

    QPixmap pixmap;
    QPoint pixmapOffset;
    bool m_zoom;
    QGridLayout  *grid;
    caLineEdit *labelMin;
    caLineEdit *labelMax;
    QWidget *imageW;
    bool drawValues[4];
    int geoValues[4];
};

#endif
