include(../qtdefs.pri)
CONFIG += caQtDM_xdl2ui
include(../../caQtDM.pri)

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

#LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -ledlParser
LIBS += $(CAQTDM_COLLECT)/libedlParser.a
OTHER_FILES += \
    stylesheet.qss

