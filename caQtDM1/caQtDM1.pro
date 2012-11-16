include (../caQtDM.pri)

TARGET = caQtDM
TEMPLATE = app

LIBS += -L$${QTCONTROLS} -Wl,-rpath,$${QTCONTROLS} -lqtcontrols -lcaQtDM_Lib