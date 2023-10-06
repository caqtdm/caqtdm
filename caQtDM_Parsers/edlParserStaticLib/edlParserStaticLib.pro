CAQTDM_TOP = ../..
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
CONFIG += caQtDM_xdl2ui
CONFIG += Define_Build_objDirs
CONFIG += caQtDM_Installation
CONFIG += Define_library_settings
CAQTDM_INSTALL_LIB = edlParser
include($$CAQTDM_TOP/caQtDM.pri)



contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}
contains(QT_VER_MAJ, 6) {
  QT       += widgets core
}

TEMPLATE = lib
CONFIG	+= static

INCLUDEPATH += .
INCLUDEPATH += ../edlParserSrc


VPATH += ../edlParserSrc



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


