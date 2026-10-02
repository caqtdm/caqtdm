include(../../caQtDM_Viewer/qtdefs.pri)
CONFIG += caQtDM_xdl2ui
include(../../caQtDM.pri)

QT = core
contains(QT_VER_MAJ, 5) {
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = lib
CONFIG	+= static
# the static lib is linked into libqtcontrols.so on linux
unix:!macx: QMAKE_CXXFLAGS += -fPIC

INCLUDEPATH += .
INCLUDEPATH += ../alhParserSrc

MOC_DIR = moc
VPATH += ../alhParserSrc

HEADERS += alhmodel.h \
    alhconfigparser.h \
    alhuigenerator.h \
    alhparserdefs.h

SOURCES += alhmodel.cpp \
    alhconfigparser.cpp \
    alhuigenerator.cpp

TARGET = alhParser
