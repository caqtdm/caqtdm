include(./plugins.pri)

SOURCES	+= qtcontrols_graphics_plugin.cpp
HEADERS	+= qtcontrols_graphics_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_graphics_plugin
DESTDIR = $(CAQTDM_COLLECT)
