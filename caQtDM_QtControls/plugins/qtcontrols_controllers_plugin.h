/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */

#ifndef QTCONTROLS_CONTROLLERS_PLUGIN_H
#define QTCONTROLS_CONTROLLERS_PLUGIN_H

#include <qglobal.h>
#if QT_VERSION > 0x050400
    #include <QtUiPlugin/QDesignerCustomWidgetInterface>
#else
    #include <QDesignerCustomWidgetInterface>
#endif

class CustomWidgetInterface_Controllers: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    CustomWidgetInterface_Controllers(QObject *parent);

    virtual bool isContainer() const { if(d_name.contains("caFrame")) return true; else return false; }
    virtual bool isInitialized() const { return d_isInitialized; }
    virtual QIcon icon() const { return d_icon; }
    virtual QString codeTemplate() const { return d_codeTemplate; }
    virtual QString domXml() const { return d_domXml; }
    virtual QString group() const { return "caQtDM Controllers - PSI"; }
    virtual QString includeFile() const { return d_include; }
    virtual QString name() const { return d_name; }
    virtual QString toolTip() const { return d_toolTip; }
    virtual QString whatsThis() const { return d_whatsThis; }
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

class CustomWidgetCollectionInterface_Controllers: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)   
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.qtcontrols_controllers")  
#endif 

public:
    CustomWidgetCollectionInterface_Controllers(QObject *parent = NULL);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> d_plugins;
};

class caNumericInterface: public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caNumericInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class EApplyButtonInterface: public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    EApplyButtonInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caApplyNumericInterface: public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caApplyNumericInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caSliderInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caSliderInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caMenuInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caMenuInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caChoiceInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caChoiceInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caRelatedDisplayInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caRelatedDisplayInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caShellCommandInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caShellCommandInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caTextEntryInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caTextEntryInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caMessageButtonInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caMessageButtonInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caToggleButtonInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caToggleButtonInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caScriptButtonInterface : public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caScriptButtonInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caSpinboxInterface: public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caSpinboxInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caByteControllerInterface: public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caByteControllerInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caMimeDisplayInterface: public CustomWidgetInterface_Controllers
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caMimeDisplayInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};



#endif
