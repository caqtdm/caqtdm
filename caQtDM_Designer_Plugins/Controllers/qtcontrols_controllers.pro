CAQTDM_PLUGINNAME = controllers
include(../Common/plugins.pri)

SOURCES	+= qtcontrols_controllers_plugin.cpp
HEADERS	+= qtcontrols_controllers_plugin.h  designerPluginTexts.h
RESOURCES += ../Common/qtcontrolsplugin.qrc
TARGET = qtcontrols_controllers_plugin

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}
