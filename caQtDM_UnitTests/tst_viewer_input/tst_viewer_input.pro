include(../../caQtDM_Viewer/qtdefs.pri)
include(../unitTests.pri)

QT += network gui widgets designer uitools printsupport

unix:!macx {
    equals(QT_MAJOR_VERSION, 5): QT += x11extras
    LIBS += -lX11
}

web {
    QT += websockets
}

# the viewer sources expect the build stamps that caQtDM_Viewer.pro/caQtDM.pri provide
DEFINES += SUPPORT=\\\"EPICS\\\"
DEFINES += BUILDARCH=\\\"UNITTEST\\\"
DEFINES += BUILDTIME=\\\"\\\"
DEFINES += BUILDDATE=\\\"\\\"

SOURCES += tst_viewer_input.cpp \
    tst_envfilter.cpp \
    tst_networkaccess.cpp

HEADERS += tst_envfilter.h \
    tst_networkaccess.h \
    ../helpers/fakenetworkaccessmanager.h \
    ../helpers/fakenetworkreply.h

# --- Tested classes below ---
# caqtdm_lib.cpp has to be compiled in: under UNIT_TESTING the class is renamed
# to CaQtDM_Lib_TEST, which the prebuilt library does not export.

HEADERS += ../../caQtDM_Viewer/src/fileopenwindow.h \
    ../../caQtDM_Viewer/src/messagebox.h \
    ../../caQtDM_Viewer/src/configDialog.h \
    ../../caQtDM_Viewer/src/loggingcategories.h \
    ../../caQtDM_QtControls/src/networkaccess.h \
    ../../caQtDM_Lib/src/caqtdm_lib.h \
    ../../caQtDM_Lib/src/panelthemeapplier.h

SOURCES += ../../caQtDM_Viewer/src/fileopenwindow.cpp \
    ../../caQtDM_Viewer/src/messagebox.cpp \
    ../../caQtDM_Viewer/src/configDialog.cpp \
    ../../caQtDM_QtControls/src/networkaccess.cpp \
    ../../caQtDM_Lib/src/caqtdm_lib.cpp \
    ../../caQtDM_Lib/src/panelthemeapplier.cpp

FORMS += ../../caQtDM_Viewer/src/main.ui

INCLUDEPATH += ../helpers \
    ../../caQtDM_Viewer/src \
    ../../caQtDM_QtControls/src \
    ../../caQtDM_Lib \
    ../../caQtDM_Lib/src \
    ../../caQtDM_Plugins \
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
        -L$$(QWTHOME)/lib \
        -l$$(QWTLIBNAME)
}
