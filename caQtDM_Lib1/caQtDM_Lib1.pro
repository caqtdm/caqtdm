include (../caQtDM_Lib.pri)

TARGET = caQtDM_Lib
OBJECTS_DIR = ../obj1
TEMPLATE = lib

QMAKE_POST_LINK = cp ../libcaQtDM_Lib.so $(QTBASE)

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
DESTDIR = ../
UI_DIR += ../
INCLUDEPATH += ../../caQtDM_QtControls/src



