include(../caQtDM/qtdefs.pri)

DEFINES += QT_NO_DEBUG_OUTPUT

CONFIG += uitools qwt plugin designer warn_on

TARGET = qtcontrols
TEMPLATE = lib

DEFINES += QTCONTROLS_LIB_LIBRARY

win32 {

    win32-g++ {
      INCLUDEPATH = $${QWTHOME}/src
      LIBS += $${QWTHOME}/lib/libqwt.a
     }
     win32-msvc* {
        DEFINES += QTCON_MAKEDLL _CRT_SECURE_NO_WARNINGS
        DebugBuild {
                INCLUDEPATH = $${QWTHOME}/include
                LIBS += $${QWTHOME}/lib/qwtd.lib
        }

        ReleaseBuild {
        	INCLUDEPATH = $${QWTHOME}/include
                LIBS += $${QWTHOME}/lib/qwt.lib
                QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\qtcontrols.dll $${BINARY_LOCATION}
        }
     }
}

unix {
    QWTLIB = $(QWTHOME)/lib
    INCLUDEPATH = $(QWTHOME)/src
    INCLUDEPATH += src/ /usr/include/X11
    LIBS += -L$${QWTLIB} -Wl,-rpath,$${QWTLIB} -lqwt
    MOC_DIR = moc
    OBJECTS_DIR = obj
    DESTDIR = .
    QMAKE_POST_LINK = cp libqtcontrols.so ../caQtDM_Binaries/
    QMAKE_CXXFLAGS += "-g"
    QMAKE_CFLAGS_RELEASE += "-g"
}

INCLUDEPATH += src
RESOURCES = qtcontrols.qrc

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
    src/qwt_thermo_marker.cpp\
    src/caslider.cpp \
    src/cacartesianplot.cpp \
    src/castripplot.cpp \
    src/cacamera.cpp \
    src/imagewidget.cpp \
    src/cacalc.cpp \
    src/capolylinetaskmenu.cpp \
    src/capolylinedialog.cpp \
    src/parsepepfile.cpp \
    src/catogglebutton.cpp \
    src/cascriptbutton.cpp \
    src/cadoubletabwidget.cpp \
    src/cadoubletabwidgetextension.cpp \
    src/cadoubletabwidgetextensionfactory.cpp

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
    src/qwt_thermo_marker.h\
    src/caslider.h \
    src/castripplot.h \
    src/cacartesianplot.h \
    src/cacamera.h \
    src/imagewidget.h \
    src/cacalc.h \
    src/capolylinetaskmenu.h \
    src/capolylinedialog.h \
    src/qtcontrols_global.h \
    src/alarmdefs.h \
    src/parsepepfile.h \
    src/catogglebutton.h \
    src/cascriptbutton.h \
    src/cadoubletabwidget.h \
    src/cadoubletabwidgetextension.h \
    src/cadoubletabwidgetextensionfactory.h

OTHER_FILES += README
