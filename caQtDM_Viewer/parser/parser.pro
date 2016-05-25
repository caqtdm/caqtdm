include(../qtdefs.pri)
CONFIG += caQtDM_xdl2ui
include(../../caQtDM.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = app
INCLUDEPATH += .
MOC_DIR = moc

# Input
HEADERS += XmlWriter.h parser.h \
    myParser.h \
    QtProperties.h \
    dmsearchfile.h
SOURCES += myParser.cpp XmlWriter.cpp parser.c \
    QtProperties.c \
    dmsearchfile.cpp

TARGET = adl2ui

OTHER_FILES += \
    stylesheet.qss


