CAQTDM_TOP = ..
TARGET_PRODUCT = "Display Manager"
TARGET_FILENAME = "caqtdm.exe"

include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
CONFIG += caQtDM_Viewer
CONFIG += caQtDM_Installation
CAQTDM_INSTALL_LIB = caQtDM
include($$CAQTDM_TOP/caQtDM.pri)

contains(QT_VER_MAJ, 4) {
   QT     += core gui svg network
   CONFIG += qt warn_on thread uitools
}
contains(QT_VER_MAJ, 5) {
   QT     += core gui svg uitools network opengl
   modbus{
      QT += serialbus
      DEFINES += CAQTDM_MODBUS
   }
   gps{
      QT += positioning
      DEFINES += CAQTDM_GPS
   }

   !ios:!android {
       QT     += printsupport
    }
# x11 extras must be included for qt5.1 and higher
unix:!macx:!ios:!android{
      greaterThan(QT_VER_MIN,0){
         QT += x11extras
     }
   }
   CONFIG += qt warn_on thread widgets
   DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
}

TARGET = caQtDM
TEMPLATE = app

DEFINES += SUPPORT=\\\"EPICS\\\"

INCLUDEPATH += ./src

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}

RESOURCES += ./src/caQtDM.qrc
RC_FILE = ./src/caQtDM.rc

SOURCES +=\
    src/caQtDM.cpp \
    src/fileopenwindow.cpp \
    src/messagebox.cpp \
    src/configDialog.cpp \
    src/pipereader.cpp

HEADERS  +=  \
    src/messagebox.h \
    src/fileopenwindow.h \
    src/configDialog.h \
    src/pipereader.h

FORMS += src/main.ui

# when epics4 not build, then do not import statically in fileopenwindow.cpp
epics4 {
 DEFINES +=  EPICS4
}

# had to take the image formats out for building ios now. However images still work?!!
ios {
QTPLUGIN.imageformats=-
}

android {
DISTFILES += \
    src/Android/AndroidManifest.xml \
    src/Android/gradle/wrapper/gradle-wrapper.jar \
    src/Android/gradle/wrapper/gradle-wrapper.properties \
    src/Android/gradlew \
    src/Android/gradlew.bat
}

