include (../../caQtDM_Viewer/qtdefs.pri)
include(../unitTests.pri)

QT += network gui widgets designer

SOURCES += tst_qtcontrols.cpp \
    tst_pvdialog.cpp \
    tst_gensoftpv.cpp \
    tst_canumeric.cpp \
    tst_caspinbox.cpp \
    tst_caapplynumeric.cpp \
    tst_alhparser.cpp \
    tst_alarmtree_logic.cpp

HEADERS += tst_pvdialog.h \
    tst_gensoftpv.h \
    tst_canumeric.h \
    tst_caspinbox.h \
    tst_caapplynumeric.h \
    tst_numeric_suite.h \
    fakeformwindow.h \
    tst_alhparser.h \
    tst_alarmtree_logic.h

# --- Tested classes below ---

HEADERS += ../../caQtDM_QtControls/src/pvdialog.h

SOURCES += ../../caQtDM_QtControls/src/pvdialog.cpp

# alh parser library sources compiled directly (caQtDM_Parsers/alhParserSrc)
SOURCES += ../../caQtDM_Parsers/alhParserSrc/alhmodel.cpp \
    ../../caQtDM_Parsers/alhParserSrc/alhconfigparser.cpp \
    ../../caQtDM_Parsers/alhParserSrc/alhuigenerator.cpp
RESOURCES += tst_alh_fixtures.qrc

INCLUDEPATH += ../../caQtDM_QtControls/src \
    ../../caQtDM_Lib/src \
    ../../caQtDM_Parsers/alhParserSrc \
    $$(QWTINCLUDE)

LIBS += \
    -L$$(CAQTDM_COLLECT) \
    -lqtcontrols

macx {
    LIBS += -lz
    LIBS += -F$$(QWTLIB) -framework $$(QWTLIBNAME)
    QMAKE_RPATHDIR += $$(QWTLIB)
}
