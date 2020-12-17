#==========================================================================================================
#
#   Main caQtDM Components
#
#==========================================================================================================
caQtDM_Viewer {include(./caQtDM_BuildingFactory/caQtDM_Viewer.pri)}
caQtDM_Lib {include(./caQtDM_BuildingFactory/caQtDM_Library.pri)}
caQtDM_QtControls {include(./caQtDM_BuildingFactory/caQtDM_QtControls.pri)}
caQtDM_xdl2ui {include(./caQtDM_BuildingFactory/caQtDM_Parser.pri)}
#==========================================================================================================
#
#   caQtDM Data Sources
#
#==========================================================================================================
archive_plugin {include(./caQtDM_BuildingFactory/caQtDM_Plugin_ARCHIVE.pri)}
bsread_Plugin {include(./caQtDM_BuildingFactory/caQtDM_Plugin_BSREAD.pri)}
demo_plugin {include(./caQtDM_BuildingFactory/caQtDM_Plugin_DEMO.pri)}
environment_Plugin {include(./caQtDM_BuildingFactory/caQtDM_Plugin_Environment.pri)}
epics3_plugin {include(./caQtDM_BuildingFactory/caQtDM_Plugin_EPICS3.pri)}
epics4_plugin {include(./caQtDM_BuildingFactory/caQtDM_Plugin_EPICS4.pri)}
gps_plugin {include(./caQtDM_BuildingFactory/caQtDM_Plugin_GPS.pri)}
modbus_Plugin {include(./caQtDM_BuildingFactory/caQtDM_Plugin_MODBUS.pri)}
#==========================================================================================================
#
#   internal Libraries
#
#==========================================================================================================

include(./caQtDM_BuildingFactory/caQtDM_internal_Libraries.pri)

#==========================================================================================================
#
#   External Libraries
#
#==========================================================================================================
include(./caQtDM_BuildingFactory/caQtDM_Ext_EPICS.pri)
include(./caQtDM_BuildingFactory/caQtDM_Ext_PYTHON.pri)
include(./caQtDM_BuildingFactory/caQtDM_Ext_QWT.pri)
include(./caQtDM_BuildingFactory/caQtDM_Ext_ZMQ.pri)

#==========================================================================================================
#
#   Compiler/Linker Specials
#
#==========================================================================================================
include(./caQtDM_BuildingFactory/caQtDM_Compiler_Linker.pri)
#==========================================================================================================
#
#   Installation
#
#==========================================================================================================
include(./caQtDM_BuildingFactory/caQtDM_Installation.pri)
