include(../../caQtDM_Viewer/qtdefs.pri)
include(../unitTests.pri)

QT += network gui widgets designer uitools printsupport

web {
    QT += websockets
}

SOURCES += tst_lib.cpp \
    tst_caqtdm_lib.cpp

HEADERS += tst_caqtdm_lib.h \
    fakefileopenwindow.h

# --- Tested classes below ---

HEADERS += ../../caQtDM_Lib/src/caqtdm_lib.h \
    ../../caQtDM_Plugins/internal/internal_channel.h \
    ../../caQtDM_Plugins/internal/internal_plugin.h

SOURCES += ../../caQtDM_Lib/src/caqtdm_lib.cpp \
    ../../caQtDM_Plugins/internal/internal_channel.cpp \
    ../../caQtDM_Plugins/internal/internal_plugin.cpp

FORMS += ../../caQtDM_Viewer/src/main.ui

INCLUDEPATH += ../../caQtDM_QtControls/src \
    ../../caQtDM_Lib \
    ../../caQtDM_Lib/src \
    ../../caQtDM_Plugins \
    ../../caQtDM_Plugins/internal \
    ../../caQtDM_Parsers/adlParserSrc \
    ../../caQtDM_Parsers/edlParserSrc \
    $$(QWTINCLUDE) \
    $$(EPICSINCLUDE)

LIBS += \
    -L$$(CAQTDM_COLLECT) \
    -lcaQtDM_Lib \
    -lqtcontrols

_EPICSLIB = $$(EPICSLIB)
!isEmpty(_EPICSLIB) {
    QMAKE_RPATHDIR += $$(EPICSLIB)
    LIBS += -L$$(EPICSLIB) -lca -lCom
} else {
    QMAKE_RPATHDIR += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
    LIBS += -L$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) -lca -lCom
}

macx {
    LIBS += -F$$(QWTLIB) -framework $$(QWTLIBNAME)
    QMAKE_RPATHDIR += $$(QWTLIB)
} else {
    LIBS += \
        -L$$(QWTLIB) \
        -l$$(QWTLIBNAME)
}
