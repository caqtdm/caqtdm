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

#ifndef QTCONTROLS_GRAPHICS_PLUGIN_H
#define QTCONTROLS_GRAPHICS_PLUGIN_H

#include <qglobal.h>

#if QT_VERSION > QT_VERSION_CHECK(5, 4, 0)
    #include <QtUiPlugin/QDesignerCustomWidgetInterface>
#else
    #include <QDesignerCustomWidgetInterface>
#endif

class CustomWidgetInterface_Graphics: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    CustomWidgetInterface_Graphics(QObject *parent);

    virtual bool isContainer() const {
      if(d_name.contains("caFrame")) {
	return true; 
      } else if(d_name.contains("caDoubleTabWidget")) {
	return true;
      } else {
	return false; 
    }
    }
    virtual bool isInitialized() const { return d_isInitialized; }
    virtual QIcon icon() const { return d_icon; }
    virtual QString codeTemplate() const { return d_codeTemplate; }
    virtual QString domXml() const { return d_domXml; }
    virtual QString group() const { return "caQtDM Graphics -PSI"; }
    virtual QString includeFile() const { return d_include; }
    virtual QString name() const { return d_name; }
    virtual QString toolTip() const { return d_toolTip; }
    virtual QString whatsThis() const { return d_whatsThis; }
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    void initialize(QDesignerFormEditorInterface *core) override;
#else
    virtual void initialize(QDesignerFormEditorInterface *);
#endif

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

class CustomWidgetCollectionInterface_Graphics: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)   
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")  
#endif 

public:
    CustomWidgetCollectionInterface_Graphics(QObject *parent = NULL);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> d_plugins;
};

class caLabelInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caLabelInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caLabelVerticalInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caLabelVerticalInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};


class caGraphicsInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caGraphicsInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caFrameInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caFrameInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caImageInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caImageInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caPolyLineInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caPolyLineInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caIncludeInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caIncludeInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caDoubleTabWidgetInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caDoubleTabWidgetInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caClockInterface : public CustomWidgetInterface_Graphics
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caClockInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

#endif
