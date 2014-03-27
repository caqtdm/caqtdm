#ifndef EWIDGET_WITH_RCLICK_MENU
#define EWIDGET_WITH_RCLICK_MENU

class QMenu;

/** \brief interface that must be inherited by any widget that 
 *         wants to export a right click menu to the inheriting widgets.
 *
 * If your widget needs to export a right click menu, then it must export it through
 * this interface and implement the two pure virtual methods rightClickMenu() and
 * setRightClickMenu().
 */
class RightClickMenuable
{
  public:
	/** \brief returns the QMenu associated with the right click of the inheriting widget.
	 *
	 * This method is pure virtual and so needs to be implemented in your widget.
	 * Simply create a new QMenu or retrieve the existing one if you are already subclassing a 
	 * RightClickMenuable.
	 * Then you can modify the menu (if already existing) adding or removing actions from it.
	 * Finally do not forget to return the menu you want to be displayed when the user right clicks
	 * on your widget.
	 *
	 * An example can be found in class EPlotLight.
	 */
	virtual QMenu* rightClickMenu() = 0;

	/** \brief a method that is invoked in RightClickEvent just before exec() is called on the menu
	 *
	 * The default implementation is empty, but you can reimplement it in a subclass to provide 
	 * last minute checks and modifications for on menu about to be shown or in its actions.
	 * For instance, in your subclass you might want to hide or disable some actions which in a 
	 * particular moment are not available or pertinent.
	 */
	virtual void rightClickMenuAboutToShow() {};
};

#endif

