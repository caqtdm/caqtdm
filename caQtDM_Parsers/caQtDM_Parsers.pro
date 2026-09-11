#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.6.")
}
include (../caQtDM_Viewer/qtdefs.pri)

TEMPLATE = subdirs
# MSVC emits a .lib import library for the shared parsers, which would
# conflict with the static parser libraries of the same target name.
!win32-msvc*:!msvc {
    SUBDIRS += adlParserStaticLib prcParserStaticLib
}
!MOBILE:{
    SUBDIRS += adlParserSharedLib prcParserSharedLib
}
unix:{
    SUBDIRS += edlParserStaticLib
    !MOBILE:{
        SUBDIRS += edlParserSharedLib
    }

}
