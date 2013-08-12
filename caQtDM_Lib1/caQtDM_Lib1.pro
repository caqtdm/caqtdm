include (../caQtDM_Lib.pri)

TARGET = caQtDM_Lib
OBJECTS_DIR = ../obj1
TEMPLATE = lib

QMAKE_POST_LINK = cp ../libcaQtDM_Lib.so $(QTBASE)




