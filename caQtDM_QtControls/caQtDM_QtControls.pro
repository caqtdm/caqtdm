TARGET_PRODUCT = "Widgets for Display Manager"
TARGET_FILENAME = "qtcontrols.dll"

include(../caQtDM_Viewer/qtdefs.pri)
CONFIG += caQtDM_QtControls caQtDM_xdl2ui_Lib
include(../caQtDM.pri)

#DEFINES += QT_NO_DEBUG_OUTPUT

contains(QT_VER_MAJ, 4) {
      CONFIG += qwt plugin thread uitools network
      CONFIG += designer
}
contains(QT_VER_MAJ, 5) {
      QT += widgets concurrent uitools opengl network
      CONFIG  += qwt plugin
      DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000
      ios | android {
         greaterThan(QT_MINOR_VERSION, 4) {
            QT += uiplugin
         } else {
            QT += designer
         }
      } else {
          QT += designer
      }
}
contains(QT_VER_MAJ, 6) {
      QT += widgets concurrent uitools opengl core
      CONFIG  += plugin
      ios | android {
            QT += uiplugin
      }else {
        QT += designer
      }
}
CONFIG += warn_on
CONFIG += console

TARGET = qtcontrols
TEMPLATE = lib
MOC_DIR = moc
INCLUDEPATH += src
INCLUDEPATH += ../caQtDM_Lib/src
INCLUDEPATH += ../caQtDM_Parsers/adlParserSrc
INCLUDEPATH += ../caQtDM_Parsers/edlParserSrc

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}

RESOURCES = qtcontrols.qrc
RC_FILE = ./src/qtcontrols.rc

#vtune study
#INCLUDEPATH += /opt/intel/parallel/vtune_amplifier_xe_2013/include
#LIBS += /opt/intel/parallel/vtune_amplifier_xe_2013/lib32/libittnotify.a


PRE_TARGETDEPS += \
     moc/moc_caslider.cpp \
     moc/moc_cacartesianplot.cpp \
     moc/moc_castripplot.cpp \
     moc/moc_cameter.cpp \
     moc/moc_caclock.cpp

contains(QWT_VER_MIN, 1)|contains(QWT_VER_MIN, 2) {
  PRE_TARGETDEPS += moc/moc_qwt_thermo_marker_61.cpp
}

SOURCES	+= \
    src/caframe.cpp \
    src/cainclude.cpp \
    src/caimage.cpp \
    src/cagraphics.cpp \
    src/capolyline.cpp \
    src/searchfile.cpp \
    src/elabel.cpp \
    src/esimplelabel.cpp \
    src/fontscalingwidget.cpp \
    src/calabel.cpp \
    src/imagepushbutton.cpp \
    src/epushbutton.cpp \
    src/enumeric.cpp \
    src/leftclick_with_modifiers_eater.cpp \
    src/number_delegate.cpp \
    src/dec_int_from_format.cpp \
    src/eapplybutton.cpp \
    src/camenu.cpp \
    src/camessagebutton.cpp \
    src/cashellcommand.cpp \
    src/carowcolmenu.cpp \
    src/carelateddisplay.cpp \
    src/caapplynumeric.cpp \
    src/canumeric.cpp \
    src/eapplynumeric.cpp \
    src/cachoice.cpp \
    src/catextentry.cpp \
    src/calineedit.cpp \
    src/caled.cpp \
    src/eled.cpp \
    src/cabitnames.cpp \
    src/eflag.cpp \
    src/catable.cpp \
    src/cabyte.cpp \
    src/rectangle.cpp \
    src/cagauge.cpp \
    src/egauge.cpp \
    src/eng_notation.cpp \
    src/cathermo.cpp \
    src/caslider.cpp \
    src/cacartesianplot.cpp \
    src/castripplot.cpp \
    src/cacamera.cpp \
    src/imagewidget.cpp \
    src/cacalc.cpp \
    src/parsepepfile.cpp \
    src/catogglebutton.cpp \
    src/cascriptbutton.cpp \
    src/cadoubletabwidget.cpp \
    src/stripplotthread.cpp \
    src/cawaterfallplot.cpp \
    src/snumeric.cpp \
    src/caspinbox.cpp \
    src/qwtplotcurvenan.cpp \
    src/cawavetable.cpp \
    src/specialFunctions.cpp \
    src/caclock.cpp \
    src/cameter.cpp \
    src/colormaps.cpp \
    src/cascan2d.cpp \
    src/mdaReader.cpp \
    src/mda_loader.c \
    src/messageQueue.cpp \
    src/cabytecontroller.cpp \
    src/calabelvertical.cpp \
    src/camultilinestring.cpp \
    src/camimedisplay.cpp \
    src/calinedraw.cpp \
    src/wmsignalpropagator.cpp \
    src/replacemacro.cpp \
    src/JSON.cpp \
    src/JSONValue.cpp \
    src/textedit.cpp

ADL_EDL_FILES {
    SOURCES	+= src/parseotherfile.cpp
}

XDR_HACK {
    SOURCES += src/xdr_hack.c
    HEADERS += src/xdr_hack.h
}

!MOBILE {
    SOURCES +=  src/pvtaskmenu.cpp src/pvdialog.cpp
    HEADERS +=  src/pvtaskmenu.h src/pvdialog.h
    SOURCES +=  src/cadoubletabwidgetextensionfactory.cpp  src/cadoubletabwidgetextension.cpp
    SOURCES +=  src/capolylinetaskmenu.cpp src/capolylinedialog.cpp
    HEADERS +=  src/cadoubletabwidgetextension.h
    HEADERS +=  src/cadoubletabwidgetextensionfactory.h  src/capolylinetaskmenu.h
}

QT += network
HEADERS += src/networkaccess.h src/fileFunctions.h \
    src/calinedraw.h \
    src/plotHelperClasses.h \
    src/wmsignalpropagator.h \
    src/replacemacro.h \
    src/JSON.h \
    src/JSONValue.h \
    src/networkmodel.h \
    src/textedit.h
SOURCES += src/networkaccess.cpp src/fileFunctions.cpp

contains(QWT_VER_MIN, 0) {
   HEADERS	+= src/qwt_thermo_marker.h
   SOURCES	+= src/qwt_thermo_marker.cpp
}
contains(QWT_VER_MIN, 1)|contains(QWT_VER_MIN, 2) {
   HEADERS	+= src/qwt_thermo_marker_61.h
   SOURCES	+= src/qwt_thermo_marker_61.cpp
}

HEADERS	+= \
    src/caframe.h \
    src/cainclude.h \
    src/caimage.h \
    src/cagraphics.h \
    src/capolyline.h \
    src/searchfile.h \
    src/elabel.h \
    src/esimplelabel.h \
    src/fontscalingwidget.h \
    src/calabel.h \
    src/imagepushbutton.h \
    src/epushbutton.h \
    src/enumeric.h \
    src/number_delegate.h \
    src/leftclick_with_modifiers_eater.h \
    src/eapplybutton.h \
    src/dec_int_from_format.h \
    src/camenu.h \
    src/camessagebutton.h \
    src/cashellcommand.h \
    src/carowcolmenu.h \
    src/carelateddisplay.h \
    src/canumeric.h \
    src/caapplynumeric.h \
    src/eapplynumeric.h \
    src/cachoice.h \
    src/catextentry.h \
    src/calineedit.h \
    src/caled.h \
    src/eled.h \
    src/cabitnames.h \
    src/eflag.h \
    src/catable.h \
    src/cabyte.h \
    src/rectangle.h \
    src/cagauge.h \
    src/egauge.h \
    src/eng_notation.h \
    src/cathermo.h \
    src/caslider.h \
    src/castripplot.h \
    src/cacartesianplot.h \
    src/cacamera.h \
    src/imagewidget.h \
    src/cacalc.h \
    src/qtcontrols_global.h \
    src/alarmdefs.h \
    src/parsepepfile.h \
    src/catogglebutton.h \
    src/cascriptbutton.h \
    src/cadoubletabwidget.h \
    src/stripplotthread.h \
    src/cawaterfallplot.h \
    src/snumeric.h \
    src/caspinbox.h \
    src/qwtplotcurvenan.h \
    src/cawavetable.h \
    src/capolylinedialog.h \
    src/specialFunctions.h \
    src/caclock.h \
    src/cameter.h \
    src/colormaps.h \
    src/cascan2d.h \
    src/mdaReader.h \
    src/mda-load.h \
    src/messageQueue.h \
    src/cabytecontroller.h \
    src/qtdefinitions.h \
    src/pathdefinitions.h \
    src/camultilinestring.h \
    src/formattype.h \
    src/calabelvertical.h \
    src/camimedisplay.h \
    src/caWidgetInterface.h \
    src/addevent.h \
    src/animationcode.h \
    src/hideobjectcode.h

ADL_EDL_FILES {
    HEADERS	+= src/parseotherfile.h
}

OTHER_FILES += README
