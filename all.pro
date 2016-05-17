_QTHOME=$$(QTHOME)
_QWTHOME=$$(QWTHOME)
_EPICSBASE=$$(EPICS_BASE)
_EPICSHOSTARCH=$$(EPICS_HOST_ARCH)

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
 message("EPICS located in $$_EPICSHOSTARCH")
}

#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.6.")
}

TEMPLATE = subdirs
SUBDIRS = caQtDM_QtControls caQtDM_Lib caQtDM_Viewer
SUBDIRS += qtcontrols_controllers qtcontrols_graphics qtcontrols_monitors 
SUBDIRS += caQtDM_Plugins

qtcontrols_controllers.file = caQtDM_QtControls/plugins/qtcontrols_controllers.pro 
qtcontrols_controllers.depends = caQtDM_QtControls caQtDM_Lib

qtcontrols_graphics.file = caQtDM_QtControls/plugins/qtcontrols_graphics.pro 
qtcontrols_graphics.depends = caQtDM_QtControls caQtDM_Lib

qtcontrols_monitors.file = caQtDM_QtControls/plugins/qtcontrols_monitors.pro 
qtcontrols_monitors.depends = caQtDM_QtControls caQtDM_Lib

caQtDM_Plugins.file = caQtDM_Lib/caQtDM_Plugins/csplugins.pro 
caQtDM_Plugins.depends = caQtDM_Lib

!ios {
!android {
   SUBDIRS += parser
   parser.file = caQtDM_Viewer/parser/parser.pro
}
}

unix {
!ios {
!android {
   SUBDIRS +=  parserEDM
   parserEDM.file = caQtDM_Viewer/parserEDM/parserEDM.pro
}
}
}

caQtDM_Viewer.depends = caQtDM_QtControls caQtDM_Lib qtcontrols_controllers qtcontrols_graphics qtcontrols_monitors caQtDM_Plugins
caQtDM_Lib.depends = caQtDM_QtControls


