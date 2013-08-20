#ifndef QTCONTROLS_CONTROLLERS_PLUGIN_H
#define QTCONTROLS_CONTROLLERS_PLUGIN_H

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
    virtual QString group() const { return "caQt_Controllers"; };
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
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.qtcontrols_controllers")  
#endif 

public:
    CustomWidgetCollectionInterface(QObject *parent = NULL);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> d_plugins;
};

class caNumericInterface: public CustomWidgetInterface 
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caNumericInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class EApplyButtonInterface: public CustomWidgetInterface 
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    EApplyButtonInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caApplyNumericInterface: public CustomWidgetInterface 
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caApplyNumericInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caSliderInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caSliderInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caMenuInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caMenuInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caChoiceInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caChoiceInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caRelatedDisplayInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caRelatedDisplayInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caShellCommandInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caShellCommandInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caTextEntryInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caTextEntryInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caMessageButtonInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caMessageButtonInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caToggleButtonInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caToggleButtonInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caScriptButtonInterface : public CustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caScriptButtonInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};




#endif
