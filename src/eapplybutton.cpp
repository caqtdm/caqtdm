#include "eapplybutton.h"
#include "leftclick_with_modifiers_eater.h"

EApplyButton::EApplyButton(QWidget *parent) : QPushButton(parent)
{
	this->setFocusPolicy(Qt::StrongFocus);
	this->setText("APPLY");
	d_modified = false;
	setMinimumWidth(MIN_BUTTON_SIZE);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	defaultTextColor = palette().color(QPalette::ButtonText);	
	connect(this, SIGNAL(clicked()), this, SLOT(clearModified()), Qt::QueuedConnection);
	defaultTextColor = palette().color(QPalette::ButtonText);
	LeftClickWithModifiersEater *leftClickWithModifiersEater = new LeftClickWithModifiersEater(this);
	installEventFilter(leftClickWithModifiersEater);
}

void EApplyButton::valueModified(double)
{
	if (isEnabled())
	{
		QPalette p = palette();
		p.setColor(QPalette::ButtonText, Qt::red);
		setPalette(p);
		d_modified = true;
	}
}

void EApplyButton::clearModified()
{
	if (isEnabled())
	{
		QPalette p = palette();
		p.setColor(QPalette::ButtonText, defaultTextColor);
		setPalette(p);
	}
	d_modified = false;
}
