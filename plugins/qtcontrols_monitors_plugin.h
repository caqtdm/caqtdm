#ifndef QTCONTROLS_MONITORS_PLUGIN_H
#define QTCONTROLS_MONITORS_PLUGIN_H

#include <qglobal.h>

#include <QDesignerCustomWidgetInterface>

class CustomWidgetInterface: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    CustomWidgetInterface(QObject *parent);

    virtual bool isContainer() const { if(d_name.contains("caFrame")) return true; else return false; };
    virtual bool isInitialized() const { return d_isInitialized; };
    virtual QIcon icon() const { return d_icon; };
    virtual QString codeTemplate() const { return d_codeTemplate; };
    virtual QString domXml() const { return d_domXml; };
    virtual QString group() const { return "caQt_Monitors"; };
    virtual QString includeFile() const { return d_include; };
    virtual QString name() const { return d_name; };
    virtual QString toolTip() const { return d_toolTip; };
    virtual QString whatsThis() const { return d_whatsThis; };
    virtual void initialize(QDesignerFormEditorInterface *);

protected:
    QString d_name; 
    QString d_include; 
    QString d_toolTip; 
    QString d_whatsThis; 
    QString d_domXml; 
    QString d_codeTemplate;
    QIcon d_icon;

private:
    bool d_isInitialized;
};

class CustomWidgetCollectionInterface: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)   
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")  
#endif 

public:
    CustomWidgetCollectionInterface(QObject *parent = NULL);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> d_plugins;
};

/* CustomWidgetInterface */
class caBitnamesInterface: public CustomWidgetInterface 
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caBitnamesInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caLedInterface: public CustomWidgetInterface 
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caLedInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caLinearGaugeInterface: public CustomWidgetInterface 
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caLinearGaugeInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caCircularGaugeInterface: public CustomWidgetInterface 
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caCircularGaugeInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caLineEditInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caLineEditInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caThermoInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caThermoInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caCartesianPlotInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caCartesianPlotInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caStripPlotInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caStripPlotInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caByteInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caByteInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caTableInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caTableInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caCameraInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caCameraInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caCalcInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caCalcInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};


#endif
