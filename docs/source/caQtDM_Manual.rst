=============
caQtDM Manual
=============

| **Anton Mezger**
| **February 2014**
| Paul Scherrer Institute
| CH-5232 Villigen
| Switzerland

**This manual**
---------------

This manual has been written in HTML and after conversion introduced in
the Qt assistant in order to get help for the caQtDM system. It has to
be noted that caQtDM is though as a replacement of the very known EPICS
Display Manager MEDM, therefore this manual is of course also inspired
of the existing MEDM manual.

The HTML was converted to restructured text using pandoc, then hand-edited.

**Introduction**
----------------

caQtDM is a package based on Qt developed at PSI as a successor of
MEDM, a well known package used in the EPICS community for building
synoptic displays. The caQtDM package uses the standard Qt GUI (Qt
designer) for designing synoptic displays, containing all the controls
graphical elements necessary to visualize and control a facility. Qt
designer uses the custom widgets designed for this purpose and writes
a description file (.ui file) that will be used by the synoptic viewer
caQtDM. A reload function in caQtDM allows to rapidly see the changes
made by the editor in order to accelerate the process of editing and
viewing.

The heart of the package are the graphical objects (Widgets) plugged
into the Qt designer. These widgets will be described in detail with
their properties below. Many objects have been built and more are
introduced as soon as they are needed. The Qt system allows in a very
easy way to add continuously new objects or to modify these.

Technically the package consists of 3 class libraries with the
graphical objects, a class library used by the viewer or other
applications and the viewer (caQtDM) itself as well as 3 libraries
with the plugin definitions for the Qt designer. The editor (Qt
designer) is part of Qt. It has to be noted that the graphical objects
are not control system aware. The graphical objects have properties
and methods used by the editor. The control system knowledge is then
integrated and handled by the viewer. This architecture allowed us to
handle easily three control systems: epics, epics4 and an older in
house control system. These are all handled by the viewer. This
distribution supports per default epics 3. Epics4 can be integrated
when building it, but only a few tests could be done while not many
process variables could be tested so far.

Part of the package is also a converter (adl2ui) that can convert MEDM
.adl files into .ui files used by the caQtDM package. In most cases
this converter gives good results. Besides the already mentioned .ui
files the viewer can also uses in house developed .prc files. These
last files represent asci files to be edited with any text editor
using are in house syntax.

The Qt system is widely used today on many platforms and is therefore
an ideal system to develop new applications. The advantage of Qt is
therefore that an application using this architecture will run on many
platforms, so that caQtDM will run on linux as well as on windows
systems.

Designer: editor
--------------------

The designer coming with Qt is the editor used for developing the
synoptic screens. In this manual we will not describe this editor
exhaustively, while the editor has it own manual that can be called by
pressing the F1 key. The editor allows by dragging and dropping the
graphical objects to be used into the synoptic display. The properties
of the graphical objects can then be customized in the property editor.
The most important property is normally the device that is monitored or
controlled and is represented by the property channel. An exhaustive
description of the graphical objects and their properties will be given
below.

Install
-----------

| Since version 3.1.4 of caQtDM a new build and installation procedure
  working for several linux platforms are available.
| First you will have to unpack the tar archive, which will create the
  necessary directories and files. In the upper directory you will find
  the necessary procedures to build the package, run it locally without
  installing and also an installation procedure.

#. you will have to install Qt-4.8 and qwt-6 when they are not already
   installed. On modern systems Qt4 should already be installed, but the
   qt4-designer not necessarily and you should install it (eventually
   with sudo apt-get install qt4-designer). qwt is normally not
   installed and you should install this too.
#. you will have to install EPICS
#. all the environment variables used for the installation will be
   defined in the script caQtDM_Env. When some of the variables are
   already defined in your environment, the script will use them,
   otherwise they will be defined. It is up to you to edit this file and
   adapt the variables to your environment.
#. the script caQtDM_BuildAll will build you the package now into the
   directory ./caQtDM_Binaries. You should obtain there adl2ui, caQtDM,
   libqtcontrols.so and libcaQtDM_Lib.so as well as a designer directory
   containing 3 files libqtcontrols*_plugin.so. if you encounter a
   problem when building due to some environment variable problem or
   while not everything was installed before, you may correct your
   problem and run the script again. In case you want to cleanup you use
   the script caQtDM_CleanAll, which will also delete the resulting
   binaries.
#. you may try to install the package: the package should normally be
   installed in the EPICS extensions and the environment variables are
   pointing to them.
#. running caQtDM: when the package has been successfully build into the
   directory ./caQtDM_Binaries, you can start caQtDM by using the script
   startDM_Local and the qt4-designer by the script qtdesigner (all
   located in the main directory where you expanded the tar file). These
   scripts will set some important environment variables in order to set
   the path, the plugin_path and to point to the test directory provided
   with the package (./caQtDM_Tests). In this directory also the
   stylesheets are located to be used for a nice rendering. You may call
   the test.ui file with the above mentioned scripts. In order to test
   the display some EPICS channels have to be defined. This can be done
   by running a soft-ioc shell called with run-epics also provided in
   the test directory.
#. in case you have qwt 6.1 you will have to use in
   caQtDM_QtControls/src the files qwt_thermo_marker_61.\* instead of
   qwt_thermo_marker.\*
#. in case you already use Qt5.1 with qwt6.1 the building should also be
   straight forward.
#. Instructions for compiling caQtDM on Windows Requirements:

   -  Qt 4.8.2
   -  Qwt 6.0.1
   -  EPICS 3.12.2
   -  MS Visual Studio 2010
   -  Wix 3.0.5419.0

   With caQtDM_Env.bat you can configure your system. All settings which
   are needed will be taken from here.
   The building can be started with the batchfile : caQtDM_BuildAll.bat
   The packaging is made with the batch file : caQtDM_Build_Package.bat
   This will generate a MSI installation file in the folder of
   caQtDM_Binaries.
   To clean the Folder you should use caQtDM_CleanAll.bat.

**Development history**
-----------------------

The following list describe the new features and bug fixes for every
release. You can follow the development history and detect if a bug in
the used version has been solved.

| 3.7.2

-  cagraphic lost its linecolor after no connection, corrected
-  calineedit was not showing alarm colors in alarm_default mode,
   corrected

.. container::

   3.7.1

-  improved performance for cartesian and waterfall plot; very long
   arrays can be displayed
-  zoom in cartesian plot was not working well for small values and has
   been corrected
-  epics request for arrays (ca_add_array_event) changed to a request
   for 0 elements in order to get the arrays with their actual length
-  added properties to caLineEdit in order to be able to make a framed
   border

.. container::

   3.7.0

-  a spinbox has been integrated in order to be smaller than the
   wheelswitch. for the spinbox the digit has to be activated by
   clicking in order to change it.
-  strings in caLineEdit were trimmed; took that out
-  selection in caTable can be rows or columns now, caTable can execute
   a specified script with the associated pv
-  eliminated sending message in caMessage with right button of mouse.

.. container::

   3.6.4

-  an empty char waveform was not clearing the caLineEdit: corrected
-  enums with empty strings were not displaying correctly in caLineEdit
-  when reloading a .prc file, size of display was not minimized
-  default back and foreground colors can be set now for
   caLineEdit/caTextEntry in ui-file stylesheet of mainwindow or in
   stylesheet.qss
-  negative precision was leading to an unexpected behaviour, caLineEdit
   will use the precision and switch to exponential display
-  spacebar press has been suppressed for caMessageButton,
   caShellCommand, caRelatedDisplay, caRowcolMenu, caChoice
-  changed for caSlider and caNumeric the limits for channel to DRVL and
   DRVH
-  added to formread in prc file command and comlab modifiers

.. container::

   3.6.2

-  caCartesian plot and caStripPlot can have a logarithmic scale now

.. container::

   3.6.1

-  suppressed slidervaluecallback when setting limits from hopr, lopr
-  added a first attempt of a caWaterfall plot for waveforms
-  corrected for caCamera, position calculated when zooming to small
   images
-  in case of reload, data display is now stopped until all displays are
   loaded again
-  camera images can now be zoomed and scrolled
-  reset zoom for caCartesianPlot did not reset correctly in case of
   channel limits
-  added zoom mechanism for caCartesianPlot
-  added update mechanism choice in main menu in order to switch to
   direct monitoring or timed monitoring
-  starting performance optimized for caRelatedDisplay, caChoice, caMenu
-  splashscreen added for loading includes (in some cases loading time
   can be significant)

.. container::

   3.5.2

-  by using CAQTDM_EXEC_LIST as environment variable (as in MEDM with
   the same syntax), menu items can be added to the context menus
-  for caThermo in parser the look was not see as property and caThermo
   itself was initializing itself per default wrong

.. container::

   3.5.1

-  bug corrected when unfinished enum definition
-  slider was writing to epics on first call, now it does not
-  caPolyLine now does not extend its lines when linesize > 1 is used,
   reproducing the behaviour of MEDM
-  caThermo and caSlider resize handle and labels according to the size,
   foreground and background are working correctly now
-  however with qwt6.1.0 in qwt_slider the routine scaleDraw(); must be
   set protected instead of private as was in qwt6.0

.. container::

   3.5.0

-  caQtDM gives information now about number of monitors/s, number of
   displays/s and highest monitor

.. container::

   3.4.7

-  adapted caSlider for qwt6.1

.. container::

   3.4.6

-  corrected a bug for caGauge with alarms no
-  use control key for amplifying 10x the increment

.. container::

   3.4.5

-  caSlider has now a menu to change its increment, in designer
   increment can be specified to
-  caSlider moves also on left mouse click and an wheel scroll; when it
   has the focus, up and down keys can be used (when focus outline red
   gets drawn)
-  gauges have now following properties: alarmLimits : Channel_Alarms,
   User_Alarms, None
-  displayLimits : Channel_Limits, User_Limits
-  adl2ui has been corrected for these properties

.. container::

   3.4.4

-  enhanced performance for caCamera widget

.. container::

   3.4.3

-  package can also be build with Qt4.6 and qwt6.0, necessary for SL6
   (caCartesianplot, caDoubletabWidget and plugins slightly modified)
-  epics enums and strings modified for count > 1

.. container::

   3.4.2

-  caCamera stuff modified
-  added mutex to synchronize data and display
-  font bug in caTable solved
-  found a datarace in caStripplot, corrected

.. container::

   3.4.1

-  caCamera displays intensity at x, y
-  caCamera context menu for grey / spectrum and zoom

.. container::

   3.4.0

-  caStripplot has been revisited
-  caCamera uses now concurrent mode for calculations

.. container::

   3.3.1

-  mainwindow and messagewindow have been merged

.. container::

   3.3.0

-  caTable gets now the correct font after resizing when values update
-  caStripPlot has been corrected for incorrect time behaviour.

.. container::

   3.2.2

-  in menu you have the possibility to raise the main and message
   windows

.. container::

   3.2.1

-  when requesting a display that was already started, it did not popup
   but made a new instance. this has been corrected
-  this was working before, but by some modifications was not correct
   any more

.. container::

   3.2.0

-  The build mechanism for linux and windows should do on most
   platforms, see caQtDM_README
-  caQtDM build for qt4.8 with qwt6.0 and for qt5.1 with qwt6.1
-  caLed can use now the severity to colorize
-  caThermo was inadvertently using nodeco mode when mode was used
-  caCircularGauge will draw a white scale when not connected and bug
   corrected
-  for enums without enum string write just the value in caLineEdit

.. container::

   3.1.1

-  The main characteristic of this version is that you can resize the
   caQtDM synoptic display windows without using Qt layouts. You can
   build a display normally and everything will be resized when you
   resize the window.
-  In order to make the resizing correctly, you should specify for the
   fontscalemode of caLabel and caLineEdit the parameter
   "WidthAndHeight".
-  When you convert MEDM displays with the utility adl2ui, this is done
   automatically.
-  When you do not want a window to be resized, you can specify
   -noResize on the command line.
-  Another enhancement existing since version 2.8 is the fact that you
   can hook a digit of the wheelswitch to the keyboard in order to
   modify it with the up and down keys.

.. container::

   2.8

-  All the changes have been documented in qtdefs.pri

.. container::

   2.5

-  Scales of Stripplot can now be changed during runtime. The caQtDM
   application normally limits the update rate of monitors to 5 Hz,
-  however now a JSON string can be written behind the channel name.
-  An example is given in the presentation `"introduction to
   caQtDM" <caQtDM_introduction.ppt>`__.
-  Color mode for calabel is now working (was forgotten). Also an icon
   is presented on the window bars.
-  A Windows distribution package allowing to work immediately with the
   designer and caQtDM has been build and can be downloaded here
-  (this package does not contain any sources, merely executables, work
   done by H.Brands at PSI).

.. container::

   2.3

-  Timebase of Stripplot was wrong and has been corrected. Some other
   compiler warnings have been eliminated and the building process was
   slightly modified.

.. container::

   2.1

-  When using correctly the layout possibilities offered by Qt, one can
   resize a window (you will find an example in
   work/caQtDM/parser/proscanFiles/phaseslitsnew.ui).
-  Up to now when using "includes" only a placeholder could be seen in
   the designer. Now the included ui file is displayed inside the
   designer.
-  For polylines and polygones, a graphical editor is integrated into
   the designer. One can create segments by pressing the left mouse
   button when moving the
-  mouse and one can delete the last created segment by pressing the
   right mouse button.
-  The graphical entities (rectangles, circles, ellipses, lines, arrows,
   triangles) can be rotated by specifying a tilt angle.

.. container::

   2.0

-  The major changes in this version are changes for stability (in the
   previous version sometimes crashes occurred) and the introduction
-  of soft pv's. A variable can be defined, calculated and used as a
   normal pv.
-  All the features of medm used for the cartesian plots are implemented
   now. The caCartesian plot background can be transparent so that a
   camera image can be
-  underlaid. The limits of the x and y axes can be changed by
   specifiying pv's instead of values when using channel limits. The
   stripplot widget has been
-  modified while the behaviour was not quite correct. The message
   button and textentry were not working well and have been corrected.
   Labels can be transparent too.
-  A camera widget has been introduced. At PSI we can now display are
   usual cameras bw and color. You can easily modify the code to treat
   other formats
-  of camera waveforms.
-  caQtDM gives the number of connected and unconnected channels,
   furthermore a list of unconnected channels with their file location
   can be shown.
-  caQtDM is now protected against ui description file syntax errors;
-  caQtDM supports the command line parameters -attach -noMsg -display
   -macro -x -noStyles -dg. -x has no effect but suppresses the error
   message when
-  keeping the medm command line parameters.
-  caQtDM runs in native mode on linux as well as on microsoft windows.

**caQtDM Custom Widgets**
-------------------------

| The custom widgets used in caQtDM have been divide in three categories
  as was done in MEDM. MEDM users are used to these cartegories and we
  have therefore kept the categorisation in Graphics, Controller and
  Monitors. As in MEDM, Graphics are items such as frames, labels,
  graphical entities like rectangles, triangles, lines, ..., images and
  composite objects (called here includes, while included from other ui
  files. Monitors are objects that monitor the state or values of
  process variables through display of values, plots, tables,
  thermometers, dials, ... Controllers are objects that change the
  values of process variables through many different objects. you will
  find here an exact description of them with their properties: For all
  objects, you have normally their geometry (position and size),,
  sizepolicy and some other properties like tooltips. It has to be noted
  that for many widgets their stylesheet will be overwritten in order to
  get the representation that is requested when specifying for example
  background, foreground, border colors or other properties. The
  stylesheet can be influenced in some cases by specifying a stylesheet
  for the mainwindow or in stylesheet.qss, a stylesheet found in the
  caQtDM path. The adl2ui converter uses normally this stylesheet to
  integrate these defaults into the generated ui file. You may delete
  this stylesheet or change it to your convenience. However as mentioned
  before, many styles will be overwritten.
| The following table gives you an overview of all objects that are
  actually available:

.. FIXME: empty cells, links

=========================================================== =================================== ====================================
Graphics                                                    Monitors                            Controllers
=========================================================== =================================== ====================================
caGraphics: Rectangle, Circle, Arc, Triangle, Line, Arrow   caLineEdit                          caChoice
caPolyline: polyline and polygon                            caLed                               caMenu
caLabel: text                                               caCartesianPlot                     caMessageButton
caFrame                                                     caLinearGauge, caCircularGauge      caRelatedDisplay
caImage: any image format, mostly used for animated gif     caThermo                            caShellCommand
caInclude: the equivalent to composite of MEDM              caStripPlot                         caSlider
caDoubleTabWidget: a general purpose widget                 caByte                              caTextEntry
.                                                           caCamera                            caNumeric, caApplyNumeric
.                                                           caWaterfallPlot                     caToggleButton
.                                                           caBitNames                          caScriptButton
.                                                           caCalc                              caSpinBox
.                                                           caTable
=========================================================== =================================== ====================================

|
| Their main properties are described below. It has to be noted that
  through inheritance of Objects, other properties exist, that are not
  necessary relevant here.

**all monitor objects**
-----------------------

.. _caLineEdit:

caLineEdit, the most used graphical object for data monitoring
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

is the equivalent of the Text Update in MEDM.

   | `Geometry <geometry>`__ is used for any object
   | **Description:**

   **channel:**
      QString: this string represents the control system process
      variable
   **foreground:**
      QColor: color used for the foreground
   **background:**
      QColor: color used for the background
   **colorMode:**
      +---------------+-----------------------------------------------------+
      | Default       | Show the object in colors given by stylesheets.     |
      +---------------+-----------------------------------------------------+
      | Static        | Show the object in colors given by the properties   |
      |               | background and foreground.                          |
      +---------------+-----------------------------------------------------+
      | Alarm_Default | Show the object in alarm colors based on the        |
      |               | severity of the associated process variable.        |
      |               | (green, yellow, red, white or gray). The foregound  |
      |               | or the background are given by the stylesheets      |
      |               | depending on the alarmHandling specifying if alarm  |
      |               | handling is done on the foreground or on the        |
      |               | background.                                         |
      +---------------+-----------------------------------------------------+
      | Alarm         | as Alarm_Default, but the foreground or background  |
      |               | color is taken from the properties background or    |
      |               | foreground                                          |
      +---------------+-----------------------------------------------------+

   **framePresent:**
      boolean: specifies if a border will be drawn around the object,
      frameLineWidth must be different from zero in order to see the
      border
   **frameColor:**
      QColor: color used for the border
   **frameLineWidth:**
      Integer: linewidth of the border
   **alarmHandling:**
      when colormode is set to alarm, it can be specified here if the
      foreground or the background will show alarm colors.
   **precision:**
      Integer: precision in case of precisionMode=User
   **precisionMode:**
      ======= ======================================================
      Channel Precision specified by the control system will be used
      User    Precision specified by the user will be used
      ======= ======================================================

   **limitsMode:**
      +---------+-----------------------------------------------------------+
      | Channel | Limits specified by the control system will be usd (HOPR, |
      |         | LOPR for EPICS)                                           |
      +---------+-----------------------------------------------------------+
      | User    | Limits specified by the user will be used.                |
      +---------+-----------------------------------------------------------+

   **maxValue:**
      float: maximum value used in case of limitsMode=User
   **minValue:**
      float: minimum value used in case of limitsMode=User
   **fontScaleMode:**
      +----------------+----------------------------------------------------+
      | None           | No scaling will be done                            |
      +----------------+----------------------------------------------------+
      | Height         | Text will be scaled to the height of its           |
      |                | containing box                                     |
      +----------------+----------------------------------------------------+
      | WidthAndHeight | Text will be scaled to the height and width of its |
      |                | scaling box; this should be used for a correct     |
      |                | resizing of the display                            |
      +----------------+----------------------------------------------------+

   **unitsEnabled:**
      boolean: when checked will append the unit to the value
   **formatType:**
      +----------------------------------+----------------------------------+
      | decimal                          | value encoded in f format using  |
      |                                  | precision from user or channel,  |
      |                                  | in case of negative precision    |
      |                                  | will switch to e format          |
      +----------------------------------+----------------------------------+
      | exponential, engr_notation       | value encoded in e format using  |
      |                                  | absolaute precision from user or |
      |                                  | channel                          |
      +----------------------------------+----------------------------------+
      | compact                          | value encode in e or f format    |
      |                                  | using absolaute precision from   |
      |                                  | user or channel, format will     |
      |                                  | switch to e format for values <  |
      |                                  | 1.0e-4                           |
      +----------------------------------+----------------------------------+
      | truncated, sexagesimal,          | not supported yet                |
      | sexagesimal_hms, sexagesimal_dms |                                  |
      +----------------------------------+----------------------------------+
      | hexadecimal                      | value will be encoded in         |
      |                                  | hexadecimal format               |
      +----------------------------------+----------------------------------+
      | octal                            | value will be encoded in octal   |
      |                                  | formatoct                        |
      +----------------------------------+----------------------------------+
      | string                           | will be treated as decimal       |
      |                                  | formal                           |
      +----------------------------------+----------------------------------+

   Description:

--------------

caThermo
~~~~~~~~

is the equivalent of the Bar Monitor in MEDM.

   | `Geometry <geometry>`__ is used for any object
   | **Description:**

   **channel:**
      QString: this string represents the control system process
      variable
   **foreground:**
      QColor: color used for the foreground
   **background:**
      QColor: color used for the background
   **colorMode:**
      +---------+-----------------------------------------------------------+
      | Default | Show the object in colors given by stylesheets.           |
      +---------+-----------------------------------------------------------+
      | Static  | Show the object in colors given by the properties         |
      |         | background and foreground.                                |
      +---------+-----------------------------------------------------------+
      | Alarm   | Show the object in colors given by the properties         |
      |         | background and foreground, but change the color of the    |
      |         | pipe in case of alarm                                     |
      +---------+-----------------------------------------------------------+

   **direction:**
      Up, Down, Left, Right
   **look:**
      noLabel, noDeco, Outline, Limits, ChannelV; These property values
      are for compatibility with MEDM, and should be implemented later.
      Actually Outline, Limits and ChannelV will give you a scale when
      scalePosition is different from noScale; noLabel and noDeco will
      have no scale.
   **logScale:**
      boolean: true or false
   **limitsMode:**
      +---------+-----------------------------------------------------------+
      | Channel | Limits specified by the control system will be usd (HOPR, |
      |         | LOPR for EPICS)                                           |
      +---------+-----------------------------------------------------------+
      | User    | Limits specified by the user will be used.                |
      +---------+-----------------------------------------------------------+

   **type:**
      Pipe, marker, PipefromCenter
   **scalePosition:**
      Noscale, LeftScale, RightScale, TopScale, Bottomscale
   **maxValue:**
      float: maximum value used in case of limitsMode=User
   **minValue:**
      float: minimum value used in case of limitsMode=User

--------------

caLed
~~~~~

has no equivalent in MEDM.

   | `Geometry <geometry>`__ is used for any object
   | **Description:**

   **channel:**
      QString: this string represents the control system process
      variable
   **bitNr:**
      Integer: the bit that has to be considered
   **falseColor:**
      QColor: color used when the bit is not set
   **trueColor:**
      QColor: color used when the bit is set
   **colorMode:**
      +--------+------------------------------------------------------------+
      | Static | Show the object in colors given by the properties          |
      |        | background and foreground.                                 |
      +--------+------------------------------------------------------------+
      | Alarm  | Show the object in colors given by the properties          |
      |        | background and foreground, but with alarm handling         |
      +--------+------------------------------------------------------------+

--------------

caLinearGauge
~~~~~~~~~~~~~

is the equivalent of the Bar Monitor in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caCircularGauge
~~~~~~~~~~~~~~~

is the equivalent of the Meter Monitor in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caCartesianPlot
~~~~~~~~~~~~~~~

is the equivalent of the Cartesian plot in MEDM and will plot up to 6
curves

   | `Geometry <geometry>`__ is used for any object
   | **Description:**

   **Title**
      QString: Titel of the plot
   **TitleX**
      QString: Title of the X-axis
   **TitleY**
      QString: Title of the Y-axis
   **channels_1, channels_2, channels_3, channels_4, channels_5, channels_6**
      QString: these strings represents the control system process
      variables; each channelpair is composed of two channels separated
      by a semicolumn.
      When specifying only one channel, you will have to put a
      semicolumn in fron or at the end of the string in order to specify
      x or y, you will find in the table below how the cartesian plot
      behaves in case of array or scalars or when only one channel is
      specified.
   **Style_1, Style_2, Style_3, Style_4, Style_5, Style_6:**
      ========= ============================================================
      NoCurve   no curve, only symbols will be drawn when they are specified
      Lines     normal curve drawn with lines
      Sticks    curve draw with sticks from the x base
      Steps     curve drawn with steps
      Dots      curve only dots are drawn
      FillUnder curve will be filled from the x base
      ThinLines curve draw with thin lines
      HorSticks curve drawn with sticks from the y base
      ========= ============================================================

   **symbol_1, symbol_2, symbol_3, symbol_4, symbol_5, symbol_6:**
      ========= =======================
      NoSymbol  no symbol will be drawn
      Ellipse
      Diamond
      Triangle
      DTriangle
      UTriangle
      LTriangle
      RTriangle
      Cross
      XCross
      HLine
      VLine
      Star1
      Star2
      Hexagon
      ========= =======================

   **color_1, color_2, color_3, color_4, color_5, color_6:**
      QColor: color used for the curve
   **countNumOrChannel**
      QString: The Count for the Cartesian Plot may be an integer or the
      name of a process variable. If the value starts with a non-digit,
      then it is considered to be a process variable name, and the value
      of Count will come from the process variable. If the process
      variable is not found or its value is less than 1, Count will not
      be executed
   **triggerChannel**
      QString: The Trigger Channel is a process variable that causes the
      entire plot to be updated. If there is a trigger channel, the plot
      is updated whenever the value of that process variable changes.
      Otherwise, each individual trace is updated whenever any of the
      data for that trace changes.
   **eraseChannel**
      QString: The Erase Channel is a process variable that causes
      erasing of the plot. If there is an Erase Channel, the plot erases
      when the process variable turns either zero or non-zero, depending
      on the Erase Mode . The Erase Mode is only relevant if there is an
      erase channel.
   **eraseMode:**
      +-----------+---------------------------------------------------------+
      | ifnotzero | Erase the plot if the erase-channel process variable is |
      |           | not zero                                                |
      +-----------+---------------------------------------------------------+
      | ifzero    | Erase the plot if the erase-channel process variable is |
      |           | zero                                                    |
      +-----------+---------------------------------------------------------+

   **plotMode:**
      +--------------------+------------------------------------------------+
      | PlotNPointsAndStop | Plot n points corresponding to the first n     |
      |                    | changes of the process variable, then do not   |
      |                    | plot any more points                           |
      +--------------------+------------------------------------------------+
      | PlotLastNPoints    | Plot n points corresponding to the last n      |
      |                    | changes of the process variable, overwriting   |
      |                    | previous points.                               |
      +--------------------+------------------------------------------------+

   **XaxisType/YaxisType:**
      ====== =================
      linear Use a linear axis
      log10  Use a log axis
      ====== =================

   **XaxisScaling/YaxisScaling:**
      +---------+-----------------------------------------------------------+
      | Auto    | Let the graph routine decide on the axis range depending  |
      |         | on the data                                               |
      +---------+-----------------------------------------------------------+
      | Channel | Get the axis range from the process variable              |
      +---------+-----------------------------------------------------------+
      | User    | Specifically specify the minimum and maximum values for   |
      |         | the axis.                                                 |
      +---------+-----------------------------------------------------------+

   **background:**
      QColor: color used for the background
   **foreground:**
      QColor: color used for the foreground
   **scaleColor:**
      QColor: color used for the scale if scales are specified
   **gridColor:**
      QColor: color used for the grid if grid is specified
   **XaxisEnabled:**
      bool: specifies if the x axis should be drawn
   **YaxisEnabled:**
      bool: specifies if the y axis should be drawn
   **grid:**
      bool: specifies if a grid should be drawn
   **XaxisLimits:**
      QString: minimum value used in case of limitsMode=User; the values
      must be separated by a semicolumn
   **YaxisLimits:**
      QString: minimum value used in case of limitsMode=User; the values
      must be separated by a semicolumn

   .. container::

      in caQtDM curve can be zoomed with the mouse by pressing the left
      mouse button and selecting the area that has to be zoomed. The
      area is shown by a rubberband rectangle. Reset zoom can be chosen
      through the context menu. Translation can be done by pressing the
      middle mouse button and moving the mouse. Pressing the right mouse
      button will bring up a context menu (for pvInfo, resetting zoom
      and changing axes)
      A caQtDM Cartesian Plot has the same conventions as a MEDM
      Cartesian Plot. This plot is a very important tool used in control
      systems and its behaviour was taken from MEDM. The Cartesian
      consists of an X and one or two Y axes on which data can be
      plotted. The sets of data are called traces and consist of a set
      of (x, y) pairswith the properties shown above. The traces
      correspond to curves on the plot. Currently there can be up to six
      traces on a plot. Each trace can (but does not have to) have a
      process variable from which to get the x values and another from
      which to get the y values. These process variables can be array
      process variables, such as Waveforms, or they can be scalar
      process variables with only a single value. The variables have to
      be separated by a semicolumn (see above).
      There are eight possible kinds of traces as seen in the following
      table. The traces for a given plot do not all have to be of the
      same kind – they can be mixed. (In the table Nx is the number of
      elements in the process variable specified for x, and Ny is the
      number of elements in the process variable specified for y. The
      letter n denotes a number greater than one, and a blank indicates
      no process variable is specified. The axis limits LOPR and HOPR
      denote the limits obtained from Channel Access for the process
      variable. Typically, these are the fields LOPR and HOPR in the
      associated record, but can be specified by the user with the
      properties XaxisScaling and YaxisScaling. Count is the specified
      Count for the Cartesian Plot, which is described in more detail
      below.)

      .. rubric:: Kinds of XY Traces

      ==  ==  =================== ========= ======= ========== ==== ========== ==========
      Nx  Ny  Type                Points    Xmin    Xmax       Ymin Ymax       NPoints
      ==  ==  =================== ========= ======= ========== ==== ========== ==========
      n   n   X,Y Vector          x(i),y(i) LOPR    HOPR       LOPR HOPR       Min(Nx, Ny)
      n   1   X Vector, Y Scalar  x(i),y    LOPR    HOPR       LOPR HOPR       Nx
      1   n   Y Vector, X Scalar  x, y(i)   LOPR    HOPR       LOPR HOPR       Ny
      n   ..  X Vector            x(i),i    LOPR    HOPR       0    Count - 1  Nx
      ..  n   Y Vector            i,y(i)    0       Count - 1  LOPR HOPR       Ny
      1   1   X,Y Scalar          x(i),y(i) LOPR    HOPR       LOPR HOPR       Count
      1   ..  X Scalar            x(i),i    x(i),i  LOPR       HOPR 0          Count - 1
      ..  1   Y Scalar            i,y(i)    0       Count - 1  LOPR HOPR       Count
      ==  ==  =================== ========= ======= ========== ==== ========== ==========


   .. container::

      If one of the process variables is an array with more than one
      element and if Count is a number greater than zero, then the
      specified Count is ignored and the value shown in the last column
      of the table is used. Under the same circumstances, if Count is a
      name, then it is used only if it is greater than 0 and less than
      what would be used if it were a number. That is, Count from a
      process variable can only restrict NPoints to a lower number than
      it would be otherwise. The points are plotted from i = 0 to
      NPoints - 1 and update as the values change. In the cases where
      one of the process variables is not specified, the history is
      plotted on that axis against values from 0 to NPoints – 1 on the
      other axis.
      If one of the process variables is an array with more than one
      element and the CA server of the process variable supports dynamic
      arrays, then when a zero is specified for Count, the number of
      array elements will be set to the current number of valid elements
      in the array which may vary with subsequent array update events.
      In the remaining cases, where neither process variable is an array
      with more than one element, Count corresponds to a history of the
      process variable. (These are the cases where NPoints is shown as
      Count in the table.) Each time the process variable changes (or
      when either one changes, in the case of X, Y Scalar) a new point
      is plotted until there are Count points. The points are plotted
      from i = 0 to the lesser of Count -1 and the number of updates.
      When the Plot Mode is “plot n pts & stop,” no more points are
      plotted. When the Plot Mode is “plot last n pts,” the earliest
      point is discarded, the others are moved down, and the latest is
      plotted at the end. In the cases where one of the process
      variables is not specified, the history is plotted on that axis
      against values from 0 to Count – 1 on the other axis.
      The above behaviour still has to be tested thoroughly. It appeared
      already that perhaps Count must be twice the value described above
      ?

--------------

caStripPlot
~~~~~~~~~~~

is the equivalent of the StripChart Monitor in MEDM.

   | `Geometry <geometry>`__ is used for any object
   | **Description:**

   **Title**
      QString: Titel of the plot
   **TitleX**
      QString: Title of the X-axis
   **TitleY**
      QString: Title of the Y-axis
   **channels**
      QString: this string represents the control system process
      variables; the channels have to be separated by a semicolumn.
   **units**
      =========== ======================================
      Millisecond The update period is in milli-seconds.
      second      The update period is in seconds.
      minute      The update period is in minutes.
      =========== ======================================

   **Period**
      Integer: the timespan of the plot for the specified units
   **XaxisType**
      +------------+--------------------------------------------------------+
      | TimeScale  | The scale of the x-axis is given with                  |
      |            | hours:minutes:seconds                                  |
      +------------+--------------------------------------------------------+
      | ValueScale | The scale of the x-axis is given with the values based |
      |            | on the period value                                    |
      +------------+--------------------------------------------------------+

   **YaxisType:**
      ====== =================
      linear Use a linear axis
      log10  Use a log axis
      ====== =================

   **XaxisScalingMax_1 .....7/YaxisScalingMin_1, .....7:**
      +---------+-------------------------------------------------------------------+
      | Channel | Get the axis range from the process variable                      |
      +---------+-------------------------------------------------------------------+
      | User    | Specifically specify the minimum and maximum values for the axis. |
      +---------+-------------------------------------------------------------------+

   **Style_1 ....7:**
      ========= ====================================
      Lines     normal curve
      FillUnder curve will be filled from the x base
      ========= ====================================

   **color_1 ....7:**
      QColor: color used for the curve
   **XaxisLimitsMax_1 .....7/YaxisLimitsMin_1, .....7:**
      QString: minimum/maximum value used in case of limitsMode=User
   **YaxisLimits:**
      QString: minimum value used in case of limitsMode=User; the values
      must be separated by a semicolumn
   **background:**
      QColor: color used for the background
   **foreground:**
      QColor: color used for the foreground
   **scaleColor:**
      QColor: color used for the scale if scales are specified
   **gridColor:**
      QColor: color used for the grid if grid is specified
   **XaxisEnabled:**
      bool: specifies if the x axis should be drawn
   **YaxisEnabled:**
      bool: specifies if the y axis should be drawn
   **LegendEnabled:**
      bool: specifies if the legend should be drawn

--------------

caByte
~~~~~~

is the equivalent of the Byte Monitor in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caCamera
~~~~~~~~

has no equivalent in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caCalc
~~~~~~

has no equivalent in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caWaterfallPlot
~~~~~~~~~~~~~~~

has no equivalent in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caBitNames
~~~~~~~~~~

has no equivalent in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caTable
~~~~~~~

has no equivalent in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

**all graphical objects**
-------------------------

caLabel
~~~~~~~

is the equivalent of Text in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caGraphics
~~~~~~~~~~

is the equivalent of all primary graphical objects (like circles, lines,
arcs, triangles, arrows, ...) in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caFrame
~~~~~~~

has no equivalent of Text in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caImage
~~~~~~~

is the equivalent of image in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**
   Qt supports many graphics format, try yours to see if it will be
   displayed. The most interesting format is however the animated GIF
   format. These GIF image files may include multiple frames,
   transparency, and most other features supported by the GIF format. If
   the GIF image file has multiple frames, caQtDM can choose to animate
   the images or to display a specified frame. The default is to animate
   if there is more than one frame. To display a specific frame, enter
   an expression for the Image Calc that returns a frame number. Frame
   numbers start with 0. The `syntax <#CalcExpression>`__ for the Image
   Calc expression is the same as that for the Visibilty property, and
   it uses channels A-D in the Dynamic Attribute. The only difference is
   that the CALC expression should return a frame number, not True or
   False. The value will be rounded to the nearest integer. Frame
   numbers that are too high will use the last frame, and frame numbers
   that are too small will use the first frame. The Image will be black
   in EXECUTE mode if the Image Calc expression is invalid. The
   visibility of the Image is determined in the usual way if there is a
   channel specified. A multiple-frame Image even with no process
   variables specified for the Dynamic Attribute does update with some
   speed. Use the Image Calc expression and the process variables in the
   Dynamic Attribute to specify when to display each color. There is an
   `EPICS extension <#EPICSExtension>`__\ called `Simple GIF Animator
   (SGA) <https://web.archive.org/web/20221004231152/http://www.aps.anl.gov/epics/extensions/index.php>`__
   that allows you to create and modify animated GIFs for use in caQTDM.
   There are also many other GIF animators available, especially for
   Microsoft Windows. It does not make any difference which platform you
   use to create or manipulate the images. You cannot use SGA or most of
   the animators to create the images themselves, only to add, delete,
   and rearrange them plus modify some of the GIF parameters. There are
   many drawing and image editing programs that allow you to create and
   edit images. For UNIX, the `GNU Image Manipulation Program
   (GIMP) <https://web.archive.org/web/20221004231152/http://www.gimp.org/>`__\ is
   a good choice and is free.

--------------

caPolyLine
~~~~~~~~~~

is the equivalent of Polyline and Polygone in MEDM.

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caInclude
~~~~~~~~~

is the equivalent of the Composite in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:** Composite objects are created by the designer as a
   new ui file to be included in a main file. For many operations they
   are treated as one object. Its uses also visibility (Color Mode,
   Visibility, Visibility Calc, Channels A-D) properties. One can
   specify `macros <#MacroSubstitution>`__. you can specify them by
   placing a semi-colon after the file name and entering them as on the
   command line.

--------------

caDoubleTabWidget
~~~~~~~~~~~~~~~~~

has no equivalent in MEDM and is not a controls object

   `Geometry <geometry>`__ is used for any object
   **Description:**

**all controller objects**
--------------------------

caNumeric
~~~~~~~~~

is the equivalent of the Wheelswitch in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**
   The WheelSwitch has arrow buttons above and below the digits that are
   used to change individual digits, and it also accepts keyboard input.
   The up and down arrow buttons are the main feature of the
   WheelSwitch. You click them to increment the digit. The arrow buttons
   can also be navigated and operated via the keyboard. You have to put
   the focus on the selected digit, which get surrounded with a red
   color. The cursor has to remain inside the widget otherwise the focus
   will be lost. The up and down arrow keys increment or decrement the
   selected digit. Clicking on an arrow button will increment or
   decrement the digit. The WheelSwitch will also accept input of a new
   value via the keyboard by double clicking on the value. Escape aborts
   the input. Type Enter to finish the input.

--------------

caApplyNumeric
~~~~~~~~~~~~~~

is the equivalent of the Wheelswitch in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**
   The same behaviour as the Wheelswitch, with the difference that the
   value will be applied when pressing the apply button.

--------------

caSlider
~~~~~~~~

is the equivalent of the Wheelswitch in MEDM

   <`Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caShellCommand
~~~~~~~~~~~~~~

is the equivalent of the Shell command in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caMenu
~~~~~~

is the equivalent of the Menu in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caChoice
~~~~~~~~

is the equivalent of the Choice Button in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**

   +------------+--------------------------------------------------------+
   | column     | The buttons are arranged in a row. (This appears to be |
   |            | a mistake, which will not be corrected because of      |
   |            | existing screens.)                                     |
   +------------+--------------------------------------------------------+
   | row        | The buttons are arranged in a column. (This appears to |
   |            | be a mistake, which will not be corrected because of   |
   |            | existing screens.)                                     |
   +------------+--------------------------------------------------------+
   | row column | The buttons are automatically arranged in rows and     |
   |            | columns.                                               |
   +------------+--------------------------------------------------------+

--------------

caRelatedDisplay
~~~~~~~~~~~~~~~~

is the equivalent of the Related Display in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**

   ================== ===================================================
   create new display Create a new display and leave the current one.
   replace display    Create a new display that replaces the current one.
   ================== ===================================================

   +---------------------+-----------------------------------------------+
   | menu                | Use a pull down menu for the choices.         |
   +---------------------+-----------------------------------------------+
   | a row of buttons    | Use a row of buttons for the choices.         |
   +---------------------+-----------------------------------------------+
   | a column of buttons | Use a column of buttons for the choices.      |
   +---------------------+-----------------------------------------------+
   | invisible           | Do not show anything for the choices. This    |
   |                     | mode is intended to be used with a graphic or |
   |                     | other object on top of the related display.   |
   |                     | The graphic should make the operation of the  |
   |                     | Related Display clear. In EXECUTE mode, the   |
   |                     | `Execute-Mode Popup                           |
   |                     | Menu <#ExecuteModePopupMenu>`__ contains an   |
   |                     | item to toggle the makring of hidden buttons  |
   |                     | in case the user cannot find them.            |
   +---------------------+-----------------------------------------------+

--------------

caTextEntry
~~~~~~~~~~~

is the equivalent of the Text Entry in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caMessageButton
~~~~~~~~~~~~~~~

is the equivalent of the Message Button in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caToggleButton
~~~~~~~~~~~~~~

has no equivalent in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caScriptButton
~~~~~~~~~~~~~~

has no equivalent in MEDM

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------

caSpinBox
~~~~~~~~~

represents a simplified Wheelswitch

   `Geometry <geometry>`__ is used for any object
   **Description:**

--------------


`Requirements <requirements>`__
-------------------------------

| caQtDM requires that Qt, qwt and EPICS are installed. Actually the
  suported version are Qt4.6 and above, qwt6.0 and above. Qt5 needs at
  least qwt6.1. The synoptic editor (designer) is part of Qt and the
  caQtDM objects are plugged into the designer. This help can be called
  from the designer by pressing F1 on any object and requires the
  assistant which is a part of Qt. caQtDM runs in principle on any
  plattform and has been tested for several linux flavours and also on
  Microsoft Windows. For MAC OS probably some work has to be done. Qt
  requires actually on linux normally X11 on Microsoft Windows the
  native window system.
| caQtDM requires that Qt, qwt and EPICS are installed. Actually the
  suported version are Qt4.6 and above, qwt6.0 and above. Qt5 needs at
  least qwt6.1. The synoptic editor (designer) is part of Qt and the
  caQtDM objects are plugged into the designer. This help can be called
  from the designer by pressing F1 on any object and requires the
  assistant which is a part of Qt. caQtDM runs in principle on any
  plattform and has been tested for several linux flavours and also on
  Microsoft Windows. For MAC OS probably some work has to be done. Qt
  requires actually on linux normally X11 on Microsoft Windows the
  native window system.

`Command Line Options <commandline>`__
--------------------------------------

| caQtDM can be executed with the same options as MEDM; this to be
  compatible with its options. the following options are supported:
| [-help \| -h \| -‌], *gives a description of the options*
| [-x], *has no effect*
| [-attach], *will attach to a running caQtDM*
| [-noMsg], *will iconize the main window*
| [-noStyles], *no stylesheet will be loaded, works only when not
  attaching*
| [-print], *will print file and exit*
| [-noResize], *will prevent resizing, works only when not attaching*
| [-macro "*xxx*\ =\ *aaa*,\ *yyy*\ =\ *bbb*, ..."], *will apply macro
  substitution to replace occurrences of $(xxx\ ) with value aaa\ .*
| [-dg [*xpos*\ [x\ *ypos*]][+\ *xoffset*\ [+\ *yoffset*\ s]],
  *specifies the geometry (location and size) of the synoptic display*.
| Parameters in square brackets [] are optional.

Examples:
~~~~~~~~~

| Start up, performing macro substitution of all occurrences of $(a) and
  $(c) in the display file, xyz.ui:
| **caQtDM -macro "a=b,c=d" xyz.adl &**
| Start up in execute mode; resize the display window corresponding to
  abc.adl to 100 by 100 pixels and move it to the location, x = 100 and
  y = 100; and move the display window corresponding to def.adl to x =
  400 and y = 150:
| **caQtDM –dg 100x100+100+100 abc.ui&**

Description Files
-----------------------------

| With the Qt designer you produce xml files with the extension .ui.
  These files are used as input for the synoptic viewer caQtDM. These
  xml ascii file can be edited by hand, but care has to be taken while
  caQtDM and Qt designer do not forgive any error. Qt designer will
  however tell you at what line the mistake is produced.
| The files are normally loaded from the current directory and/or from
  the directory specified by the environment variable
  CAQTDM_DISPLAY_PATH. The directories are specified with the separator
  : for linux and ; for microsoft windows.

`Connection Problems and Access Rights <connection>`__
------------------------------------------------------

When caQtDM cannot connect the specified process variable normally white
colors will be drawn. A list of unconnected process variables can be
called through the menu of the main window of caQtDM. When a process
variable is not writeable the cursor will change to some special sign.

`Fonts <fonts>`__
-----------------

| The custom widgets make extensively use of the Qt font *"Lucida Sans
  Typewriter"*. If this font does not exist in some cases it will fall
  back to a font called "Monospace". However the "Lucida" font is really
  the appropriated font to display numbers and it is warmly advised to
  have this font installed.

**General Properties**
----------------------

.. _geometry:

.. rubric:: Geometry
   .. :name: geometry

.. container::

   All the widgets have geometry properties that can be set by the
   designer; these properties are normally at the top of the property
   sheet:

+------------+--------------------------------------------------------+
| X Position | x coordinate of the top left corner of the             |
|            | objectrelative to the display.                         |
+------------+--------------------------------------------------------+
| Y Position | y coordinate of the top left corner of the object      |
|            | relative to the display.                               |
+------------+--------------------------------------------------------+
| Width      | Width of the object.                                   |
+------------+--------------------------------------------------------+
| Height     | Height of the object.                                  |
+------------+--------------------------------------------------------+

.. rubric:: Visibility
   :name: visibility

.. container::

   For the `graphical objects <#graphics>`__ except caDoubleTabWidget
   which is not a control widget

+-----------------+---------------------------------------------------+
| Color Mode      | Color Mode for the object. only for caGraphics,   |
|                 | caLabel and caPolyLine, See the                   |
|                 | `ColorMode <#colormode>`__ property               |
+-----------------+---------------------------------------------------+
| Visibility      | Visibility mode for the object. See the           |
|                 | `Visibility <#visibilitymode>`__ property         |
+-----------------+---------------------------------------------------+
| Visibility Calc | A CALC expression that determines whether the     |
|                 | object is displayed or not. The expression should |
|                 | return 0 for False and anything else for True.    |
|                 | See `CALC Expression <#calcexpression>`__ .       |
+-----------------+---------------------------------------------------+
| Channel A       | Name of the main process variable associated with |
|                 | the object. An A in the CALC expression is        |
|                 | replaced by the value of this process variable,   |
|                 | and the values corresponding to G through L are   |
|                 | obtained from this process variable.              |
+-----------------+---------------------------------------------------+
| Channel B       | Name of the second process variable associated    |
|                 | with the object. A B in the CALC expression is    |
|                 | replaced by the value of this process variable.   |
+-----------------+---------------------------------------------------+
| Channel C       | Name of the third process variable associated     |
|                 | with the object. A C in the CALC expression is    |
|                 | replaced by the value of this process variable.   |
+-----------------+---------------------------------------------------+
| Channel D       | Name of the fourth process variable associated    |
|                 | with the object. A D in the CALC expression is    |
|                 | replaced by the value of this process variable.   |
+-----------------+---------------------------------------------------+

.. rubric:: Color Mode
   :name: color-mode

|

+--------+------------------------------------------------------------+
| static | Show the object in its normal colors.                      |
+--------+------------------------------------------------------------+
| alarm  | Show the object in alarm colors based on the severity of   |
|        | the associated process variable. The alarm colors are      |
|        | Green for NO_ALARM, Yellow for MINOR_ALARM, Red for        |
|        | MAJOR_ALARM, White for INVALID_ALARM, and Gray if the      |
|        | alarm has an unknown value (because of an error in the     |
|        | record, for example).                                      |
+--------+------------------------------------------------------------+

.. rubric:: Visibility Mode
   :name: visibility-mode

|

+-------------+-------------------------------------------------------+
| static      | The object is displayed always. Updating objects are  |
|             | always above non-updating objects.                    |
+-------------+-------------------------------------------------------+
| if not zero | The object is displayed if the process variable is    |
|             | not zero.                                             |
+-------------+-------------------------------------------------------+
| if zero     | The object is displayed if the object is zero.        |
+-------------+-------------------------------------------------------+
| calc        | Use calc expression to determine the visability.      |
+-------------+-------------------------------------------------------+

.. rubric:: Direction
   :name: direction

.. container::

   For the `Bar Monitor <#BarMonitor>`__ , `Byte
   Monitor <#ByteMonitor>`__ , `Scale Monitor <#ScaleIndicator>`__ ,
   and `Slider <#Slider>`__

+-------+-------------------------------------------------------------+
| up    | Direction for the object is up or up/down.                  |
+-------+-------------------------------------------------------------+
| right | Direction for the object is right or left/right.            |
+-------+-------------------------------------------------------------+
| down  | Only used in the Bar Monitor. Otherwise, MEDM treats this   |
|       | the same as up.                                             |
+-------+-------------------------------------------------------------+
| left  | Only used in the Bar Monitor. Otherwise, MEDM treats this   |
|       | the same as right.                                          |
+-------+-------------------------------------------------------------+

|

.. rubric:: Label
   :name: label

.. container::

   For the `Meter <#Meter>`__, `Bar Monitor <#BarMonitor>`__, `Scale
   Monitor <#ScaleIndicator>`__, and `Slider <#Slider>`__.

+----------------+----------------------------------------------------+
| none           | No extra features, except the limits are shown for |
|                | the meter. You can eliminate the limits on the     |
|                | meter by resizing it vertically so they do not     |
|                | show.                                              |
+----------------+----------------------------------------------------+
| no decorations | Same as none except for the Bar Monitor. For the   |
|                | Bar Monitor only the background and the bar show.  |
|                | This allows making bar graphs in MEDM.             |
+----------------+----------------------------------------------------+
| outline        | Show limits.                                       |
+----------------+----------------------------------------------------+
| limits         | Show limits and a box for the value (except there  |
|                | is no box for the Slider).                         |
+----------------+----------------------------------------------------+
| channel        | Show limits, a box for the value, and the process  |
|                | variable name (except there is no box for the      |
|                | Slider).                                           |
+----------------+----------------------------------------------------+

.. rubric:: Text Align
   :name: text-align

|

+-----------------+---------------------------------------------------+
| horiz. left     | Align the text at the top left of the object.     |
|                 | (Capital letters will line up with the top of the |
|                 | object, and the text will start at the left of    |
|                 | the object.)                                      |
+-----------------+---------------------------------------------------+
| horiz. centered | Align the text at the top center of the object.   |
|                 | (Capital letters will line up with the top of the |
|                 | object, and the text will be horizontally         |
|                 | centered in the object.)                          |
+-----------------+---------------------------------------------------+
| horiz. right    | Align the text at the top right of the object.    |
|                 | (Capital letters will line up with the top of the |
|                 | object, and the text will end at the right of the |
|                 | object.)                                          |
+-----------------+---------------------------------------------------+
| vert. top       | No longer used. In ADL files MEDM treats this the |
|                 | same as horiz. left (as it used to) for backward  |
|                 | compatibility.                                    |
+-----------------+---------------------------------------------------+
| vert. centered  | No longer used. In ADL files MEDM treats this the |
|                 | same as horiz. center (as it used to) for         |
|                 | backward compatibility.                           |
+-----------------+---------------------------------------------------+
| vert. bottom    | No longer used. In ADL files MEDM treats this the |
|                 | same as horiz. right (as it used to) for backward |
|                 | compatibility.                                    |
+-----------------+---------------------------------------------------+

.. rubric:: Text Format
   :name: text-format

.. container::

   For the `Text Entry <#TextEntry>`__ and `Text
   Monitor <#TextIndicator>`__ . For all of the formats, the result
   depends on the number itself and the precision as returned from
   channel access for the process variable. The precision is
   typically the PREC field for the associated record. The PREC field
   is only determined when the process variable is connected, so
   subsequent changes to the PREC do not affect the format until the
   synoptic display is reloaded.

.. rubric:: CALC Expressions
   :name: calc-expressions

.. container::

   CALC expressions are used to determine visibility for graphical
   objects and to determine the frame number for the
   `caImage <#caImage>`__ , which also has a Dynamic Attribute. For
   visibility the expression should return 0 for False and anything
   else for True. For the frame number for the Image it should return
   a number, which will be rounded to the nearest integer. Frame
   numbers start with 0. The complete syntax of the CALC expression
   is described in detail in the Record Reference Manual found in the
   `Epics
   Documentation <https://web.archive.org/web/20221004231152/http://www.aps.anl.gov/epics>`__
   under IOC software for any version of EPICS base, through at least
   Base 3.13. It is a case-independent C expression that can include
   functions such as ABS, SQR, MIN, MAX, CEIL, FLOOR, LOG, LOGE, EXP,
   SIN, SINH, ASIN, COS, COSH, ACOS, TAN, TANH, and ATAN, as well as
   the usual C operators, except that != is replaced by # and == is
   replaced by =. The letters A-L obtain their values from the
   Channels A-D and are replaced as follows.

= ===========================
A The value of Channel A.
B The value of Channel B.
C The value of Channel C.
D The value of Channel D.
E Reserved.
F Reserved.
G The COUNT of Channel A.
H The HOPR of Channel A.
I The STATUS of Channel A.
J The SEVERITY of Channel A.
K The PRECISION of Channel A.
L The LOPR of Channel A.
= ===========================

.. container::

   Examples are:
   Show the object whenever the value of Channel A is zero: !A
   Show the object whenever the value of Channel A is not zero: A
   Show the object whenever the value of Channel A is 12: A=12
   Show the object whenever the value of Channel A is not 12: A#12
   Show the object whenever the values of Channel A, Channel B, and
   Channel C are all negative: A<0&&B<0&&C<0
   Show the object whenever the main process variable is within 90%
   of its HOPR or LOPR limits: A<.9*L||A>.9*H
   Show the object whenever the SEVERITY of the main process variable
   is not 0: !J

Macro Substitution
~~~~~~~~~~~~~~~~~~

| Strings of the form $(name) in an ADL file can be replaced by some
  other string, both on the command line and when calling a `related
  display <#RelatedDisplay>`__. Specific directions for each of these
  cases are given in the correspoonding sections of the manual. In
  general, there is an argument string of the form
  “name1=value1[,name2=value2]…”. All occurrences of “$(name1)” in the
  ADL file are replaced with “value1”, then all occurences of $(name2)
  are replaced by value2, *etc*. The substitition is recursive; that is,
  if value1 contains an occurrence of $(name2), then when name2=value2
  is processed, that occurrence will be replaced by value2.

Drag-And-Drop
~~~~~~~~~~~~~

you may drag and drop an item to some destination, however there is no
drag and drop possible between Qt and X11. When using KDE you may drag
and drop to a konsole. For simple cut and paste you can call the pv info
box and get the variable you want to paste.

Main Window
~~~~~~~~~~~

| The main window of caQTDM present messages, a menu bar and a status
  bar.
| The menu bar has the following items:

+------------+-----------+-------------------------------------------+
| Menu       | Open File | calls a dialog box for opening a .ui or   |
|            |           | .prc file (.prc files represent psi       |
|            |           | special ascii files for rapid             |
|            |           | prototyping)                              |
+------------+-----------+-------------------------------------------+
|            | Reload    | will close and reload all displays; very  |
|            |           | handy during editing                      |
+------------+-----------+-------------------------------------------+
|            | Exit      | will exit caQTDM                          |
+------------+-----------+-------------------------------------------+
|            | About     | gives some information about the build    |
|            |           | and author                                |
+------------+-----------+-------------------------------------------+
| PV         |           | will display a list of unconnected PV's   |
+------------+-----------+-------------------------------------------+
| UpdataType | Direct    | Wenn caQtDM is in this mode, all the      |
|            |           | monitors will be displayed as soon as     |
|            |           | they come                                 |
+------------+-----------+-------------------------------------------+
|            | Timed     | Wenn caQtDM is in this mode, all the      |
|            |           | monitors will be displayed will be        |
|            |           | displayed with a highest rate of 5Hz,     |
|            |           | however this rate can be set on a         |
|            |           | individual base by a JSON string after    |
|            |           | the channel (in designer) with the        |
|            |           | following syntax                          |
|            |           | channel{"monitor":{"maxdisplayrate":20}}, |
|            |           | where you can choose your display rate.   |
+------------+-----------+-------------------------------------------+

|
| the Status bar will display the following information: memory used by
  caQtDM, the number of connected and unconnected PV's, the number of
  monitors per second, the number of displays per second and the monitor
  with the highest rate.

Context Menu
~~~~~~~~~~~~

By pressing the right mouse button over an object you will get a context
menu. This menu contains at least an item called PV info in order to get
information about all the process variables associated with that object.
For some other objects you will find other items for example for
changing axes of the caStripplot and caCartesianplot or for interaction
with caCamera.

Context Menu Customization
~~~~~~~~~~~~~~~~~~~~~~~~~~

| The context menu can be configured by adding other items with their
  actions. This configuration is specified in the CAQTDM_EXEC_LIST
  environment variable. This variable is of the form: name1;
  program1[[:name2; program2]…].The items between colons represent each
  menu item. All of the characters up to the first semi-colon in each
  item are the text that appears on the menu. The remaining characters
  represent the system command that will be executed when the item is
  selected. The system command can include the following special
  characters:

+----+----------------------------------------------------------------+
| &P | The process variable will be automatically taken from the      |
|    | object.                                                        |
+----+----------------------------------------------------------------+
| &A | The full path name of the ui file associated with the display. |
+----+----------------------------------------------------------------+
| &T | The short name of the file.                                    |
+----+----------------------------------------------------------------+
| &X | The X window id of the ui screen. This can be used with        |
|    | commands like Xwd, for example. only for linux                 |
+----+----------------------------------------------------------------+

|
| Examples are:
| export CAQTDM_EXEC_LIST='Probe;probe &P &:PV Name(s);echo &P'
| export MEDM_EXEC_LIST='Full Name;echo &A:Short Name;echo &T'
| export MEDM_EXEC_LIST='XTerm;xterm -fg black -bg white &'
| export CAQTDM_EXEC_LIST=' Dump;xwd -id &X \| xwdtopnm \| pnmtops \|
  lpr &'

Help
~~~~

You may obtain help for any widget in the designer by pressing F1. For
the caQtDM custom widgets the help file generated from this html file
should be registered by the Qt assistant.

Resizing Displays
~~~~~~~~~~~~~~~~~

| Displays With caQtDM you can resize the synoptic display windows
  without using Qt layouts. You can build a display normally and
  everything will be resized when you resize the window. Furthermore you
  could try Qt designer layouts (for experts only)

Printing
~~~~~~~~

| By pressing the right mouse button on the background of your synoptic
  display you may get a context menu with the item "Print". Normally you
  should get a print dialog.

Environment Variables
~~~~~~~~~~~~~~~~~~~~~

MEDM uses the following environment variables:

+---------------------+-----------------------------------------------+
| CAQTDM_DISPLAY_PATH | A colon-separated (semi-colon-separated on    |
|                     | Mircosoft Windows) list of directories in     |
|                     | which to look for display files. Only looks   |
|                     | in the current working directory if not       |
|                     | specified. Related Displays have to be in     |
|                     | your current directory or in this path        |
+---------------------+-----------------------------------------------+
| CAQTDM_EXEC_LIST    | A list of commands for the Context Menu . See |
|                     | the `Context Menu <#contextcustomize>`__ for  |
|                     | the format.                                   |
+---------------------+-----------------------------------------------+

|
