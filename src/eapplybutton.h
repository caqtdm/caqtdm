#ifndef EAPPLYBUTTON_H
#define EAPPLYBUTTON_H

#include <qtcontrols_global.h>
#include <econstants.h>
#include <QPushButton>

/**
 * \brief a button to apply a setting
 *
 * this is a simple button used to apply a setting, it has to slots 
 * that allows the developer to change the color of the displayed text:
 * valueModified sets the color to red and clearModified reverts to
 * the original setting
 */
class QTCON_EXPORT EApplyButton : public QPushButton
{
	Q_OBJECT
public:
	EApplyButton(QWidget *parent);
    ~EApplyButton(){}
	
	bool isModified() { return d_modified; }

public slots:
	/**
	 * tell the widget that the value to which it's associated
	 * has changed, so the color of the font changes to red
	 */
	void valueModified(double);
	/**
	 * resets font color to black
	 */
	void clearModified();

private:
	QColor defaultTextColor;
	bool d_modified;
};

#endif
