#==========================================================================================================
#
#   Main caQtDM Components
#
#==========================================================================================================
include(./caQtDM_BuildingFactory/caQtDM_Viewer.pri)
include(./caQtDM_BuildingFactory/caQtDM_Library.pri)
include(./caQtDM_BuildingFactory/caQtDM_QtControls.pri)
include(./caQtDM_BuildingFactory/caQtDM_Parser.pri)

#==========================================================================================================
#
#   caQtDM Data Sources
#
#==========================================================================================================
include(./caQtDM_BuildingFactory/caQtDM_Plugin_ARCHIVE.pri)
include(./caQtDM_BuildingFactory/caQtDM_Plugin_BSREAD.pri)
include(./caQtDM_BuildingFactory/caQtDM_Plugin_DEMO.pri)
include(./caQtDM_BuildingFactory/caQtDM_Plugin_Environment.pri)
include(./caQtDM_BuildingFactory/caQtDM_Plugin_EPICS3.pri)
include(./caQtDM_BuildingFactory/caQtDM_Plugin_EPICS4.pri)
include(./caQtDM_BuildingFactory/caQtDM_Plugin_GPS.pri)
include(./caQtDM_BuildingFactory/caQtDM_Plugin_MODBUS.pri)

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

