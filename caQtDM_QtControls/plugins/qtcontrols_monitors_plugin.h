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

#ifndef QTCONTROLS_MONITORS_PLUGIN_H
#define QTCONTROLS_MONITORS_PLUGIN_H

#include <qglobal.h>

#include <QDesignerCustomWidgetInterface>

class CustomWidgetInterface_Monitors: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    CustomWidgetInterface_Monitors(QObject *parent);

    virtual bool isContainer() const { if(d_name.contains("caFrame")) return true; else return false; };
    virtual bool isInitialized() const { return d_isInitialized; };
    virtual QIcon icon() const { return d_icon; };
    virtual QString codeTemplate() const { return d_codeTemplate; };
    virtual QString domXml() const { return d_domXml; };
    virtual QString group() const { return "caQtDM Monitors - PSI"; };
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

class CustomWidgetCollectionInterface_Monitors: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)   
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")  
#endif 

public:
    CustomWidgetCollectionInterface_Monitors(QObject *parent = NULL);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> d_plugins;
};

/* CustomWidgetInterface */
class caBitnamesInterface: public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caBitnamesInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caLedInterface: public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caLedInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caLinearGaugeInterface: public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caLinearGaugeInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caCircularGaugeInterface: public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caCircularGaugeInterface(QObject *parent);
    virtual QWidget *createWidget(QWidget *parent);
};

class caLineEditInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caLineEditInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caThermoInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caThermoInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caCartesianPlotInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caCartesianPlotInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caStripPlotInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caStripPlotInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caByteInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caByteInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caTableInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caTableInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caWaveTableInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caWaveTableInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};


class caCameraInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caCameraInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caCalcInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    caCalcInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caWaterfallPlotInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caWaterfallPlotInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};

class caMeterInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caMeterInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};
 
class caScan2DInterface : public CustomWidgetInterface_Monitors
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
	
public:
    caScan2DInterface(QObject* parent);
    virtual QWidget* createWidget(QWidget* parent);
};
#endif
