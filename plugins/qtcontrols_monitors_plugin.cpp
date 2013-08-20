#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <QtControls>
#include <qtcontrols_monitors_plugin.h>
#include <qglobal.h>
#include <QtPlugin>
typedef char strng[40];

char *XmlFunc(const char *clss, const char *name, int x, int y, int w, int h, strng propertyname[], strng propertytype[], int nb)
{
  char mess[2048], *xml;
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
    if (d_isInitialized) return;
    d_isInitialized = true;
}

caBitnamesInterface::caBitnamesInterface(QObject *parent): CustomWidgetInterface(parent)
{
    strng name[2], type[2];
    strcpy(name[0], "channelEnum");
    strcpy(type[0], "multiline");
    strcpy(name[1], "channelValue");
    strcpy(type[1], "multiline");
    d_domXml = XmlFunc("caBitnames", "cabitnames", 0, 0, 160, 90, name, type, 2);
    d_toolTip = "[Enum table with state]";
    d_name = "caBitnames";
    d_include = "caBitnames";
    d_icon = QPixmap(":pixmaps/eflag.png");
}

QWidget *caBitnamesInterface::createWidget(QWidget *parent)
{
    return new caBitnames(parent);
}

caLedInterface::caLedInterface(QObject *parent): CustomWidgetInterface(parent)
{
    strng name[1], type[1];
    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caLed", "caled", 0, 0, 30, 30, name, type, 1);
    d_toolTip = "[LED for true or false bitstate or for severity states]";
    d_name = "caLed";
    d_include = "caLed";
    d_icon = QPixmap(":pixmaps/eled.png");
}

QWidget *caLedInterface::createWidget(QWidget *parent)
{
    return new caLed(parent);
}

caLinearGaugeInterface::caLinearGaugeInterface(QObject *parent): CustomWidgetInterface(parent)
{
    strng name[1], type[1];
    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caLinearGauge", "calineargauge", 0, 0, 40, 100, name, type, 1);
    d_toolTip = "[Colorized bar monitor]";
    d_name = "caLinearGauge";
    d_include = "caGauge";
    d_icon = QPixmap(":pixmaps/elineargauge.png");
}

QWidget *caLinearGaugeInterface::createWidget(QWidget *parent)
{
    return new caLinearGauge(parent);
}

caCircularGaugeInterface::caCircularGaugeInterface(QObject *parent): CustomWidgetInterface(parent)
{
    strng name[1], type[1];
    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caCircularGauge", "cacirculargauge", 0, 0, 100, 100, name, type, 1);
    d_toolTip = "[Colorized meter]";
    d_name = "caCircularGauge";
    d_include = "caGauge";
    d_icon = QPixmap(":pixmaps/ecirculargauge.png");
}

QWidget *caCircularGaugeInterface::createWidget(QWidget *parent)
{
    return new caCircularGauge(parent);
}

QWidget *caLineEditInterface::createWidget(QWidget* parent)
{
	return new caLineEdit(parent);
}

caLineEditInterface::caLineEditInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caLineEdit", "calineedit", 0, 0, 100, 20, name, type, 1);
        d_toolTip = "[Text Monitor]";
	d_name = "caLineEdit";
	d_include = "caLineEdit";
	d_icon = QPixmap(":pixmaps/textmonitor.png");
}

QWidget *caThermoInterface::createWidget(QWidget* parent)
{
	return new caThermo(parent);
}

caThermoInterface::caThermoInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[1], type[1];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        d_domXml = XmlFunc("caThermo", "cathermo", 0, 0, 100, 30, name, type, 1);
        d_toolTip = "[Scale & Bar Monitor]";
	d_name = "caThermo";
	d_include = "caThermo";
	d_icon = QPixmap(":pixmaps/thermo.png");
}

QWidget *caCartesianPlotInterface::createWidget(QWidget* parent)
{
	return new caCartesianPlot(parent);
}

caCartesianPlotInterface::caCartesianPlotInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[12], type[12];
        strcpy(name[0], "channels_1");
        strcpy(type[0], "multiline");
        strcpy(name[1], "channels_2");
        strcpy(type[1], "multiline");
        strcpy(name[2], "channels_3");
        strcpy(type[2], "multiline");
        strcpy(name[3], "channels_4");
        strcpy(type[3], "multiline");
        strcpy(name[4], "channels_5");
        strcpy(type[4], "multiline");
        strcpy(name[5], "channels_6");
        strcpy(type[5], "multiline");
        strcpy(name[6], "countNumOrChannel");
        strcpy(type[6], "multiline");
        strcpy(name[7], "triggerChannel");
        strcpy(type[7], "multiline");
        strcpy(name[8], "eraseChannel");
        strcpy(type[8], "multiline");
        strcpy(name[9], "Title");
        strcpy(type[9], "multiline");
        strcpy(name[10], "TitleX");
        strcpy(type[10], "multiline");
        strcpy(name[11], "TitleY");
        strcpy(type[11], "multiline");
        d_domXml = XmlFunc("caCartesianPlot", "cacartesianplot", 0, 0, 200, 50, name, type, 12);
        d_toolTip = "[Cartesian Plot]";
	d_name = "caCartesianPlot";
	d_include = "caCartesianPlot";
	d_icon = QPixmap(":pixmaps/cartesian.png");
}

QWidget *caStripPlotInterface::createWidget(QWidget* parent)
{
	return new caStripPlot(parent);
}

caStripPlotInterface::caStripPlotInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[4], type[4];
        strcpy(name[0], "channels");
        strcpy(type[0], "multiline");
        strcpy(name[1], "Title");
        strcpy(type[1], "multiline");
        strcpy(name[2], "TitleX");
        strcpy(type[2], "multiline");
        strcpy(name[3], "TitleY");
        strcpy(type[3], "multiline");
        d_domXml = XmlFunc("caStripPlot", "castripplot", 0, 0, 150, 50, name, type, 4);
        d_toolTip = "[Strip Chart]";
	d_name = "caStripPlot";
	d_include = "caStripPlot";
	d_icon = QPixmap(":pixmaps/qwtplot.png");
}

caByteInterface::caByteInterface(QObject *parent): CustomWidgetInterface(parent)
{
    strng name[1], type[1];
    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caByte", "cabyte", 0, 0, 20, 120, name, type, 1);
    d_toolTip = "[Byte Monitor]";
    d_name = "caByte";
    d_include = "caByte";
    d_icon = QPixmap(":pixmaps/eflag.png");
}

QWidget *caByteInterface::createWidget(QWidget *parent)
{
    return new caByte(parent);
}

caTableInterface::caTableInterface(QObject *parent): CustomWidgetInterface(parent)
{
    strng name[1], type[1];
    strcpy(name[0], "channels");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caTable", "catable", 0, 0, 120, 120, name, type, 1);
    d_toolTip = "[Table Monitor]";
    d_name = "caTable";
    d_include = "caTable";
    d_icon = QPixmap(":pixmaps/table.png");
}

QWidget *caTableInterface::createWidget(QWidget *parent)
{
    return new caTable(parent);
}

QWidget *caCameraInterface::createWidget(QWidget* parent)
{
    return new caCamera(parent);
}

caCameraInterface::caCameraInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[8], type[8];
        strcpy(name[0], "channelData");
        strcpy(type[0], "multiline");
        strcpy(name[1], "channelWidth");
        strcpy(type[1], "multiline");
        strcpy(name[2], "channelHeight");
        strcpy(type[2], "multiline");
        strcpy(name[3], "channelCode");
        strcpy(type[3], "multiline");
        strcpy(name[4], "channelBPP");
        strcpy(type[4], "multiline");
        strcpy(name[5], "dataProcChannels");
        strcpy(type[5], "multiline");
        strcpy(name[6], "minLevel");
        strcpy(type[6], "multiline");
        strcpy(name[7], "maxLevel");
        strcpy(type[7], "multiline");
        d_domXml = XmlFunc("caCamera", "cacamera", 0, 0, 200, 200, name, type, 8);
        d_toolTip = "[Image from camera]";
	d_name = "caCamera";
	d_include = "caCamera";
	d_icon = QPixmap(":pixmaps/camera.png");
}

QWidget *caCalcInterface::createWidget(QWidget* parent)
{
    return new caCalc(parent);
}

caCalcInterface::caCalcInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[6], type[6];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        strcpy(name[1], "channelB");
        strcpy(type[1], "multiline");
        strcpy(name[2], "channelC");
        strcpy(type[2], "multiline");
        strcpy(name[3], "channelD");
        strcpy(type[3], "multiline");
        strcpy(name[4], "calc");
        strcpy(type[4], "multiline");
        strcpy(name[5], "variable");
        strcpy(type[5], "multiline");

        d_domXml = XmlFunc("caCalc", "cacalc", 0, 0, 5, 5, name, type, 6);
        d_toolTip = "[define internal process variable]";
	d_name = "caCalc";
	d_include = "caCalc";
	d_icon = QPixmap(":pixmaps/calc.png");
}

CustomWidgetCollectionInterface::CustomWidgetCollectionInterface(QObject *parent): QObject(parent)
{
    d_plugins.append(new caLedInterface(this));
    d_plugins.append(new caLinearGaugeInterface(this));
    d_plugins.append(new caCircularGaugeInterface(this));
    d_plugins.append(new caLineEditInterface(this));
    d_plugins.append(new caThermoInterface(this));
    d_plugins.append(new caCartesianPlotInterface(this));
    d_plugins.append(new caStripPlotInterface(this));
    d_plugins.append(new caByteInterface(this));
    d_plugins.append(new caTableInterface(this));
    d_plugins.append(new caBitnamesInterface(this));
    d_plugins.append(new caCameraInterface(this));
    d_plugins.append(new caCalcInterface(this));
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetCollectionInterface::customWidgets(void) const
{
    return d_plugins;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) 
#else
Q_EXPORT_PLUGIN2(QtControls, CustomWidgetCollectionInterface)
#endif
