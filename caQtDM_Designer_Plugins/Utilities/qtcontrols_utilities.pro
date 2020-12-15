CAQTDM_PLUGINNAME = utilities
include(../Common/plugins.pri)

SOURCES	+= qtcontrols_utilities_plugin.cpp
HEADERS	+= qtcontrols_utilities_plugin.h  ../Common/designerPluginTexts.h
RESOURCES += ../Common/qtcontrolsplugin.qrc
TARGET = qtcontrols_utilities_plugin

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}
