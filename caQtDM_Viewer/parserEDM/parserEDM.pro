CAQTDM_TOP=../..
TARGET_PRODUCT = "EDL converter for Display Manager"


include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
CONFIG += caQtDM_xdl2ui caQtDM_xdl2ui_Lib
CONFIG += caQtDM_Installation
include($$CAQTDM_TOP/caQtDM.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = app
INCLUDEPATH += .
INCLUDEPATH += ../../caQtDM_Parsers/edlParserSrc
MOC_DIR = moc

# Input
HEADERS += \
    edlParser.h
SOURCES += \
    edlParser.cpp

TARGET = edl2ui

OTHER_FILES += \
    stylesheet.qss

