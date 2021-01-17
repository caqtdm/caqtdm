
# put in file 5.13.1/ios/mkspecs/features/toolchain.prf  after line 184 darwin {
# cxx_flags += -arch $$QMAKE_APPLE_DEVICE_ARCHS otherwise nothing works for ios 14
# even parsing of the .pro files fails
CAQTDM_TOP=.
_QTHOME=$$(QTHOME)
_QWTHOME=$$(QWTHOME)
_EPICSBASE=$$(EPICS_BASE)
_EPICSHOSTARCH=$$(EPICS_HOST_ARCH)

include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)

isEmpty(_QTHOME){
 message("QTHOME must be defined in order to locate QT")
 error(please define QTHOME.)
}else{
 message("QT located in $$_QTHOME")
}

isEmpty(_QWTHOME){
 message("QWTHOME must be defined in order to locate QWT")
 error(please define QWTHOME.)
}else{
 message("qwt located in $$_QWTHOME")
}

isEmpty(_EPICSBASE){
 message("EPICS_BASE must be defined in order to use EPICS")
 error(please define EPICS_BASE.)
}else{
 message("EPICS located in $$_EPICSBASE")
}

isEmpty(_EPICSHOSTARCH){
 message("EPICS_HOST_ARCH must be defined in order to use EPICS")
 error(please define EPICS_HOST_ARCH.)
}else{
 message("EPICS_HOST_ARCH located in $$_EPICSHOSTARCH")
}

#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.6.")
}

TEMPLATE = subdirs
SUBDIRS = caQtDM_QtControls caQtDM_Lib caQtDM_Viewer
SUBDIRS += caQtDM_Designer_Plugins
SUBDIRS += caQtDM_Plugins

android {
  SUBDIRS += caQtDM_AndroidFunctions
}

caQtDM_Designer_Plugins.depends = caQtDM_QtControls

caQtDM_Plugins.file = caQtDM_Lib/caQtDM_Plugins/csplugins.pro 
caQtDM_Plugins.depends = caQtDM_Lib

!MOBILE {
   SUBDIRS +=  caQtDM_Parsers parser
   unix:{
        SUBDIRS += parserEDM
   }
   parser.file = caQtDM_Viewer/parser/parser.pro
   parser.depends = caQtDM_Parsers
   parserEDM.file = caQtDM_Viewer/parserEDM/parserEDM.pro
   parserEDM.depends = caQtDM_Parsers
   ADL_EDL_FILES {
      caQtDM_Lib.depends += caQtDM_Parsers
      caQtDM_QtControls.depends += caQtDM_Parsers
    }

}

caQtDM_Viewer.depends = caQtDM_QtControls caQtDM_Lib caQtDM_Designer_Plugins
#beccause of building a bundle
macx:{
    caQtDM_Viewer.depends += caQtDM_Plugins
}



caQtDM_Lib.depends += caQtDM_QtControls



android:!macx {
  caQtDM_Lib.depends += caQtDM_AndroidFunctions
}

archiveCA.depends = caQtDM_QtControls caQtDM_Lib
archivePRO.depends = caQtDM_QtControls caQtDM_Lib
archiveSF.depends = caQtDM_QtControls caQtDM_Lib
archiveHIPA.depends = caQtDM_QtControls caQtDM_Lib

#iosArchitectures=armv7
#iosTarget=5.0

# Set "Architectures"
#QMAKE_IOS_DEVICE_ARCHS = $$iosArchitectures

# Set "Target"
#QMAKE_IOS_DEPLOYMENT_TARGET = $$iosTarget

# Set "Devices" (1=iPhone, 2=iPad, 1,2=Universal)
#QMAKE_IOS_TARGETED_DEVICE_FAMILY = 2

