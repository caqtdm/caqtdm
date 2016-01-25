TARGET_PRODUCT = "Display Manager"
TARGET_FILENAME = "caqtdm.exe"

include(qtdefs.pri)
CONFIG += caQtDM_Viewer
include(../caQtDM.pri)

contains(QT_VER_MAJ, 4) {
   QT     += core gui svg network
   CONFIG += qt warn_on thread uitools
}
contains(QT_VER_MAJ, 5) {
   QT     += core gui svg uitools  printsupport network opengl
   CONFIG += qt warn_on thread widgets
   DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000 
} 

TARGET = caQtDM
TEMPLATE = app

DEFINES += SUPPORT=\\\"EPICS\\\"

MOC_DIR = ./moc
VPATH += ./src
INCLUDEPATH += ./src
UI_DIR += ./src

INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += ../caQtDM_QtControls/src
INCLUDEPATH += ../caQtDM_Lib/src
INCLUDEPATH += ../caQtDM_Lib/caQtDM_Plugins

RESOURCES += ./src/caQtDM.qrc
RC_FILE = ./src/caQtDM.rc

SOURCES +=\
    caQtDM.cpp \
    fileopenwindow.cpp \
    messagebox.cpp \
    configDialog.cpp

HEADERS  +=  \
    messagebox.h \
    fileopenwindow.h \
    configDialog.h

FORMS += main.ui



