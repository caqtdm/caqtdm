include(../../caQtDM_Viewer/qtdefs.pri)

DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG += release

contains(QT_VER_MAJ, 4) {
   CONFIG += designer plugin qt thread warn_on
}

contains(QT_VER_MAJ, 5) {
   CONFIG += plugin qt thread warn_on
   QT += designer widgets
}

TEMPLATE = lib

ios {
  CONFIG += static
  LIBS += $$(QWTHOME)/lib/qwt.a
  LIBS += ../../caQtDM_Binaries/libqtcontrols.a
  INCLUDEPATH += $(QWTINCLUDE)
  INCLUDEPATH += $$(QWTHOME)/src
  MOC_DIR = moc
  OBJECTS_DIR = obj
}

win32 {
     INCLUDEPATH += $$(QWTHOME)/src
     
     win32-g++ {
	     LIBS += $$(QWTLIB)/lib/libqwt.a
	     LIBS += $$(QTCONTROLS_LIBS)/release/libqtcontrols.a
     }
     win32-msvc* {
	     DebugBuild {
                     INCLUDEPATH += $(QWTINCLUDE)
		     LIBS += $$(QWTHOME)/lib/qwtd.lib
		     LIBS += $$(QTCONTROLS_LIBS)/debug/qtcontrols.lib
	     }

	     ReleaseBuild {
                     INCLUDEPATH += $(QWTINCLUDE)
		     LIBS += $$(QWTHOME)/lib/qwt.lib
		     LIBS += $$(QTCONTROLS_LIBS)/release/qtcontrols.lib
	     }
     }  
}

!ios {
   unix {
     INCLUDEPATH += $(QWTINCLUDE)
     MOC_DIR = moc
     OBJECTS_DIR = obj
   }

   unix:!macx {
      LIBS += -L$(QWTLIB) -Wl,-rpath,$(QWTLIB) -lqwt
      LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
   }

   macx: {
      INCLUDEPATH += $(QWTINCLUDE)
      QMAKE_LFLAGS_PLUGIN -= -dynamiclib
      QMAKE_LFLAGS_PLUGIN += -bundle
      LIBS += -F$(QWTLIB) -framework qwt
      LIBS += -L $(QTBASE) -lqtcontrols
      QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
   }
}

INCLUDEPATH += ../src
INCLUDEPATH += .
