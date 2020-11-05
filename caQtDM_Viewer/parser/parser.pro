TARGET_PRODUCT = "ADL converter for Display Manager"
TARGET_FILENAME = "adl2ui.exe"

include(../qtdefs.pri)
CONFIG += caQtDM_xdl2ui caQtDM_xdl2ui_Lib
include(../../caQtDM.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = app
INCLUDEPATH += .
INCLUDEPATH += ../../caQtDM_Parsers/adlParserSrc
MOC_DIR = moc
RC_FILE = ../../caQtDM_Viewer/src/caQtDM.rc

# Input
HEADERS += adlParser.h
SOURCES += adlParser.cpp

TARGET = adl2ui



OTHER_FILES += \
    stylesheet.qss


