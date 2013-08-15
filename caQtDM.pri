include(qtdefs.pri)

QT       += core gui svg
CONFIG      += qt warn_on thread uitools

unix: {
  CONFIG += x11
}

SOURCES +=\
    caQtDM.cpp \
    fileopenwindow.cpp \
    messagebox.cpp

HEADERS  +=  \
    messagebox.h \
    fileopenwindow.h \


FORMS += main.ui


unix:{
	DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
	DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"
}
win32{
	DEFINES += BUILDTIME=\\\"\\\"
	DEFINES += BUILDDATE=\\\"\\\"
	DEFINES += SUPPORT=\\\"\\\"
}
DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"

RESOURCES += src/caQtDM.qrc
RC_FILE = src/caQtDM.rc
