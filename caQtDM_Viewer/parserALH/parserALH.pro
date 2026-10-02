TARGET_PRODUCT = "ALH config debug tool for Display Manager"
TARGET_FILENAME = "alh2ui.exe"

include(../qtdefs.pri)
CONFIG += caQtDM_xdl2ui

include(../../caQtDM.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets uitools
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}
contains(QT_VER_MAJ, 6) {
  QT       += widgets uitools
}
TEMPLATE = app
macx: CONFIG -= app_bundle
INCLUDEPATH += .
MOC_DIR = moc
RC_FILE = ../../caQtDM_Viewer/src/caQtDM.rc

# debug/test only: built with CAQTDM_ALH2UI=1, never installed or packaged
INCLUDEPATH += ../../caQtDM_Parsers/alhParserSrc
SOURCES += alh2ui.cpp

unix {
    LIBS += $(CAQTDM_COLLECT)/libalhParser.a
}
win32 {
    win32-msvc* || msvc {
        LIBS += $$(CAQTDM_COLLECT)/alhParser.lib
    }
    win32-g++ {
        LIBS += $$(CAQTDM_COLLECT)/libalhParser.a
    }
}

TARGET = alh2ui
