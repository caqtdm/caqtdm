include(../qtdefs.pri)
CONFIG += caQtDM_xdl2ui
include(../../caQtDM.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}
!win32 {
  QMAKE_CXXFLAGS += "-Wno-write-strings"
}

TEMPLATE = app
INCLUDEPATH += .
MOC_DIR = moc

# Input
HEADERS += XmlWriter.h  \
    myParserEDM.h \
    QtProperties.h \
    dmsearchfile.h \
    tag_pkg.h \
    utility.h \
    expString.h \
    parserClass.h
SOURCES += myParserEDM.cpp XmlWriter.cpp \
    dmsearchfile.cpp \
    tag_pkg.cc \
    utility.cc \
    expString.cc \
    parserClass.cc

TARGET = edl2ui

OTHER_FILES += \
    stylesheet.qss



