CAQTDM_TOP = ..
TARGET_PRODUCT = "android functions"

include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
CONFIG += caQtDM_AndroidFunctions
include($$CAQTDM_TOP/caQtDM.pri)

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


