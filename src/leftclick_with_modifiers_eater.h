#ifndef LEFT_CLICK_WITH_MODIFIERS_EATER_H
#define LEFT_CLICK_WITH_MODIFIERS_EATER_H

#include <QObject>

class LeftClickWithModifiersEater : public QObject
{
  Q_OBJECT
  public:
	LeftClickWithModifiersEater(QObject *parent) : QObject(parent) {};
	
  protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);
};

#endif
