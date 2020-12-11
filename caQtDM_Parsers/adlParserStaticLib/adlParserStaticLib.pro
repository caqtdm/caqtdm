CAQTDM_TOP = ../..
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
CONFIG += caQtDM_xdl2ui
include($$CAQTDM_TOP/caQtDM.pri)


contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = lib
CONFIG	+= static

INCLUDEPATH += .
INCLUDEPATH += ../adlParserSrc

MOC_DIR = moc
VPATH += ../adlParserSrc

HEADERS += XmlWriter.h \
    adlParserMain.h \
    QtProperties.h \
    dmsearchfile.h

SOURCES +=  XmlWriter.cpp parser.c \
    adlParserMain.cpp \
    QtProperties.c \
    dmsearchfile.cpp

TARGET = adlParser

OTHER_FILES += \
    stylesheet.qss


