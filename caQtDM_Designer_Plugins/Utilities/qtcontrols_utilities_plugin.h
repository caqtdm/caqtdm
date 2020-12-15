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

#ifndef QTCONTROLS_UTILITIES_PLUGIN_H
#define QTCONTROLS_UTILITIES_PLUGIN_H

#include <qglobal.h>
#if QT_VERSION > 0x050400
    #include <QtUiPlugin/QDesignerCustomWidgetInterface>
#else
    #include <QDesignerCustomWidgetInterface>
#endif

class CustomWidgetInterface_Utilities: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    CustomWidgetInterface_Utilities(QObject *parent);

    virtual bool isContainer() const { if(d_name.contains("caFrame")) return true; else return false; }
    virtual bool isInitialized() const { return d_isInitialized; }
    virtual QIcon icon() const { return d_icon; }
    virtual QString codeTemplate() const { return d_codeTemplate; }
    virtual QString domXml() const { return d_domXml; }
    virtual QString group() const { return "caQtDM Utilities - PSI"; }
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

class CustomWidgetCollectionInterface_Utilities: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)   
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.qtcontrols_utilities")
#endif 

public:
    CustomWidgetCollectionInterface_Utilities(QObject *parent = NULL);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> d_plugins;
};

class replaceMacroInterface : public CustomWidgetInterface_Utilities
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    replaceMacroInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class wmSignalPropagatorInterface : public CustomWidgetInterface_Utilities
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    wmSignalPropagatorInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caMimeDisplayInterface: public CustomWidgetInterface_Utilities
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caMimeDisplayInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caScriptButtonInterface : public CustomWidgetInterface_Utilities
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caScriptButtonInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caShellCommandInterface : public CustomWidgetInterface_Utilities
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caShellCommandInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};


#endif
