include (../../caQtDM_Viewer/qtdefs.pri)
include(../unitTests.pri)

QT += network gui widgets designer

SOURCES += tst_qtcontrols.cpp \
    tst_pvdialog.cpp \
    tst_gensoftpv.cpp \
    tst_canumeric.cpp \
    tst_caspinbox.cpp \
    tst_caapplynumeric.cpp \
    tst_caimage.cpp

HEADERS += tst_pvdialog.h \
    tst_gensoftpv.h \
    tst_canumeric.h \
    tst_caspinbox.h \
    tst_caapplynumeric.h \
    tst_caimage.h \
    tst_numeric_suite.h \
    fakeformwindow.h

# --- Tested classes below ---

HEADERS += ../../caQtDM_QtControls/src/pvdialog.h \
    ../../caQtDM_QtControls/src/caimage.h

SOURCES += ../../caQtDM_QtControls/src/pvdialog.cpp

INCLUDEPATH += ../../caQtDM_QtControls/src \
    ../../caQtDM_Lib/src \
    $$(QWTINCLUDE)

LIBS += \
    -L$$(CAQTDM_COLLECT) \
    -lqtcontrols

macx {
    LIBS += -lz
    LIBS += -F$$(QWTLIB) -framework $$(QWTLIBNAME)
    QMAKE_RPATHDIR += $$(QWTLIB)
}
