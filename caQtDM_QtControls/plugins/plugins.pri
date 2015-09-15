include(../../caQtDM_Viewer/qtdefs.pri)

contains(QT_VER_MAJ, 4) {
      CONFIG += plugin qt thread warn_on
      CONFIG += designer
}
contains(QT_VER_MAJ, 5) {
      CONFIG += plugin qt thread warn_on
      QT += widgets uitools
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

TEMPLATE = lib

ios | android {
  CONFIG += static
  LIBS += $$(QWTHOME)/lib/qwt.a
  LIBS += ../$(CAQTDM_COLLECT)/libqtcontrols.a
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
	     CONFIG(DebugBuild, DebugBuild|ReleaseBuild) { 
                     INCLUDEPATH += $(QWTINCLUDE)
		     LIBS += $$(QWTHOME)/lib/qwtd.lib
		     LIBS += $(CAQTDM_COLLECT)/debug/qtcontrols.lib
		     DESTDIR = $(CAQTDM_COLLECT)/debug/designer
		     
	     }

	     CONFIG( ReleaseBuild, DebugBuild|ReleaseBuild) {
	             INCLUDEPATH += $(QWTINCLUDE)
		     LIBS += $$(QWTHOME)/lib/qwt.lib
		     LIBS += $(CAQTDM_COLLECT)/qtcontrols.lib
		     DESTDIR = $(CAQTDM_COLLECT)/designer
		    
	     }
     }  
}

unix {
   DESTDIR = $(CAQTDM_COLLECT)/designer
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
      LIBS += -L $(CAQTDM_COLLECT) -lqtcontrols
      QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
   }
}

INCLUDEPATH += ../src
INCLUDEPATH += .
