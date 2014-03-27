include (../caQtDM_Lib.pri)

TARGET = caQtDM_Lib_Hipa
OBJECTS_DIR = ../obj2
TEMPLATE = lib

SOURCES += acsSubs.c medmblock.c
HEADERS += acsSubs.h

LIBS += -L$(ACS_BUILD_LIBRARY) -lDEV -lCDB -lInclude -lProfAcc
INCLUDEPATH += $(ACS_BUILD_INCLUDE)
DEFINES += ACS

QMAKE_POST_LINK = cp ../libcaQtDM_Lib_Hipa.so $(QTBASE)

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
DESTDIR = ../
UI_DIR += ../
INCLUDEPATH += ../../caQtDM_QtControls/src
