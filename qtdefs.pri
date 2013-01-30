CAQTDM_VERSION = V2.6.4

# new in 2.6.4
# circulargauge was not taken warning limits into account. adl2ui has now also better defaults for this widget
# channel limits for stripplot were not properly displayed, a refresh had to be done
# reload will now reload all windows from files
# adl2ui: messagebutton per default has fontscalemode height; slider  got foreground and background
# when no write access, cursor will change, but widget will not be disabled any more but action is prevented
# cartesianplot got dialog in order to change its scale
# cartesianplot will set scale to auto when channel was specified but no channel is present; adl2ui will not use the second y axis
# slider could have the wrong orientation, should be fixed; gets now also correct colors with adl2ui
# messagebutton got more shade, carowcolmenu also
# default for cachoice is centered text, colors should be correct
# epushbutton scales its text when changing scalemode
# caQtDM will exit when closing last window; it will not ask any more with timeout

# new in 2.6.3
# fixed memory overflows due to non null terminated strings from epics

# new in 2.6.2
# about icon will present our application epics icon
# command line parameter -dg works also in attach mode
# title bar now yust showing the filename and can be overwritten by other applications without problem
# &X implemented in shell command (only for linux)

# new in 2.6.1
# command line parameter -dg implemented, works only in standalone mode (not when attaching)

# new in 2.6
# textentry will get its contents back if contents was changed without pressing return when leaving textentry
# green alarm color has been changed to a darker green in order to have a better contrast
# adl2ui got the option to suppress the legends for the stripplot, valuescale will be generated instead of timescale
# cartesian plot was enhanced for single scalar values with only one channel
# special arguments &A and &T were implemented in shell commands
# cabitnames, cachoice, cashellcommand : possibility to choose the font and size, buttons will be also separated by 2 pixels
# in case of static visibility, channels will be ignored.

win32 {
QWTHOME=C:/Qwt-6.0.1/Qwt

# location of QtControls 
QTCONTROLS_LIBS=C:/work/QtControls/qtcontrols/release
QTCONTROLS_INCLUDES=C:/work/QtControls/qtcontrols/src

# location where the plugins will be copied
PLUGINS_LOCATION=C:\QtSDK\Desktop\Qt\4.8.1\mingw\plugins\designer
BINARY_LOCATION=C:\QtSDK\Desktop\Qt\4.8.1

# location of epics 
EPICS_LIBS=C:/epics/base-3.14.12/lib/win32-x86-mingw
EPICS_BASE = C:/epics/base-3.14.12

# location of the caQtDM_Lib 
CAQTDM_LIBS=C:/work/caQtDM_Lib/release
CAQTDM_INCLUDES=C:/work/caQtDM_Lib/src
}

unix {
# defined by environment variables
}

