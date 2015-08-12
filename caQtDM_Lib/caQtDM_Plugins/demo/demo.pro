CONFIG += warn_on
CONFIG += release

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = demo_plugin.h ../controlsinterface.h
SOURCES         = demo_plugin.cpp
TARGET          = $$qtLibraryTarget(demo_plugin)
DESTDIR         = $(CAQTDM_COLLECT)/controlsystems


unix:!macx {
 INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
}

macx: {
        LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        plugins.path = Contents/PlugIns/controlsystems
        plugins.files += $(CAQTDM_COLLECT)/controlsystems/libdemo_plugin.dylib
}
