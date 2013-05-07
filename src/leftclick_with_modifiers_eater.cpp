#include "leftclick_with_modifiers_eater.h"
#include <QMouseEvent>

bool LeftClickWithModifiersEater::eventFilter(QObject *obj, QEvent *event)
{
  if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
  {
	QMouseEvent *me = static_cast<QMouseEvent *>(event);
	if(me->button() == Qt::LeftButton && me->modifiers() != Qt::NoModifier)
	  return true;
  }

  return QObject::eventFilter(obj, event);
}

