include (../caQtDM.pri)

TARGET = caQtDM_Hipa
TEMPLATE = app

LIBS += -L$${QTCONTROLS} -Wl,-rpath,$${QTCONTROLS} -lqtcontrols -lcaQtDM_Lib_Hipa