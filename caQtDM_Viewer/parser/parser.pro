include(../qtdefs.pri)
CONFIG += caQtDM_xdl2ui
include(../../caQtDM.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = app
INCLUDEPATH += .
INCLUDEPATH += ../../caQtDM_Parsers/adlParserSrc
MOC_DIR = moc

# Input
HEADERS += adlParser.h
SOURCES += adlParser.cpp

TARGET = adl2ui

LIBS += $(CAQTDM_COLLECT)/libadlParser.a

OTHER_FILES += \
    stylesheet.qss


