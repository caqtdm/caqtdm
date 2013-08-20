include(../qtdefs.pri)

contains(QT_VER_MAJ, 5) {
  QT       += widgets
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TEMPLATE = app
INCLUDEPATH += .

MOC_DIR = moc
OBJECTS_DIR = obj
DESTDIR = .

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

unix {
  QMAKE_POST_LINK = cp adl2ui ../../caQtDM_Binaries/
}

win32-msvc* {
 ReleaseBuild {
   QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\adl2ui.exe ..\..\caQtDM_Binaries
   OBJECTS_DIR = release/obj	
 }
 DebugBuild {
   OBJECTS_DIR = debug/obj
 }

}
