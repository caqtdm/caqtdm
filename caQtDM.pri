include(qtdefs.pri)

contains(QT_VER_MAJ, 4) {
   QT     += core gui svg 
   CONFIG += qt warn_on thread uitools
}
contains(QT_VER_MAJ, 5) {
   QT     += core gui svg uitools  printsupport
   CONFIG += qt warn_on thread widgets
   DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000 
}   

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

