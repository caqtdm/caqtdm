include(../qtdefs.pri)

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
OBJECTS_DIR = obj
DESTDIR = $(CAQTDM_COLLECT)

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

 DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
 DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"


CONFIG += console
 ReleaseBuild {
   OBJECTS_DIR = release/obj
 }
 DebugBuild {
   DESTDIR = $(CAQTDM_COLLECT)/debug
   OBJECTS_DIR = debug/obj
 }



