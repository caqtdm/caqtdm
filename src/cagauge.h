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

#ifndef _CAGAUGE_H
#define _CAGAUGE_H
#include <qtcontrols_global.h>
#include <egauge.h>

class QTCON_EXPORT caAbstractGauge : public EAbstractGauge
{

 Q_PROPERTY(QString channel READ getPV WRITE setPV)
 Q_OBJECT

public:

        caAbstractGauge(QWidget * = NULL);
        ~caAbstractGauge(){}

        QString getPV() const;
        void setPV(QString const &newPV);
	
  private:

         QString thisPV;
};

/**
 * \brief a thermometer like widget
 */
class QTCON_EXPORT caLinearGauge : public caAbstractGauge
{
Q_OBJECT

Q_ENUMS(FillMode)
	
Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode)

public:
	/**
	 * How the value is displayed on the bar
	 */
	enum FillMode
	{
		ALL, /** fill the whole bar, moving marker */
		FROM_MIN, /** bar filled from min value to value */
		FROM_ZERO, /** bar filled from zero to value */
		FROM_REF /** bar filled from reference to value */
	};
	
        caLinearGauge(QWidget * = NULL, Qt::Orientation=Qt::Vertical);
        ~caLinearGauge(){}

        void setOrientation(Qt::Orientation o){ m_orientation = o; configure(); update(); }
        Qt::Orientation orientation(){ return m_orientation; }

        void setFillMode(FillMode m){ m_fillMode = m; configure(); update(); }
        FillMode fillMode(){ return m_fillMode; }

protected:
	void paintEvent(QPaintEvent *);
	void drawColorBar(QPainter *);
	void drawScale(QPainter *);
	void drawMarker(QPainter *, bool);
	void drawLabels(QPainter *);
	virtual void configure();

private:
	Qt::Orientation	m_orientation;
	FillMode m_fillMode;
	int scalePos, labelsPos, totalSize;
	int barSize, scaleSize, labelsSize;
};

/**
 * \brief a tachometer like widget
 * 
 * A tachometer like widget.
 */
class QTCON_EXPORT caCircularGauge : public caAbstractGauge
{
Q_OBJECT

Q_PROPERTY(bool valueDisplayed READ isValueDisplayed WRITE setValueDisplayed)
Q_PROPERTY(QString label READ label WRITE setLabel)

public:
        caCircularGauge(QWidget * = NULL);
        ~caCircularGauge(){}

    void setValueDisplayed(bool b){ m_valueDisplayed = b; update(); }
    bool isValueDisplayed(){ return m_valueDisplayed; }

    void setLabel(QString l){ m_label = l; update(); }
    QString label(){ return m_label; }
	
protected:
	void paintEvent(QPaintEvent *);
	void drawColorBar(QPainter *);
	void drawScale(QPainter *);
	void drawNeedle(QPainter *);
	void drawLabels(QPainter *);
	void drawValue(QPainter *);
	virtual void configure();
	
private:
	double m_startAngle, m_stopAngle, m_arcLength;
	int m_innerRadius, m_outerRadius, m_labelRadius;
	QString m_label;
	bool m_valueDisplayed;
};
#endif
