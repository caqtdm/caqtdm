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
   QT     += core gui svg uitools network opengl
   modbus{
      QT += serialbus
      DEFINES += CAQTDM_MODBUS
   }
   gps{
      QT += positioning
      DEFINES += CAQTDM_GPS
   }
   bleacon{
      !ios {
         QT += bluetooth
      }
      DEFINES += CAQTDM_BLEACON
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

contains(QT_VER_MAJ, 6) {
    QT     += core gui svg uitools network opengl xml
    CONFIG += qt warn_on thread widgets
    modbus{
       QT += serialbus
       DEFINES += CAQTDM_MODBUS
    }
    gps{
       QT += positioning
       DEFINES += CAQTDM_GPS
    }
    bleacon{
       !ios {
          QT += bluetooth
       }
       DEFINES += CAQTDM_BLEACON
    }
    opcua {
       MOBILE {
       QT += opcua
       }
       DEFINES += CAQTDM_OPCUA
    }

    !ios:!android {
        QT     += printsupport
     }

    android {
        QT += xml
    }

}



TARGET = caQtDM
TEMPLATE = app

DEFINES += SUPPORT=\\\"EPICS\\\"

MOC_DIR = ./moc
VPATH += ./src
INCLUDEPATH += ./src
UI_DIR += ./src

INCLUDEPATH += $$(QWTINCLUDE)
INCLUDEPATH += ../caQtDM_QtControls/src
INCLUDEPATH += ../caQtDM_Lib/src
INCLUDEPATH += ../caQtDM_Plugins

RESOURCES += ./src/caQtDM.qrc
RC_FILE = ./src/caQtDM.rc

SOURCES +=\
    caQtDM.cpp \
    fileopenwindow.cpp \
    messagebox.cpp \
    configDialog.cpp \
    pipereader.cpp

HEADERS  +=  \
    messagebox.h \
    fileopenwindow.h \
    configDialog.h \
    pipereader.h \
    src/loggingcategories.h

CAQTDM_NO_CUSTOM_LOGHANDLER = $$(CAQTDM_NO_CUSTOM_LOGHANDLER)
isEmpty(CAQTDM_NO_CUSTOM_LOGHANDLER) {
    message("Building custom caQtDM LogHandler")

    SOURCES +=\
        src/logging/consoleloghandler.cpp \
        src/logging/fileloghandler.cpp \
        src/logging/generalloghandler.cpp \
        src/logging/logstashloghandler.cpp

    HEADERS +=\
        src/logging/abstractloghandler.h \
        src/logging/consoleloghandler.h \
        src/logging/fileloghandler.h \
        src/logging/generalloghandler.h \
        src/logging/logstashloghandler.h

    unix {
        SOURCES += src/logging/syslogloghandler.cpp
        HEADERS += src/logging/syslogloghandler.h
    }
} else {
    message("caQtDM will be built without custom LogHandler")

    DEFINES += CAQTDM_NO_CUSTOM_LOGHANDLER
}

FORMS += main.ui

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

