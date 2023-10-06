#version check qt
CAQTDM_TOP=..
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.6.")
}
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)

TEMPLATE = subdirs
SUBDIRS += adlParserStaticLib
!MOBILE:{
    SUBDIRS += adlParserSharedLib
}
unix:{
    SUBDIRS += edlParserStaticLib
    !MOBILE:{
        SUBDIRS += edlParserSharedLib
    }

}
