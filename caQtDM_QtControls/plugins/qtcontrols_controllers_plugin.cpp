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

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <QtControls>
#include <qtcontrols_controllers_plugin.h>
#include <qglobal.h>
#ifndef MOBILE
#include <QtDesigner/QtDesigner>
#endif
#include <QtPlugin>

#include "designerPluginTexts.h"

typedef char strng[40];
typedef char longtext[500];

static QString XmlFunc(const char *clss, const char *name, int x, int y, int w, int h,
                strng *propertyname, strng* propertytype, longtext *propertytext, int nb)
{
#ifndef DESIGNER_TOOLTIP_DESCRIPTIONS
    Q_UNUSED(propertytext);
#endif
    QString mess = "";
    QString strng1 = "";
    QString strng2 = "";

    mess = "<ui language=\"c++\"><widget class=\"%1\" name=\"%2\">\
            <property name=\"geometry\">\
            <rect>\
            <x>%3</x>\
            <y>%4</y>\
            <width>%5</width>\
            <height>%6</height>\
            </rect>\
            </property>\
            </widget>";

            mess = mess.arg(clss).arg(name).arg(x).arg(y).arg(w).arg(h);

    if(nb > 0) {
        strng1 = " <customwidgets><customwidget><class>%1</class><propertyspecifications>";
        strng1 = strng1.arg(clss);
        for(int i=0; i<nb; i++) {
#ifdef DESIGNER_TOOLTIP_DESCRIPTIONS
            QString strng3 = "<tooltip name=\"%1\">%2</tooltip>";
            strng3 = strng3.arg(propertyname[i]).arg(propertytext[i]);
            strng1.append(strng3);
#endif
            if(strstr(propertytype[i], "multiline") != (char*) 0) {
                strng2 = " <stringpropertyspecification name=\"%1\" notr=\"true\" type=\"%2\"/>";
                strng2 = strng2.arg(propertyname[i]).arg(propertytype[i]);
            }
            strng1.append(strng2);
        }
        strng1.append(" </propertyspecifications></customwidget></customwidgets>");

    }
    mess.append(strng1);
    mess.append("</ui>");

    //control output in formatted xml format */
/*
    QString formattedOutput;
    QDomDocument doc;
    doc.setContent(mess, false);
    QTextStream writer(&formattedOutput);
    doc.save(writer, 4);
    qDebug() << formattedOutput;
*/
    return mess;
}
CustomWidgetInterface_Controllers::CustomWidgetInterface_Controllers(QObject *parent): QObject(parent), d_isInitialized(false)
{
}

void CustomWidgetInterface_Controllers::initialize(QDesignerFormEditorInterface *)
{
    if (d_isInitialized)
        return;

    d_isInitialized = true;
}

caNumericInterface::caNumericInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[2], type[2] = {"",""};
    longtext text[2] = {"",""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");

    d_domXml = XmlFunc("caNumeric", "canumeric", 0, 0, 100, 50, name, type, text, 1);
    d_name = "caNumeric";
    d_include = "caNumeric";
    QPixmap qpixmap =  QPixmap(":pixmaps/enumeric.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[wheelswitch, immediate change]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caNumericInterface::createWidget(QWidget *parent)
{
    return new caNumeric(parent);
}

caApplyNumericInterface::caApplyNumericInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caApplyNumeric", "caapplynumeric", 0, 0, 160, 70, name, type, text, 1);
    d_name = "caApplyNumeric";
    d_include = "caApplyNumeric";
    QPixmap qpixmap =  QPixmap(":pixmaps/enumeric.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[wheelswitch with applybutton]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caApplyNumericInterface::createWidget(QWidget *parent)
{
    return new caApplyNumeric(parent);
}

EApplyButtonInterface::EApplyButtonInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    d_name = "EApplyButton";
    d_include = "EApplyButton";
    QPixmap qpixmap = QPixmap(":pixmaps/eapplybutton.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio);
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

caSliderInterface::caSliderInterface(QObject* parent) : CustomWidgetInterface_Controllers(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caSlider", "caslider", 0, 0, 20, 150, name, type, text, 1);
    d_name = "caSlider";
    d_include = "caSlider";
    QPixmap qpixmap = QPixmap(":pixmaps/slider.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[slider]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caMenuInterface::createWidget(QWidget *parent)
{
    return new caMenu(parent);
}

caMenuInterface::caMenuInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caMenu", "camenu", 0, 0, 100, 30, name, type, text, 1);
    d_name = "caMenu";
    d_include = "caMenu";
    QPixmap qpixmap =  QPixmap(":pixmaps/menu.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[menu combobox for display & control of enums with/without pv-label]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caChoiceInterface::createWidget(QWidget *parent)
{
    return new caChoice(parent);
}

caChoiceInterface::caChoiceInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};
    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caChoice", "cachoice", 0, 0, 150, 150, name, type, text, 1);
    d_name = "caChoice";
    d_include = "caChoice";
    QPixmap qpixmap =  QPixmap(":pixmaps/choice.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[choice with different stacking modes for display & control of enums]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caRelatedDisplayInterface::createWidget(QWidget *parent)
{
    return new caRelatedDisplay(parent);
}

caRelatedDisplayInterface::caRelatedDisplayInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[9], type[9] = {"","","","","","","","",""};
    longtext text[9] = {"","","","","",STRINGFROMLIST, STRINGFROMLIST, STRINGFROMLIST, STRINGFROMLIST};

    strcpy(name[0], "label");
    strcpy(type[0], "multiline");
    strcpy(name[1], "labelsList");
    strcpy(name[2], "filesList");
    strcpy(name[3], "argsList");
    strcpy(name[4], "removeParentList");
    strcpy(name[5], "labels");
    strcpy(type[5], "multiline");
    strcpy(name[6], "files");
    strcpy(type[6], "multiline");
    strcpy(name[7], "args");
    strcpy(type[7], "multiline");
    strcpy(name[8], "removeParent");
    strcpy(type[8], "multiline");
    d_domXml = XmlFunc("caRelatedDisplay", "carelateddisplay", 0, 0, 170, 70, name, type, text, 9);
    d_name = "caRelatedDisplay";
    d_include = "caRelatedDisplay";
    QPixmap qpixmap =   QPixmap(":pixmaps/fileopen.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[displays a new or popups a synoptic view]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caShellCommandInterface::createWidget(QWidget *parent)
{
    return new caShellCommand(parent);
}

caShellCommandInterface::caShellCommandInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[4], type[4] = {"","","",""};
    longtext text[4] = {"","","",""};

    strcpy(name[0], "label");
    strcpy(type[0], "multiline");
    strcpy(name[1], "labelsList");
    strcpy(name[2], "filesList");
    strcpy(name[3], "argsList");
    strcpy(type[3], "multiline");
    d_domXml = XmlFunc("caShellCommand", "cashellcommand", 0, 0, 170, 70, name, type, text, 4);
    d_name = "caShellCommand";
    d_include = "caShellCommand";
    QPixmap qpixmap =   QPixmap(":pixmaps/exclamation.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[menu or button for detached processes]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caTextEntryInterface::createWidget(QWidget* parent)
{
    return new caTextEntry(parent);
}

caTextEntryInterface::caTextEntryInterface(QObject* parent) : CustomWidgetInterface_Controllers(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caTextEntry", "catextentry", 0, 0, 100, 22, name, type, text, 1);
    d_name = "caTextEntry";
    d_include = "caTextEntry";

    QPixmap qpixmap =   QPixmap(":pixmaps/textentry.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[enters a value/command to a pv]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caMessageButtonInterface::createWidget(QWidget* parent)
{
    return new caMessageButton(parent);
}

caMessageButtonInterface::caMessageButtonInterface(QObject* parent) : CustomWidgetInterface_Controllers(parent)
{
    strng name[4], type[4] = {"","","",""};
    longtext text[4] = {"","","",""};
    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    strcpy(name[1], "label");
    strcpy(type[1], "multiline");
    strcpy(name[2], "releaseMessage");
    strcpy(type[2], "multiline");
    strcpy(name[3], "pressMessage");
    strcpy(type[3], "multiline");
    d_domXml = XmlFunc("caMessageButton", "camessagebutton", 0, 0, 100, 22, name, type, text, 4);
    d_name = "caMessageButton";
    d_include = "caMessageButton";
    QPixmap qpixmap =  QPixmap(":pixmaps/message.png");
    d_icon = qpixmap.scaled(90, 90, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[send a predefined value/command to a pv]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caToggleButtonInterface::createWidget(QWidget* parent)
{
    return new caToggleButton(parent);
}

caToggleButtonInterface::caToggleButtonInterface(QObject* parent) : CustomWidgetInterface_Controllers(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caToggleButton", "catogglebutton", 0, 0, 100, 22, name, type, text, 1);
    d_name = "caToggleButton";
    d_include = "caToggleButton";
    QPixmap qpixmap = QPixmap(":pixmaps/radiobutton.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[toggle 0/1 to a pv]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caScriptButtonInterface::createWidget(QWidget* parent)
{
    return new caScriptButton(parent);
}

caScriptButtonInterface::caScriptButtonInterface(QObject* parent) : CustomWidgetInterface_Controllers(parent)
{
    strng name[3], type[3] = {"","",""};
    longtext text[3] = {"","",""};

    strcpy(name[0], "label");
    strcpy(type[0], "multiline");
    strcpy(name[1], "scriptCommand");
    strcpy(type[1], "multiline");
    strcpy(name[2], "scriptParameter");
    strcpy(type[2], "multiline");

    d_domXml = XmlFunc("caScriptButton", "cascriptbutton", 0, 0, 100, 22, name, type, text, 3);
    d_name = "caScriptButton";
    d_include = "caScriptButton";
    QPixmap qpixmap = QPixmap(":pixmaps/process.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[execute a script or image as detached process]";
    d_whatsThis = "hello, i am a whatsthis string";
}

caSpinboxInterface::caSpinboxInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caSpinbox", "caspinbox", 0, 0, 80, 30, name, type, text, 1);
    d_name = "caSpinbox";
    d_include = "caSpinbox";
    QPixmap qpixmap = QPixmap(":pixmaps/spinbox.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[spinbox]";
    d_whatsThis = "hello, i am a whatsthis string";
}

QWidget *caSpinboxInterface::createWidget(QWidget *parent)
{
    return new caSpinbox(parent);
}

caByteControllerInterface::caByteControllerInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[1], type[1] = {""};
    longtext text[1] = {""};

    strcpy(name[0], "channel");
    strcpy(type[0], "multiline");
    d_domXml = XmlFunc("caByteController", "cabytecontroller", 0, 0, 20, 120, name, type, text, 1);
    d_toolTip = "[Byte Controller]";
    d_name = "caByteController";
    d_include = "caByteController";
    QPixmap qpixmap = QPixmap(":pixmaps/eflag.png");
    d_icon = qpixmap.scaled(70, 70, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QWidget *caByteControllerInterface::createWidget(QWidget *parent)
{
    return new caByteController(parent);
}


caMimeDisplayInterface::caMimeDisplayInterface(QObject *parent): CustomWidgetInterface_Controllers(parent)
{
    strng name[3], type[3] = {"","",""};
    longtext text[3] = {"","","mime file will be looked up through absolute path or caQtDM_DISPLAY_PATH\nor caQTDM_MIME_PATH. Separate files with a semicolumn\n"};

    strcpy(name[0], "label");
    strcpy(type[0], "multiline");
    strcpy(name[1], "labelsList");
    strcpy(name[2], "filesList");
    strcpy(type[2], "multiline");

    d_domXml = XmlFunc("caMimeDisplay", "camimedisplay", 0, 0, 100, 22, name, type, text, 3);
    d_toolTip = "[Mime display]";
    d_name = "caMimeDisplay";
    d_include = "caMimeDisplay";
    QPixmap qpixmap =  QPixmap(":pixmaps/mime.png");
    d_icon = qpixmap.scaled(90, 90, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    d_toolTip = "[calls a mime application for file]";
}

QWidget *caMimeDisplayInterface::createWidget(QWidget *parent)
{
    return new caMimeDisplay(parent);
}


CustomWidgetCollectionInterface_Controllers::CustomWidgetCollectionInterface_Controllers(QObject *parent): QObject(parent)
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
    d_plugins.append(new caSpinboxInterface(this));
    d_plugins.append(new caByteControllerInterface(this));
    d_plugins.append(new caMimeDisplayInterface(this));
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetCollectionInterface_Controllers::customWidgets(void) const
{
    return d_plugins;
}
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) 
#else
Q_EXPORT_PLUGIN2(QtControls, CustomWidgetCollectionInterface_Controllers)
#endif
