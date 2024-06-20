CAQTDM_VERSION = V4.4.2

exists(../.git) {
  GIT_VERSION = $$system(git --version)
  contains(GIT_VERSION, "git") :{
	  CAQTDM_GIT_VERSION = $$system(git rev-parse --abbrev-ref HEAD)
	  CAQTDM_GIT_COMMAND = $$sprintf("git rev-parse --short=8 origin/%1", $$CAQTDM_GIT_VERSION)
	  CAQTDM_GIT_HASH = $$system($$CAQTDM_GIT_COMMAND)

	  contains(CAQTDM_GIT_VERSION, "Development") {
	     CAQTDM_VERSION = $$sprintf("%1_%2_%3", $$CAQTDM_VERSION, $$CAQTDM_GIT_VERSION, $$CAQTDM_GIT_HASH)
	  }
	  #message("$$CAQTDM_GIT_VERSION")
	  #message("$$CAQTDM_GIT_COMMAND")
	  #message("$$CAQTDM_GIT_HASH")
   }
}

#message($$CAQTDM_VERSION)

QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)
QT_VER_PAT = $$member(QT_VERSION, 2)


QWTVERSION = $$(QWTVERSION)
QWTVERSION = $$split(QWTVERSION, ".")
QWT_VER_MAJ = $$member(QWTVERSION, 0)
QWT_VER_MIN = $$member(QWTVERSION, 1)

message ("Qt $$[QT_VERSION] QWT $$(QWTVERSION)")


TARGET_COMPANY = "Paul Scherrer Institut"
TARGET_DESCRIPTION = "Channel Access Qt Display Manager"
TARGET_COPYRIGHT = "Copyright (C) 2012-2024 Paul Scherrer Institut"
TARGET_INTERNALNAME = "caqtdm"

# enable opengl in stripplot and cartesianplot (edo not use, experimental only, for Qt5 and qwt6.1)

contains(QT_VER_MAJ, 5) {
#     message(opengl requested)
#     DEFINES += QWT_USE_OPENGL
}

unix {
    QMAKE_CXXFLAGS += "-g"
    QMAKE_CFLAGS_RELEASE += "-g"
}


# Set the overall Deployment Target for MACOSX
QMAKE_MACOSX_DEPLOYMENT_TARGET = 14.0

# at psi the designer in 4.8.2 is patched in order to display tooltip description (not a nice test, but for now ok)
# when the qt version is higher then 5.5.0 then we can also compile the plugins with description texts
# be carefull with this, while when the designer does not recognize tooltip descriptions, the widgets will not be shown

contains(QT_VER_MAJ, 4) {
    contains(QT_VER_MIN, 8) {
       contains(QT_VER_PAT, 2) {
          message("caQtDM building with Qt4.8.2, therefore enabling designer tooltip description, this works only for the patched version of Qt4.8.2 at PSI")
          DEFINES += DESIGNER_TOOLTIP_DESCRIPTIONS
       }
    }
}

contains(QT_VER_MAJ, 5) {
  greaterThan(QT_MINOR_VERSION, 5) {
    DEFINES += DESIGNER_TOOLTIP_DESCRIPTIONS 
  }
}

contains(DEFINES, DESIGNER_TOOLTIP_DESCRIPTIONS ) {
  message("Building with tooltip descriptions; if not supported by designer, turn it off in qtdefs.pri")
}

# enable specialized version, wehere files will be downloaded to a local directory (used specially for IOS and Android)
ios | android {
 DEFINES += MOBILE
 CONFIG += MOBILE
}
ios {
  DEFINES += MOBILE_IOS
  QTPLUGIN += qjpeg qgif
  QMAKE_CXXFLAGS_WARN_ON = -w
}
android {
  DEFINES += MOBILE_ANDROID
  QMAKE_CXXFLAGS_WARN_ON = -w
}
macx {
  QMAKE_CXXFLAGS_WARN_ON = -w
}

# for some architectures this has to be defined for scan2D
macx | win32 | ios | android {
DEFINES += XDR_HACK
DEFINES += XDR_LE
CONFIG += XDR_HACK
}
# enable XDR_HACK test for Linux/Qt5.10 e.g. Fedora 28
unix:!macx:{
 contains(QT_VER_MAJ, 5) {
   greaterThan(QT_VER_MIN,9){
    DEFINES += XDR_HACK
    DEFINES += XDR_LE
    CONFIG += XDR_HACK
   }
 }
}

# enable ADL_EDL automatic on the fly conversion, if not enabled, when an adl file is encountered,
# an ui file is assumed
!MOBILE: {
    CONFIG += ADL_EDL_FILES
    DEFINES += ADL_EDL_FILES
}


# undefine CONFIG epics7 for epics4 plugin support with epics version 7 (only preliminary version as example)
# one can specify channel access with ca:// and pv access with pva:// (both use the epics4 plugin)
# the main work for this plugin was done by Marty Kraimer
exists($(EPICSINCLUDE)/pv/pvAccess.h) {
    #a special thing for PSI build
    eval($$(EPICS_HOST_ARCH)=RHEL7-x86_64){
        CONFIG += CAQTDM_PSI_SPECIAL_EPICS7_C11
    }
    CONFIG += epics7
    epics7 {
       message( "Configuring build for epics4 plugin with epics7" )
       CONFIG += epics4
    }
}

_CAQTDM_MODBUS = $$(CAQTDM_MODBUS)
isEmpty(_CAQTDM_MODBUS) {
message("Modbus Plugin will not be build")
}
else {
    greaterThan(QT_VER_MAJ, 4) {
        CONFIG += modbus
        modbus {
           message( "Configuring build for modbus plugin" )
        }
    }
}

_CAQTDM_GPS = $$(CAQTDM_GPS)
isEmpty(_CAQTDM_GPS) {
message("GPS Plugin will not be build")
}
else {
    CONFIG += gps
    gps {
      message( "Configuring build for GPS plugin" )
    }
}

# undefine CONFIG epics4 for epics4 plugin support with epics version 4 (only preliminary version as example)
# one can specify channel access with ca:// and pv access with pva:// (both use the epics4 plugin)
# the main work for this plugin was done by Marty Kraimer
!epics7 {
   exists($(EPICS4LOCATION)/pvAccessCPP/include/pv/pvAccess.h) {
   !MOBILE {
      message( "Configuring build for epics4 plugin with epics4" )
      CONFIG += epics4
   }
   CONFIG += EPICS4_STATICBUILD
   }
}

# undefine this to make the ca provider from pvAccess (epics4) the default provider
# otherwise the ca provider from epics3 base is the default provider
#DEFINES += PVAISDEFAULTPROVIDER

contains(QT_VER_MAJ, 4) {
    contains(QT_VER_MIN, 6) {
       message( "version 4.6 of Qt" )
       CONFIG += OLDQT
    }
}

# undefine this for bsread (zeromq) plugin support
# the main work for this plugin was done by Helge Brands
# will not be build for older version of Qt
#!OLDQT {
  exists($(ZMQINC)/zmq.h) {
    !MOBILE {
       message( "Configuring controlsystem plugin for bsread" )
       CONFIG += bsread
    }
  }
#}

defineTest(existFiles) {
    files = $$ARGS

    for(file, files) {
        !exists($$file) {
            return(false)
        }
    }
    return(true)
}

# undefine this for archive retrieval plugin support (these plugins are only valid at psi)
# take a look at the archiveSF in order to do something similar
CONFIG += archive
archive: {
# http retrieval, can always be build
   CONFIG += archiveSF
   CONFIG += archiveHTTP
# next ones are only buildable at psi

QMAKESPEC = $$(QMAKESPEC)
X64 = $$find(QMAKESPEC, 64)

isEmpty(X64) {
       exists($(CAQTDM_LOGGING_ARCHIVELIBS)/libNewLogRPC.a) {
          message( "Configuring archive plugin build for logging (32)" )
          CONFIG += archiveHIPA
          CONFIG += archivePRO
       } else {
          warning("library libNewLogRPC.a not found, archive plugin will not be build for logging (32)" )
       }

       allFiles = $(CAQTDM_CA_ARCHIVELIBS)/Storage/libStorage_32.a $(CAQTDM_CA_ARCHIVELIBS)/Tools/libTools_32.a $(CAQTDM_CA_ARCHIVELIBS)/xerces-c-3.1.4/libxerces-c_32.a
       existFiles($$join(allFiles, " ")) {
         message( "Configuring archive plugin build for CA (32)" )
         CONFIG += archiveCA
       } else {
          warning("some library (libStorage_32.a or libTools_32.a or ibxerces-c_32.a) for ca_archiver not found, ca_archive plugin will not be build" )
       }
    } else {
       exists($(CAQTDM_LOGGING_ARCHIVELIBS)/libNewLogRPC_64.a) {
          message( "Configuring archive plugin for logging (64)" )
          CONFIG += archiveHIPA
          CONFIG += archivePRO
       } else {
          warning("library libNewLogRPC_64.a not found, archive plugin will not be build for logging (64)" )
       }

       allFiles = $(CAQTDM_CA_ARCHIVELIBS)/Storage/libStorage_64.a $(CAQTDM_CA_ARCHIVELIBS)/Tools/libTools_64.a $(CAQTDM_CA_ARCHIVELIBS)/xerces-c-3.1.4/libxerces-c_64.a
       existFiles($$join(allFiles, " ")) {
          message( "Configuring archive plugin for CA (64)" )
          CONFIG += archiveCA
       } else {
          warning("some library (libStorage_64.a or libTools_64.a or ibxerces-c_64.a) for ca_archiver not found, ca_archive plugin will not be build" )
       }
    }
}

# in fileopenwindow we need to import these plugins for ios and android, so define them
ios | android {
bsread: { DEFINES += BSREAD }
epics4: { DEFINES += EPICS4 }
archiveSF: { DEFINES += ARCHIVESF }
archiveHIPA: { DEFINES += ARCHIVEHIPA }
archivePRO: { DEFINES += ARCHIVEPRO }
archiveCA: { DEFINES += ARCHIVECA }
}

# undefine this when you need to combine caQtDM with the australian epicsqt package
#CONFIG += australian

# we can add python for calculations cacalc & visibility
# include definitions and libraries are defined in caQtDM_Lib.pri for linux and macos
CONFIG += PYTHONCALC

# undefine this in order not to disable monitors for hidden pages of QTabWidgets
DEFINES += IO_OPTIMIZED_FOR_TABWIDGETS

# Defines for Windows file Information
DEFINES += TARGET_PRODUCT=\"\\\"$${TARGET_PRODUCT}\\\"\"
DEFINES += TARGET_FILENAME=\"\\\"$${TARGET_FILENAME}\\\"\"

_TARGET_VERSION = $$replace(CAQTDM_VERSION, V, '')
_TARGET_VERSION = $$split(_TARGET_VERSION, ".")
DEFINES += TARGET_VER_MAJ=$$member(_TARGET_VERSION, 0)
DEFINES += TARGET_VER_MIN=$$member(_TARGET_VERSION, 1)
DEFINES += TARGET_VER_BUILD=$$member(_TARGET_VERSION, 2)

DEFINES += TARGET_COMPANY=\"\\\"$${TARGET_COMPANY}\\\"\"
DEFINES += TARGET_DESCRIPTION=\"\\\"$${TARGET_DESCRIPTION}\\\"\"
DEFINES += TARGET_COPYRIGHT=\"\\\"$${TARGET_COPYRIGHT}\\\"\"
DEFINES += TARGET_INTERNALNAME=\"\\\"$${TARGET_INTERNALNAME}\\\"\"
DEFINES += TARGET_VERSION_STR=\"\\\"$${CAQTDM_VERSION}\\\"\"
# 4.4.1
# caQtDM can be compiled with Qt6
# new signals for caCartesianplot
# fix for caInclude with upscaling
# fix for undefined macros to define a standard value in macro
# fix for popup panels to get a panel without data monitors
# fix for caLineEdit and caTextEntry to handle strings with Signal/Slot
# fix for the start screen on some mobile iOS devices
# cleanup of data plugin messages
# fix for Qt6.4 compiler settings with EPICS4 header on Windows
# fix the PV data selector in designer, missed environment

# 4.4.0
# fileopenwindow: options changes
# fix for converted adl files
# fix for higher python versions
# fix cacartesianplot for minor ticks disappeared when changing number of ticks
# searchpaths handling for non ADL files
# catable fix for big numbers
# bsread fix null number counting messages
# epics3plugin fix for disconnected channels
# epics4plugin is now working with the epics 7 API and PVA can be used
# filter feature for epics 7 is now available
# caStripPlot: feature dynamic property "Legend" for rename the channel to a user defined
# PV-Editor for Designer with network based auto completion
# new commandline option [-savetoimage] to save screenshots as PNG files
# added C entry points for python
# new decoding functions for camera images (Mono8,Mono10p,Mono10packed,Mono12p,Mono12packed)
# future use of compression for camera images (zLib+jpg)



# 4.3.0
# POPUP status windows with possible delays
# dynamic Property caqtdmPopupUI and caqtdmPopupUI_Delay for POPUP windows (The filename must contain the word popup.ui)
# wmsignalpropergator can resize a panel
# fix for to long strings in channel names
# fix for some character problems
# fix for alarm status for caMenu
# fix for softPV Waveform Processing
# fix for softPV initialisation, wrong initial value, ui loader depending
# cleanup in the sf-archiver
# fix in caMenu, paintEvent removed
# more file status check before accessing (avoid AFS problems)
# fix the seg fault for the classname problem EPushButton
# softPV vector/scalar performance problem fixed
# start of EPICS 4 to EPICS 7 implementation
# fix for string length problems
# calinedraw emit textChanged
# optimized caCalc signal emitance
# caCamera add slots for zoom and position
# caQtDM can be compiled with VS2019
# Updates for Android and IOS
# fixes some problems with MOC and QWT
# build in ADL and EDL Parser
# shellcommand for Mac
# landscape version for Android
# fileopening with dynamic conversion causes problems using TEMP directory
# added for archivSF the redirection feature to access multiple data sources
# fix a floating point problem on RHL7 in the archiveSF



# 4.2.4
# fixed a exception in bsread at closing
# added a userdefined format to caLinEdit,caTextEntry and caTextEntry
# add macroreplacment for tooltip and statustip on: caLineEdit, caMultilineString, caGraphics, caPolyline, caMessageButton and caLed
# fix TLS problem in sf archiver for old Qt Versions
# buffer overflow in sf_archiver plugin corrected
# extend macro replacement for cascript button
# reusing connected channels in epics3 plugin for seting data
# borderWidth designer problem with numbers >1, fixed
# changes for python3
# fix for backend check. Backend property will generate a message that the backend name is not known. but it will use it.
# too strict check on RegEx match for visibility
# caTextEntry value could be set beyond channel hopr or lopr limits. this has been corrected

# 4.2.3 of 11.7.2018
# focus can be kept now in caTextEntry even on pointer leave by specifying a property
# instead of exchanging through shared memory directly for -attach, a queue has been integrated for attaching
# for caLineEdit in case of bsread, the unit will be acquired automatically through epics (PSI special)
# remove extra white spaces and special characters (lf, cr) form macrodefs
# added for caTextEntry  a drop possibility when dragging into it.
# new bsread options
# in the unit, the u in uJ will be converted to a real mu

# 4.2.2
# shared memory key was not set correctly for -attach; corrected now

# 4.2.1
# after write epics channel is cleared now
# performance enhancement for caCalc
# added some signals and slots
# internal macros added
# added property to caCartesianplot for changing size of symbols by a multiplicator
# bug fixes

# 4.2.0
# major changes in camera widget to support all possible epics data modes and camera modes
# rich text now also considered for fontscaling
# epics version number will now be printed in the message window
# added include definition for epics 3.15 and higher
# prevent too many messages when epics exceptions

# 4.1.8
# bsread support for 2D waveforms
# added ctrl+ and ctrl - to increase or decrease window size
# added a slot for executing the message button
# unresolved macros window implemented
# the problem of increasing open files solved
# added possibility to disconnect all channels in hidden tabs
# problem with autoscale and infinite values solved
# month number in date corrected
# soft variables were not always updating. eliminated test for update; soft variables will not be updated in hidden tab

# 4.1.7
# problem with cawavetable corrected
# when Qt4.6 using qwt6.0 no support for date time axis, fallback to linear axis
# problem introduced for cacalc, corrected in new release
# added possibility to get info for objects when overlay widget has no monitors

# 4.1.6 of 2.11.2017
# added to print date, time and filename
# added signals to cachoice, camessage so that setting of the index of a tabwidget will be possible
# changed all floats in archive plugins to doubles in order to get a better resolution of the time axis
# access security had a small bug when over a gateway
# command line option updatetype=direct added to set caQtDM in direct update mode

# 4.1.6 of 18.10.2017
# camenu maske integriert, allows to make entries invisible
# cathermo marker has now variable size
# cursor shape did not change when on a cawidget and when access security changed
# regex expressions now possible when using a macro (ask helge brands)

# 4.1.6 of 7.9.2017
# last index in array was not taken into account and scale was not updated for the wareerfallplot
# optimization of archive plugins
# added the possibility to update QtabWidget tab texts with a macro
# changed padding for bigger fonts in carowcolmenu
# added the possibility to modify colors of cacalc
# resize storming of carowcolmenu solved
# cascriptbutton got a property to hide the checkbox
# QTextBrowser can now also display afile speicied with an url in the searchpath (see texts.ui)

# 4.1.6 of 26.7.2017
# a hide/show slot has been added to the ca widgets
# the wmsignalpropagator got signals to propagate the resizing factor of a window. this can be used in order to correct animations with absolute pixel positions.
# when propagating through a signal a value to cacalc, this value is displayed. However was not propgated for further use. This has been corrected
# the colors of the caspinbox could be wrong due to a ack of proper initialisation, this has been corrected.
# very big doubles were not set correctly, this has been corrected.
# static visibility (while no channels) can be used now.
# handle double encoded url's in camimedisplay.
# added hide/show slot in all ca widgets (permits to hide and show a widget when propagating a signal from cacalc)
# when using waveforms from the archive, flashing of the cacartesianplot is suppressed when no data.
# added propagation of signal QRECT to parent, this allows to move a cainclude
# absolute positioning of items added to cainclude; abolute positions can also be channels that can move the positions.
# due to some changes to internal stylesheet, the carelateddisplay could not be hidden, this has been corrected
# x scale of cartesian plot can now also be used with a time scale (milliseconds from epoch)
# for visibilitycalc, when no channels specified, static calculation will be done at startup
# archive retrieval revisited, while crashing and http requests had to be canceled at exit.
# added check for when git not present
# added an environment variable CAQTD_TIMEOUT_HOURS to be used to quit caQtDM automatically, mouse event will reset the timeout.
# it is now possible to use caCalc as a vector (waveform) by specifying associated pv's. the soft variable can then be used to display a waveform

# 4.1.5 of 8.4.2017
# possibility to add a frame around a caInclude
# in case of epics 3.15 added DBE_PROPERTY
# modified caQtDM.pri for macos
# modified caLineEdit and caLineDraw for displaying long long instead of long when double to integer representation

# 4.1.5 of 5.4.2017
# caled circle will stay a circle when resizing with another aspect ration; border color can be customized
# add vumeter simulation to caThermo

# 4.1.5 of 23.3.17
# calineargauge made correct size when no scale (in order to make all sizes the same)
# for caRelatedDisplay one can now set the position for the called window
# for caThermo a problem solved in case of alarm colors (not always refreshed with right color) and transparency added for background
# for caThermo offset of 4 pixels modified in case of noscale

# 4.1.5 of 15.3.17
# added horizontal and vertical spacing properties to caInclude
# added possibility to modify position and size of a control object by the function %QRect in caCalc using 1 to 4 channels
# loading of files through internet now also over https
# again compatible with Qt4.6
# cachoice bug when using bit offset corrected
# legend added to cacartesianplot
# calineargauge and cacirculargauge modifier in indor to be smaller and changed font algorithme.
# added possibility to have another http address for archiveSF
# added test to indicate that waveforms are not supported by the archive plugins.

# 4.1.5
# archive plugins added
# epics4 plugin finalized for normative types, thanks to Marty Kraimer
# window management widget implemented (close window, ...)
# careplacemacro widget implemented; allows to redefine macros during runtime
# utilities designer plugin added for widgets not directly related to the control system
# X/Y waveforms implemented in camera view
# bsread plugin developed by Helge Brands finalized

# 4.1.3
# added for the cartesianplot resize of the fatdots plot
# softpv treatment was slowing done caQtDM and is corrected now
# camera has been more optimized for 16bit waveforms (the other types not yet, due to a lack of waveforms types)
# label and vertical label can gave a border now
# caChoice has now the possibility to display a row from the bottom to the top (rowInverse) and is optimized
# soft pv's without a name were not working, now automatically a name is generated
# in order to have cacalc's working correctly in includes, a macro can be used in order to individialize them
# zero values in cartesian plot are now replaced by the lowest non-zero value when logarithmic scale is used
# for infinite values, cartseian plot was taken unlimited resources for display, this should be corrected nw
# slots are added to cagraphics for animation of these objects (tilt angle, span angle, arc, .. can be set now through signals
# caCamera will now zoom by default on the middle of the image and otherwise around the last clicked point
# autorepeat on canumeric has been take out on request of the users
# default timing of the timed update loop has now a lower internal rate, may still be changed by a json string
# .ftvl field of epics is now used to distingues signed anand unsigned display in cawavetable
# in pep file you may add now the keyword -minwidth to an item in order to define the minimum width of it (default value=100).
#      you can steer column width like that. also -comsize has been added for the comment to steer the fontsize.
# cachoice has been slightly modified in order to grow instead of shrink (for pep files this was an issue)
# searching of a pv for the infobox would find the pv independently of the associated plugin, giving therefore confusion

# 4.1.2
# caLineDemo has been renamed in caLineDraw and is now able to draw vertically
# cawavetable takes now information from channel.FTVL into account in order to define signed or unsigned data
# added CTRL+C to camultinestring for copying to clipboard

# 4.1.0
# small change for pep files (-comsize and -minwidth), cachoice colors now also configurable through stylesheet
# caslider and cathermo can now display also a value; caslider got also the possibility to set distinctly the lower and upper limit (Josh Dassigner from Argonne)
# caQtDM will now display mu and grad on all platforms
# adapted cathermo and caslider so that it can take ito account a default style with fore and background colors; now Alarm is modified in Alarm_Default or Alarm_static
# now a widget can be added (see caLineDemo) without having to change caQtDM_lib. An interface has been defined in order to perform the datacuisition inside the widget class.
# caLineDemo widget is an example for a fast on QWidget based line monitor
# caMimeDisplay will now take macros into account
# fatDots implemented in cartesian plot
# more optimized image display
# tab order and focus considered
# trailing zeros of caspinbox and cawheelswitch suppressed
# toggle button for a soft pv was not working
# caQtDM uses now qwt6.1.1 in order to take advantage of multicore processing
# carowcolmenu performance enhancement
# options -url and -emptycache added
# shortcuts defined (Ex: Ctrl+R = reload window; Ctrl+Alt+R = reload all windows; Ctrl+O = Open File; Ctrl+P = Print)
# stylesheet entry for most of the cawidgets has been suppressed and for many cawidgets the influence of an external stylesheet can only be changed when setting default colormode

# 4.0.3
# it is now possible to pipe ui data to caQtDM
# reload management had to be changed due to some Qt memory management problem
# eliminated some memory leaks
# corrected wavetable string crash
# one can now define if the size of an included ui file will be size adjusted or not
# cabyte, cabytecontroller performance were bad and has to be be corrected
# camenu and catogglebutton color handling is now done by stylesheet and not with palette colors. This in order to prevent cascading stylesheets, that have precedence over palette colors, to interfere.
# default property for label set to transparent color
# window for included filenames shows now the number of times and average loadtime for loading.

# 4.0.2
# caQtDM can now be told to load a specific stylesheet file by the option -stylefile filename
# option -noResize has been implemented now
# capolyline initialisation was missing for proper resize
# new options on command line available for caQtDM: -cs for specifiying default plugin to be used, -options for passing options (key, value pairs) to plugins
# use now the QStringlList editor in Designer for semicolumn separated strings
# mobile handling of config files slightly modified
# adjustment of fonts for android modified / apply button of caapplynumeric is now resizable
# resizing was perturbed by a resize forced by incoming channel data.
# the vertical label has now an alignment property
# cacartesianplot was not always well triggered when a trigger channel was specified, now corrected
# camimebutton added in order to start applications defined through the file extension
# cawavetable can now adapt its rows and columns automatically when zero is specified.
# an enum can be display in calineedit as number when enumeric is specified as format (otherwise as string in all cases)
# a multistring widget has been added to display waveforms with chars and strings (in case of chars a \n will make a new line)
# initialisation of the first stripplot values with nan instead of zero. Value zero was confusing.
# added the possibility to clear the messages through the file menu
# in case of -attach with a huge macro, the shared memory was designed too small. now 2 kBytes can be transferred
# camultilinestring has been implemented in order to mainly display char waveforms where carriage returns (ascii code 13) will produce a new line
# cameter was not updating its label for Qt5; this has been corrected
# some possible buffer overrun conditions corrected
# one can choose now the number of divisions for x on castripplot
# tooltip for properties are now present in psi patched version of Qt4.8.2 and for Qt5.6.0 and higher
# tooltip for caQtDM objects will integrate the tooltip given by the designer
# signal emit of cacalc has now also its integer and double value (this way you can for example switch the tabwidget pages with a cacalc that is updated by a soft/real channel)
# pep emulation has been updated by cosylab
# some slots have been added in order to link signals and slots in designer
# QStackedWidget will also optimize its io for visible/hidden tabs
# color handling for calineedit has been slightly changed in order to be able to get white on red in case of a major alarm (wanted for pep handling)
# cartesianplots have now a group property in order to be able to align the horizontal scale (vertical label rectangle gets same width)
# when a pep file and an ui file were displayed by caQtDM, some resize problems and a problem with capolyline was detected; is now solved
# cainclude can handle now multiple instances of the same file in column, row or rowcolumn mode and use for each instance another macro

# 4.0
# caQtDM has now a controlsystem plugin structure. CS can be added by writing a plugin (see demo plugin) that will automatically be loaded. By specifying the plugin in front
# of the pv name (i.e epics3://somepv) that plugin will then be used.
# fixed a small problem while writing a string to epics (could crash)
# the build files have been simplified
# edl2ui enhanced (by Lucas Carvalho)
# several warnings (mainly on mac osx) solved
# capolyline was not resizing correctly, this has been now corrected
# a string containing a semicolumn was only displayed up to the semicolumn, this is corrected now.
# cacamera and cascan2d got display of selected values and readback values with different representations and may therefore write values to the CS
# soft variable bug corrected (was not always found when writing)
# catextentry got input dialogs (filedialog in case of strings, otherwise simple dialog)
# catextentry will keep its cursor position after pressing return to set the value
# activ widgets were always brought in front. Now you can choose if you let the designer define the layer
# QTextBrowser can be used with macro substitution. The file that will be read in, will be watched for changes and will automatically be reloaded
# a vertical label has been implemented

# 3.9.5
# calinedit for DBF_CHAR was missing.
# infobox got epics description of channel
# some small bugs corrected in infobox
# some reingineering done
# cabytecontroller directions corrected

# 3.9.4
# caQtDM will now also download from http when CAQTDM_URL_DISPLAY_PATH is defined
# When starting with the option -httpconfig you will get the configuration screen in order to use network files as in mobile apps.
# macros can now also be read when specifying -macrodefs filename
# caMessageButton can be disabled/enabled by a second channel
# configdialog for http configuration slightly changed
# Zai added some edl objects
# added cabytecontroller,for reading and setting individual bits
# camenu: prevented scrolling of menu with mouse scroll while it interferes with scrollareas
# changed default direction of caByte in parser.c in order to be compatible with MEDM
# softpv name can now contain a macro
# added calc string to info window

# 3.9.2 and 3.9.3
# severity of a char/string record was not considered and is corrected, Char waveforms longer as 1024 were not displayed in calineedit and is corrected.
# in edl2ui zais addons were integrated
# softpv's in include file enabled
# more colortables for caCamera, caScan2D, caWaterfallPlot
# caScan2D added

# 3.9.1
# added a clock allowing to display normal time or reception time of a process variable; alarm handling can be enabled to colorize the clock
# added a new dial
# problem with soft pv corrected
# QTextBrowser can be used with macros in order to dynamically load files for display them in this textbrowser
# modified the configdialog for ios and android
# prevent caQtDM from crashing when maxvalue and minvalue have the same value.
# for some images, the last row was missing and is corrected now

# 3.9.0
# many changes involving android
# many changes for iphone, epics can now also reconnect after being suspended
# calineedit a bit more performant
# on ipad/iphone files were always downloaded, this has been optimized
# graphic plugings were missing on osx/ios, has been corrected
# prevented crash on exit
# cacamera can be displayed without toolbuttons and will be zoomed in ints container
# cascriptbutton has now the possibility of a macro in its parameter
# truncate value display in calineedit is now implemented

# 3.8.10
# caStripPlot was eating the CPU and has been optimized; refreshrate (low, medium, high) has been introduced, so that it can be adjusted by the user
# some crash conditions eliminated

# 3.8.9
# change for g++ 4.4.7
# pixmaps on buttons are now rescaled
# modified cawavetable to display as matrix with a finite number of elements; elements can be modified and representation choosen
# caWaveTable can display now values as decimal, string, hex and octal
# on ipad the slider click and tapandhold interfered, this has been solved
# on ipad the autorepeat option of the wheelswitch has been disabled while interfering with tapandhold
# on ipad it is possible now to give multiple ip addresses in the EPICS_CA_ADDR_LIST separated by blancs
# eliminated some warnings
# test for null pointers added in case of absence of channels
# resizing of caTable and caWaveTable has been changed
#

# 3.8.8
# modified the shared memory key for x-windows in order to distinguish the display used

# 3.8.7
# added limits and precision dialogs for several objects
# in case of tabbed widgets, monitoring of hidden pages is now suspended in order release network traffic and cpu load
# build of product is now optimized with dependencies (thanks H.Brands)
# modified dialogs for virtual keyboard in IOS

# 3.8.6
# drvh, drvl changed back to hopr, lopr for caslider and canumeric
# slider will not move when no write access

# 3.8.5
# drvh, drvl changed back to hopr, lopr for caslider and canumeric
# adl2ui will not expand label width per default, when required, then use the option expandtext
# dialogs have been modified for better display on ios
# cacalc has now a scalable font
# region of interest selection with write to channels for cacamera
# autorepeat for caslider and canumeric
# epics data monitor changed to DBR_STS with correct initialisation when defining channels and on ioc reboot
# protected cawaterfall plot agains to fast update

# 3.8.4
# more changes for ios
# greek micro replaced by u when macos or ios
# when rescaling castripplot, figure will be cleared

# 3.8.3
# castripplot has been modified so that resizing does not clear the curves
# epics timestamp introduced for info box

# 3.8.2
# modified catogglebutton in order to be able to resize its label

# 3.8.1
# added cawavetable in order to display a waveform numerically (maximum items = 500)

# 3.8.0
# adapted source code in order to run on ios (ipad)
# cachoice crashed when index was higher than the number of ENUMS, fixed
# castripplot changed in order to get also auto scaling
# the title of the display can now be set in the window caption by defining the dynamic property "Title" for the mainwindow. when using a macro definition, this will be expanded.

# 3.7.9
# minor adjustments
# caslider was modified so that scale behaviour was incorrect, this has been corrected

# 3.7.8
# when writing a double, float was used so that the precision was lost, corrected
# related display modified for up to 16 items
# changed names for designer groups to include PSI Name
# changed caToggleButton and caLed behaviour
# changed caSlider and caThermo to be compilable with the standard version of qwt6.1 when using qt5.2
# modified caQtDM_Lib to be callable from the software of the Australian lightsource
# fixed a bug in the scrollbar behaviour of the camera widget

# 3.7.7
# in waterfall plot the demo plot was not erased at initialization with real data, corrected

# 3.7.6
# added a property to the waterfall plot in order to define the maximum number of points treated in the waveform
# due to the change 3.7.4 for reload, the popup of an existing window was not working correctly

# 3.7.5
# on mac osx camera and waterfall plot were crashing; this has been corrected

# 3.7.4
# reload did not find the ui file any more when started with explicit path. corrected
# splash screen did not count correctly in some situations. corrected
# small memory leak repaired in cainclude

# 3.7.3
# after the improved performance of the cartesian plot, the trigger mode disappeared, corrected

# 3.7.2
# cagraphic lost its linecolor after no connection
# calineedit was not showing alarm colors in alarm_default mode

# 3.7.1
# improved performance for cartesian and waterfall plot; very long arrays can be displayed
# zoom in cartesian plot was not working well for small values and has been corrected
# epics request for arrays (ca_add_array_event) changed to a request for 0 elements in order to get the arrays with their actual length
# added properties to calineedit in order to be able to make a framed border

# 3.7.0
# a spinbox has been integrated in order to be smaller than the wheelswitch. for the spinbox the digit has to be activated by click
# in order to change it.

# 3.6.6
# strings in calineedit were trimmed; took that out

# 3.6.5
# selection in catable can be rows or coluns now, catable can execute a specified script with the associated pv when doubleclicking on the value
# eliminated sending message in camessage with right button of mouse.

# 3.6.4
# an empty char waveform was not clearing the calineedit: corrected
# enums with empty strings were not displaying correctly in calineedit
# when reloading a .prc file, size of display was not minimized

# 3.6.3
# default back and foreground colors can be set now for calineedit/catextentry in ui-file stylesheet of mainwindow or in stylesheet.qss
# negative precision was leading to an unexpected behaviour, calineedit will use the precision and switch to exponential display
# spacebar press has been suppressed for camessagebutton, cashellcommand, carelateddisplay, carowcolmenu, cachoice
# changed for caslider and canumeric the limits for channel to DRVL and DRVH
# added to formread in prc file command and comlab modifiers

# 3.6.2
# cartesian and stripplot support now logaritmic scales

# 3.6.1
# suppressed slidervaluecallback when setting limits from hopr, lopr
# added a first attempt of a waterfall plot for waveforms
# corrected for cameras, position calculated when zooming to small images
# in case of reload, data display is now stopped until all displays are loaded again

# 3.6.0
# camera images can now be zoomed and scrolled
# reset zoom for cartesian plot did not reset correctly in case of channel limits

# 3.5.4
# added zoom mechanism for cartesian plot
# added update mechanism choice in main menu in order to switch to direct monitoring or timed monitoring

# 3.5.3
# starting performance optimized for relateddisplay, cachoice, camenu
# splashscreen added for loading includes (in some cases loading time can be significant)

# 3.5.2
# by using CAQTDM_EXEC_LIST as environment variable (as in MEDM with the same syntax), menu items can be added to the
# context menus
# for cathermo in parser the look was not see as property and cathermo itsself was initializing itsself per default wrong

# 3.5.1
# bug corrected when unfinished enum definition
# slider was writing to epics on first call, now it does not
# capolyline now does not extend its lines when linesize > 1 is used, reproducing the behaviour of MEDM
# cathermo and caslider resize handle and labels according to the size, foreground and background are working correctly now
# however with qwt6.1.0 in qwt_slider the routine scaleDraw(); must be set protected instead of private as was in qwt6.0


# 3.5.0
# caQtDM gives information now about number of monitors/s, number of displays/s and highest monitor

# 3.4.7
# adapted caslider for qwt6.1

# 3.4.6
# corrected a bug for cagauge with alarms no
# use control key for amplifying 10x the increment

# new in 3.4.5
# caslider has now a menu to change its increment, in designer increment can be specified to
# caslider moves also on left mouse click and an wheel scroll; when it has the focus, up and down keys can be used (when focus outline red gets drawn)
# gauges have now following properties: alarmLimits : Channel_Alarms, User_Alarms, None
#                                       displayLimits :  Channel_Limits, User_Limits
# adl2ui has been corrected for these properties

# new in 3.4.4
# enhanced performance for camera widget

# new in 3.4.3
# package can also be build with Qt4.6 and qwt6.0, necessary for SL6 (cacartesianplot, cadoubletabwidget and plugins slightly modified)
# epics enums and strings modified for count > 1

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
# windows are resizable, in order to have it working well one should always use fontscalemode=WidthAndHeight. in the deafult style sheet no font sizes should be specified.
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


