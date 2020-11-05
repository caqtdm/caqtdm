TARGET_PRODUCT = "ADL converter library for Display Manager"
TARGET_FILENAME = "adlParser.dll"

include(../../caQtDM_Viewer/qtdefs.pri)
CONFIG += caQtDM_xdl2ui
include(../../caQtDM.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = lib
CONFIG	+= shared plugin

INCLUDEPATH += .
INCLUDEPATH += ../adlParserSrc

MOC_DIR = moc
VPATH += ../adlParserSrc

RC_FILE = ../../caQtDM_Viewer/src/caQtDM.rc

HEADERS += XmlWriter.h \
    adlParserMain.h \
    QtProperties.h \
    dmsearchfile.h

SOURCES += XmlWriter.cpp parser.c \
    adlParserMain.cpp \
    QtProperties.c \
    dmsearchfile.cpp


TARGET = adlParser

OTHER_FILES += \
    stylesheet.qss


