#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <QtControls>
#include <qtcontrols_graphics_plugin.h>
#include <qglobal.h>
#include <QtDesigner/QtDesigner>
#include <QtPlugin>

typedef char strng[40];

char *XmlFunc(const char *clss, const char *name, int x, int y, int w, int h, strng *propertyname, strng* propertytype, int nb)
{
  char mess[1024], *xml;
  
  if(strstr(name, "cadoubletabwidget") != (char*) 0) {
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
             <addpagemethod>addPage</addpagemethod>\
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
	
  } else {
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
  }

  xml = (char*) malloc(strlen(mess) * sizeof(char)+1);
  memcpy(xml, mess, strlen(mess) * sizeof(char)+1);

  //printf("%s\n", xml);
  return xml;
}

CustomWidgetInterface::CustomWidgetInterface(QObject *parent): QObject(parent), d_isInitialized(false)
{
}

void CustomWidgetInterface::initialize(QDesignerFormEditorInterface *formEditor)
{
    //printf("initialize function for %s\n", d_name.toAscii().constData());

    if (d_isInitialized)
        return;

// extension for PolyDraw
    if(d_name.contains("caPolyLine")) {
       QExtensionManager *manager = formEditor->extensionManager();
       Q_ASSERT(manager != 0);
       manager->registerExtensions(new caPolyLineTaskMenuFactory(manager),
                                Q_TYPEID(QDesignerTaskMenuExtension));
    }
    if(d_name.contains("caDoubleTabWidget")) {
       QExtensionManager *manager = formEditor->extensionManager();
       Q_ASSERT(manager != 0);
       manager->registerExtensions(new caDoubleTabWidgetExtensionFactory(manager),
                                Q_TYPEID(QDesignerContainerExtension));
    } 

    // set this property in order to find out later if we use our controls through the designer or otherwise
    qApp->setProperty("APP_SOURCE", QVariant(QString("DESIGNER")));

    d_isInitialized = true;
}

caLabelInterface::caLabelInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[5], type[5];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        strcpy(name[1], "channelB");
        strcpy(type[1], "multiline");
        strcpy(name[2], "channelC");
        strcpy(type[2], "multiline");
        strcpy(name[3], "channelD");
        strcpy(type[3], "multiline");
        strcpy(name[4], "visibilityCalc");
        strcpy(type[4], "multiline");
        d_domXml = XmlFunc("caLabel", "calabel", 0, 0, 90, 28, name, type, 5);
	d_name = "caLabel";
	d_include = "caLabel";
	d_icon = QPixmap(":pixmaps/elabel.png");
}

QWidget* caLabelInterface::createWidget(QWidget* parent)
{
	return new caLabel(parent);
}

QWidget *caFrameInterface::createWidget(QWidget* parent)
{
	return new caFrame(parent);
}

caFrameInterface::caFrameInterface(QObject* parent) : CustomWidgetInterface(parent)
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
        strcpy(name[4], "visibilityCalc");
        strcpy(type[4], "multiline");
        strcpy(name[5], "macro");
        strcpy(type[5], "multiline");
        d_domXml = XmlFunc("caFrame", "caframe", 0, 0, 120, 120, name, type, 6);
	d_name = "caFrame";
	d_include = "caFrame";
	d_icon = QPixmap(":pixmaps/frame.png");
}

caImageInterface::caImageInterface(QObject *parent): CustomWidgetInterface(parent)
{
        strng name[7], type[7];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        strcpy(name[1], "channelB");
        strcpy(type[1], "multiline");
        strcpy(name[2], "channelC");
        strcpy(type[2], "multiline");
        strcpy(name[3], "channelD");
        strcpy(type[3], "multiline");
        strcpy(name[4], "visibilityCalc");
        strcpy(type[4], "multiline");
        strcpy(name[5], "imageCalc");
        strcpy(type[5], "multiline");
        strcpy(name[6], "filename");
        strcpy(type[6], "multiline");
        d_domXml = XmlFunc("caImage", "caimage", 0, 0, 50, 50, name, type, 7);
    d_name = "caImage";
    d_include = "caImage";
    d_icon = QPixmap(":pixmaps/images.png");
}

QWidget *caImageInterface::createWidget(QWidget *parent)
{
    return new caImage(parent);
}

QWidget *caPolyLineInterface::createWidget(QWidget *parent)
{
    return new caPolyLine(parent);
}

caPolyLineInterface::caPolyLineInterface(QObject *parent): CustomWidgetInterface(parent)
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
        strcpy(name[4], "visibilityCalc");
        strcpy(type[4], "multiline");
        strcpy(name[5], "xyPairs");
        strcpy(type[5], "multiline");
        d_domXml = XmlFunc("caPolyLine", "capolyline", 0, 0, 150, 150, name, type, 6);
    d_name = "caPolyLine";
    d_include = "caPolyLine";
    d_icon = QPixmap(":pixmaps/polyline.png");
}

QWidget *caGraphicsInterface::createWidget(QWidget* parent)
{
	return new caGraphics(parent);
}

caGraphicsInterface::caGraphicsInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[5], type[5];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        strcpy(name[1], "channelB");
        strcpy(type[1], "multiline");
        strcpy(name[2], "channelC");
        strcpy(type[2], "multiline");
        strcpy(name[3], "channelD");
        strcpy(type[3], "multiline");
        strcpy(name[4], "visibilityCalc");
        strcpy(type[4], "multiline");
        d_domXml = XmlFunc("caGraphics", "cagraphics", 0, 0, 100, 100, name, type, 5);
	d_name = "caGraphics";
	d_include = "caGraphics";
	d_icon = QPixmap(":pixmaps/gtool.png");
}

QWidget *caIncludeInterface::createWidget(QWidget* parent)
{
	return new caInclude(parent);
}

caIncludeInterface::caIncludeInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[7], type[7];
        strcpy(name[0], "channel");
        strcpy(type[0], "multiline");
        strcpy(name[1], "channelB");
        strcpy(type[1], "multiline");
        strcpy(name[2], "channelC");
        strcpy(type[2], "multiline");
        strcpy(name[3], "channelD");
        strcpy(type[3], "multiline");
        strcpy(name[4], "visibilityCalc");
        strcpy(type[4], "multiline");
        strcpy(name[5], "macro");
        strcpy(type[5], "multiline");
        strcpy(name[6], "filename");
        strcpy(type[6], "multiline");
        d_domXml = XmlFunc("caInclude", "cainclude", 0, 0, 100, 100, name, type, 7);
	d_name = "caInclude";
	d_include = "caInclude";
	d_icon = QPixmap(":pixmaps/frame.png");
}

QWidget *caDoubleTabWidgetInterface::createWidget(QWidget* parent)
{
     caDoubleTabWidget *widget = new caDoubleTabWidget(parent);
     return widget;
}

caDoubleTabWidgetInterface::caDoubleTabWidgetInterface(QObject* parent) : CustomWidgetInterface(parent)
{
        strng name[2], type[2];
        strcpy(name[0], "itemsHorizontal");
        strcpy(type[0], "multiline");
        strcpy(name[1], "itemsVertical");
        strcpy(type[1], "multiline");
        d_domXml = XmlFunc("caDoubleTabWidget", "cadoubletabwidget", 0, 0, 250, 250, name, type, 2);
	d_name = "caDoubleTabWidget";
	d_include = "caDoubleTabWidget";
	d_icon = QPixmap(":pixmaps/tabwidget.png");
}

CustomWidgetCollectionInterface::CustomWidgetCollectionInterface(QObject *parent): QObject(parent)
{
    d_plugins.append(new caFrameInterface(this));
    d_plugins.append(new caLabelInterface(this) );
    d_plugins.append(new caGraphicsInterface(this));
    d_plugins.append(new caPolyLineInterface(this));
    d_plugins.append(new caImageInterface(this));
    d_plugins.append(new caIncludeInterface(this));
    d_plugins.append(new caDoubleTabWidgetInterface(this));
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetCollectionInterface::customWidgets(void) const
{
    return d_plugins;
}
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) 
#else
Q_EXPORT_PLUGIN2(QtControls, CustomWidgetCollectionInterface)
#endif
