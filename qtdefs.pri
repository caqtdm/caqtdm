CAQTDM_VERSION = V3.4.2

QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

# new in 3.4.2
# camera stuff modified
# added mutex to synchronize data and display
# font bug in catable solved
# found a datarace in castripplot, corrected

# new in 3.4.1
# cacamera displays intensity at x, y
# cacamera context menu for grey / spectrum and zoom

# new in 3.4.0
# caStripplot has been revisited
# caCamera uses now concurrent mode for calculations

# new in 3.3.1
# mainwindow and messagewindow have been merged

# new in 3.3.0
# caTable gets now the correct font after resizing when values update
# caStripPlot has been corrected for incorrect time behaviour.

# new in 3.2.2
# in menu you have the possibility to raise the main and message windows

# new in 3.2.1
# when requesting a display that was already started, it did not popup but made a new instance. this has been corrected
# this was working before, but by some modifications was not correct any more

# new in 3.2.0
# build mechanism for linux and windows should do on most platforms, see caQtDM_README
# caQtDM builds for qt4.8 with qwt6.0 and for qt5 with qwt6.1

# new in 3.1.4
# caled can use now the severity to colorize
# cathermo was inadverttently using nodeco mode when mode was used
# cacirculargauge will draw a white scale when not connected

# new in 3.1.3
# modified treatment of format for pep file lineedit; html code is also possible for text

# new in 3.1.2
# bug corrected for the circular gauge
# for enums without enum string write just the value in calineedit
# pep files: font changed, background changed and implemented the possibility to change colors for printing

# new in 3.1.1
# stripplot and cartesian plot will rescale their fontsizes now too, caSlider and caThermo text will only down scale

# new in 3.1.0
# windows are resizable, in order to have to working well one should always use fontscalemode=WidthAndHeight. in the deafult style sheet no font sizes should be specified.
# when for the mainwindow a layout is used, Qt should do the work and caQtDM will not do anything except changing some font sizes.
# if on the command line -noResize is specified, then resizing will be disabled for that instance of caQtDM (in case of attaching that will not work)
# adl2ui will generate all texts with fontScaleMode=WidthAndHeight
# caChoice will not display a number for empty items with enum
# caLabel, calinedit will have per default fontscalemode=WidthAndHeight
# caRelatedDisplay, caShellCommand modified to be correctly scalable
# simplelabel modified to change font during typing
# fontscalingwidget algorithme modified

# new in 3.0.1
# one can specify building with epics4 (in caQtDM_LIB.pri) ; this first attempt allows to read and write scalars. epics4 channel have to be specified
# with @ in front of the name
# print is possible on the command line; will print specified file and exit

# new in 2.8.4
# one can use up down arrows for the wheelswitch after clicking on the digit to change
# bug corrected for digital io in acs

# new in 2.8.3
# added a doubletabwidget: gives the possibility to have two-dimensional choices

# new in 2.8.2
# bug in cartesian plot for scalar, vector has been removed

# new in 2.8.1
# small changes in pep file handling

# new in 2.8.0
# performance has been optimized further
# visibility of image did not work
# softpv had some problems, some solved
# for old acs controlsystem, problem with command has been solved
# flashing due to hide and show eliminated for capolyline and cagraphics
# color performance problem for calabel solved
# epics alarm status has been added in context box
# info could not be called for polyline; is now ok
# when opening pep files, size of window is resized to a minimum
# adl2ui some strings leaded to an array overflow; is now ok

# new in 2.7.3
# calineedit was changing its stylesheet continuously and had to be optimized for that
# on windows terminate process with the kill button was hanging for 30 seconds and is now doing right.
# filedialog box will now use files .ui and .prc
# added a control cascriptbutton for script execution, one can write a script and follow its execution

# new in 2.7.2
# a new control has been added: caToggleButton writing 0 or 1 to a channel
# psi .prc files can be used in cainclude and can also be called directly like an ui file. file parsing and treatment are not yet finished for all cases
# in camenu the menu items are now also non selectable when no write access

# new in 2.7.1
# lucida sans typewriter was not always taken, even when present. fallback to monospace had to be relaxed.
# the filename for cainclude, can be a macro now. this allows to have a generic ui file loading some include defined by macro

# new in 2.7.0
# bug corrected in stylesheet of catextentry
# stripplot can have now up to 7 curves

# new in 2.6.8
# null character was not written for array of chars in case of string, is now corrected
# catextentry and calineedit style were slightly changed in order to get bigger characters and no shift of string in case of entering a catextentry,
#  these widgets use the font "lucida sans typewriter". When this font does not exist, they will fall back to a monospace font, however the monospace font
#  present a dot inside the zero, which is not very nice. If you do not like it, change the font instruction in calinedit
# in case of a related display with removing of the parent, the parents position will be used for positioning of the new display

# new in 2.6.7
# writing through catextentry is not limited to 40 characters, when specifying string format it writes a string through char array
# catextentry has now an inset look and the styles of calineedit and catextentry through code and not by stylesheet
# castripplot is limited to five curves and will be extended to more, but nut yet done
# minimum size generated by adl2ui for a cathermo is 3 pixels wide or high
# the items of camenu were always disabled, is now fixed
# clearfocus after return in catextentry has been taken out.

# new in 2.6.6
# related display got the possibility to specify if the parent display has to be removed
# setting a dbf_char had a bug and was not set. Should be fixed

# new in 2.6.5
# for the stripplot adl2ui will not generate any more the first curve as filled when a '[' is seen in the x label
# the pipe width of caslider and cathermo are set by adl2ui to the width or height when no decorations
# cabyte was not setting correctly the alarm colors, is fixed now
# caslider is set now to readonly when no write access
# cathermo had geometry update problems, fixed
# camenu does not accept background and foreground colors when Qt style is windows. now qt applications are started with -style plastique fixing the problem
# on 64bit architectures, waveforms of type epics long were processed with long of 64 bits. now fixed using 32bits

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


