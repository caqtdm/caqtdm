TARGET_PRODUCT = "ALH config library for Display Manager"
TARGET_FILENAME = "alhParser.dll"

include(../../caQtDM_Viewer/qtdefs.pri)
CONFIG += caQtDM_xdl2ui
include(../../caQtDM.pri)

QT = core
contains(QT_VER_MAJ, 5) {
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = lib
CONFIG	+= shared plugin

DEFINES += ALHPARSER_MAKEDLL

INCLUDEPATH += .
INCLUDEPATH += ../alhParserSrc

MOC_DIR = moc
VPATH += ../alhParserSrc

RC_FILE = ../../caQtDM_Viewer/src/caQtDM.rc

HEADERS += alhmodel.h \
    alhconfigparser.h \
    alhuigenerator.h \
    alhparserdefs.h

SOURCES += alhmodel.cpp \
    alhconfigparser.cpp \
    alhuigenerator.cpp

TARGET = alhParser
