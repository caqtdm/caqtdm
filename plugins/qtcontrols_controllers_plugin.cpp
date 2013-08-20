#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <QtControls>
#include <qtcontrols_controllers_plugin.h>
#include <qglobal.h>
#include <QtPlugin>

typedef char strng[40];

char *XmlFunc(const char *clss, const char *name, int x, int y, int w, int h, strng propertyname[], strng propertytype[], int nb)
{
  char mess[1024], *xml;
        sprintf(mess, "<ui language=\"c++\">\
        <widget class=\"%s\" name=\"%s\">\
        <property name=\"geometry\">\
          <rect>\
            <x>%d</x>\
            <y>%d</y>\
            <width>%d</width>\
            <height>%d</height>\
          </rect>\
        </property>\
        </widget>",  clss, name, x, y, w, h);
        if(nb > 0) {
            sprintf(mess, "%s  <customwidgets>\
          <customwidget>\
             <class>%s</class>\
             <propertyspecifications>", mess, clss);
             for(int i=0; i<nb; i++) {
                sprintf(mess, "%s <stringpropertyspecification name=\"%s\" notr=\"true\" type=\"%s\"/>", 
                    mess, propertyname[i], propertytype[i]);
             }
             strcat(mess, " </propertyspecifications>\
                </customwidget>\
                </customwidgets>");
        }
        strcat(mess, "</ui>");
        xml = (char*) malloc(strlen(mess) * sizeof(char)+1);
        memcpy(xml, mess, strlen(mess) * sizeof(char)+1);
        //printf("%s\n", xml);
   return xml;
}

CustomWidgetInterface::CustomWidgetInterface(QObject *parent): QObject(parent), d_isInitialized(false)
{
}

void CustomWidgetInterface::initialize(QDesignerFormEditorInterface *)
{
    if (d_isInitialized)
        return;

    d_isInitialized = true;
}


caNumericInterface::caNumericInterface(QObject *parent): CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caNumeric", "canumeric", 0, 0, 100, 50, name, type, 1);
    d_name = "caNumeric";
    d_include = "caNumeric";
    d_icon = QPixmap(":pixmaps/enumeric.png");
    d_toolTip = "[wheelswitch, immediate change]";
}

QWidget *caNumericInterface::createWidget(QWidget *parent)
{
    return new caNumeric(parent);
}

caApplyNumericInterface::caApplyNumericInterface(QObject *parent): CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caApplyNumeric", "caapplynumeric", 0, 0, 160, 70, name, type, 1);
    d_name = "caApplyNumeric";
    d_include = "caApplyNumeric";
    d_icon = QPixmap(":pixmaps/enumeric.png");
    d_toolTip = "[wheelswitch with applybutton]";
}

QWidget *caApplyNumericInterface::createWidget(QWidget *parent)
{
    return new caApplyNumeric(parent);
}

EApplyButtonInterface::EApplyButtonInterface(QObject *parent): CustomWidgetInterface(parent)
{
    d_name = "EApplyButton";
    d_include = "EApplyButton";
    d_icon = QPixmap(":pixmaps/eapplybutton.png");
    d_domXml = 
        "<widget class=\"EApplyButton\" name=\"eApplyButton\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>87</width>\n"
        "   <height>28</height>\n"
        "  </rect>\n"
        " </property>\n"
        "</widget>\n";
}

QWidget *EApplyButtonInterface::createWidget(QWidget *parent)
{
    return new EApplyButton(parent);
}

QWidget *caSliderInterface::createWidget(QWidget* parent)
{
	return new caSlider(parent);
}

caSliderInterface::caSliderInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caSlider", "caslider", 0, 0, 100, 20, name, type, 1);
	d_name = "caSlider";
	d_include = "caSlider";
	d_icon = QPixmap(":pixmaps/slider.png");
        d_toolTip = "[slider]";
}

QWidget *caMenuInterface::createWidget(QWidget *parent)
{
    return new caMenu(parent);
}

caMenuInterface::caMenuInterface(QObject *parent): CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caMenu", "camenu", 0, 0, 100, 30, name, type, 1);
    d_name = "caMenu";
    d_include = "caMenu";
    d_icon = QPixmap(":pixmaps/menu.png");
    d_toolTip = "[menu combobox for display & control of enums with/without pv-label]";
}

QWidget *caChoiceInterface::createWidget(QWidget *parent)
{
    return new caChoice(parent);
}

caChoiceInterface::caChoiceInterface(QObject *parent): CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caChoice", "cachoice", 0, 0, 150, 150, name, type, 1);
    d_name = "caChoice";
    d_include = "caChoice";
    d_icon = QPixmap(":pixmaps/choice.png");
    d_toolTip = "[choice with different stacking modes for display & control of enums]";
}

QWidget *caRelatedDisplayInterface::createWidget(QWidget *parent)
{
    return new caRelatedDisplay(parent);
}

caRelatedDisplayInterface::caRelatedDisplayInterface(QObject *parent): CustomWidgetInterface(parent)
{
        strng name[5], type[5];
        strcpy(name[0], "label");
        strcpy(type[0], "multiline");
        strcpy(name[1], "labels");
        strcpy(type[1], "multiline");
        strcpy(name[2], "files");
        strcpy(type[2], "multiline");
        strcpy(name[3], "args");
        strcpy(type[3], "multiline");
        strcpy(name[4], "replaceModes");
        strcpy(type[4], "multiline");
        d_domXml = XmlFunc("caRelatedDisplay", "carelateddisplay", 0, 0, 150, 50, name, type, 5);
    d_name = "caRelatedDisplay";
    d_include = "caRelatedDisplay";
    d_icon = QPixmap(":pixmaps/fileopen.png");
    d_toolTip = "[displays a new or popups a synoptic view]";
}

QWidget *caShellCommandInterface::createWidget(QWidget *parent)
{
    return new caShellCommand(parent);
}

caShellCommandInterface::caShellCommandInterface(QObject *parent): CustomWidgetInterface(parent)
{
        strng name[4], type[4];
        strcpy(name[0], "label");
        strcpy(type[0], "multiline");
        strcpy(name[1], "labels");
        strcpy(type[1], "multiline");
        strcpy(name[2], "files");
        strcpy(type[2], "multiline");
        strcpy(name[3], "args");
        strcpy(type[3], "multiline");
        d_domXml = XmlFunc("caShellCommand", "cashellcommand", 0, 0, 150, 50, name, type, 4);
    d_name = "caShellCommand";
    d_include = "caShellCommand";
    d_icon = QPixmap(":pixmaps/exclamation.png");
    d_toolTip = "[menu or button for detached processes]";
}

QWidget *caTextEntryInterface::createWidget(QWidget* parent)
{
	return new caTextEntry(parent);
}

caTextEntryInterface::caTextEntryInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caTextEntry", "catextentry", 0, 0, 100, 22, name, type, 1);
	d_name = "caTextEntry";
	d_include = "caTextEntry";
	d_icon = QPixmap(":pixmaps/textentry.png");
        d_toolTip = "[enters a value/command to a pv]";
}

QWidget *caMessageButtonInterface::createWidget(QWidget* parent)
{
	return new caMessageButton(parent);
}

caMessageButtonInterface::caMessageButtonInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[4], type[4];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        strcpy(name[1], "label");
        strcpy(type[1], "multiline");
        strcpy(name[2], "releaseMessage");
        strcpy(type[2], "multiline");
        strcpy(name[3], "pressMessage");
        strcpy(type[3], "multiline");
        d_domXml = XmlFunc("caMessageButton", "camessagebutton", 0, 0, 100, 22, name, type, 4);
	d_name = "caMessageButton";
	d_include = "caMessageButton";
	d_icon = QPixmap(":pixmaps/message.png");
        d_toolTip = "[send a predefined value/command to a pv]";
}

QWidget *caToggleButtonInterface::createWidget(QWidget* parent)
{
	return new caToggleButton(parent);
}

caToggleButtonInterface::caToggleButtonInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caToggleButton", "catogglebutton", 0, 0, 100, 22, name, type, 1);
	d_name = "caToggleButton";
	d_include = "caToggleButton";
	d_icon = QPixmap(":pixmaps/radiobutton.png");
        d_toolTip = "[toggle 0/1 to a pv]";
}

QWidget *caScriptButtonInterface::createWidget(QWidget* parent)
{
	return new caScriptButton(parent);
}

caScriptButtonInterface::caScriptButtonInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[3], type[3];
        strcpy(name[0], "label");
        strcpy(type[0], "multiline");
        strcpy(name[1], "scriptCommand");
        strcpy(type[1], "multiline");
        strcpy(name[2], "scriptParameter");
        strcpy(type[2], "multiline");

        d_domXml = XmlFunc("caScriptButton", "cascriptbutton", 0, 0, 100, 22, name, type, 3);
	d_name = "caScriptButton";
	d_include = "caScriptButton";
	d_icon = QPixmap(":pixmaps/process.png");
        d_toolTip = "[execute a script or image as detached process]";
}

CustomWidgetCollectionInterface::CustomWidgetCollectionInterface(QObject *parent): QObject(parent)
{
    d_plugins.append(new caNumericInterface(this));
    d_plugins.append(new caApplyNumericInterface(this));
    d_plugins.append(new caSliderInterface(this));
    d_plugins.append(new caMenuInterface(this));
    d_plugins.append(new caChoiceInterface(this));
    d_plugins.append(new caRelatedDisplayInterface(this));
    d_plugins.append(new caShellCommandInterface(this));
    d_plugins.append(new caTextEntryInterface(this));
    d_plugins.append(new caMessageButtonInterface(this));
    d_plugins.append(new caToggleButtonInterface(this)); 
    d_plugins.append(new caScriptButtonInterface(this));  
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetCollectionInterface::customWidgets(void) const
{
    return d_plugins;
}
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) 
#else
Q_EXPORT_PLUGIN2(QtControls, CustomWidgetCollectionInterface)
#endif
