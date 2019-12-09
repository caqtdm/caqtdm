include(../../caQtDM_Viewer/qtdefs.pri)
CONFIG += caQtDM_xdl2ui
include(../../caQtDM.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = lib
CONFIG	+= static

INCLUDEPATH += .
INCLUDEPATH += ../edlParserSrc

MOC_DIR = moc
VPATH += ../edlParserSrc

QMAKE_CXXFLAGS += "-g -Wno-write-strings"

HEADERS += XmlWriter.h \
    edlParserMain.h \
    dmsearchfile.h \
    tag_pkg.h \
    utility.h \
    expString.h \
    parserClass.h

SOURCES += XmlWriter.cpp  \
    edlParserMain.cpp \
    dmsearchfile.cpp  \
    tag_pkg.cc \
    utility.cc \
    expString.cc \
    parserClass.cc

TARGET = edlParser

OTHER_FILES += \
    stylesheet.qss


