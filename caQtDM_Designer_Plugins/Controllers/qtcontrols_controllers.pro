CAQTDM_PLUGINNAME = controllers
CAQTDM_INSTALL_LIB = qtcontrols_controllers_plugin
include(../Common/plugins.pri)

SOURCES	+= qtcontrols_controllers_plugin.cpp
HEADERS	+= qtcontrols_controllers_plugin.h  ../Common/designerPluginTexts.h
RESOURCES += ../Common/qtcontrolsplugin.qrc
TARGET = qtcontrols_controllers_plugin

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}
