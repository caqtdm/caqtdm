include(../qtdefs.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = app
INCLUDEPATH += .

MOC_DIR = moc
OBJECTS_DIR = obj
DESTDIR = $(CAQTDM_COLLECT)

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


