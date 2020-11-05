TARGET_PRODUCT = "android functions"

include(../caQtDM_Viewer/qtdefs.pri)
CONFIG += caQtDM_AndroidFunctions
include(../caQtDM.pri)

OBJECTS_DIR = obj

CONFIG += staticlib
CONFIG += release
DESTDIR = $(CAQTDM_COLLECT)

TARGET = AndroidFunctions
TEMPLATE = lib
INCLUDEPATH += src

SOURCES	+= src/androidtimeb.c 
HEADERS += src/androidtimeb.h

message("functions for android")


