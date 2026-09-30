#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.6.")
}
include (../caQtDM_Viewer/qtdefs.pri)

TEMPLATE = subdirs
# The ADL static library is linked by caQtDM_xdl2ui_Lib on Windows.
SUBDIRS += adlParserStaticLib
# MSVC emits prcParser.lib as the shared PRC parser's import library,
# which conflicts with the static library of the same target name.
!win32-msvc*:!msvc {
    SUBDIRS += prcParserStaticLib
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
