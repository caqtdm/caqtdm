include(../../caQtDM_Viewer/qtdefs.pri)

contains(QT_VER_MAJ, 4) {
      CONFIG += plugin qt thread warn_on
      CONFIG += designer
}
contains(QT_VER_MAJ, 5) {
      CONFIG += plugin qt thread warn_on
      QT += widgets uitools opengl
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
      CONFIG += plugin qt thread warn_on
      QT += widgets uitools opengl
      !MOBILE {
        QT += designer
      }
}
TEMPLATE = lib

ios | android {
  CONFIG += static
  LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME).a
  LIBS += ../$(CAQTDM_COLLECT)/libqtcontrols.a
  INCLUDEPATH += $(QWTINCLUDE)
  INCLUDEPATH += $$(QWTHOME)/src
  INCLUDEPATH += $(QTHOME)/include
  MOC_DIR = moc
  OBJECTS_DIR = obj
}


win32 {
     INCLUDEPATH += $$(QWTHOME)/src
     
     win32-g++ {
             LIBS += $$(QWTLIB)/lib/lib$$(QWTLIBNAME).a
	     LIBS += $$(QTCONTROLS_LIBS)/release/libqtcontrols.a
     }
     win32-msvc* || msvc{
	     CONFIG(DebugBuild, DebugBuild|ReleaseBuild) { 
                     INCLUDEPATH += $(QWTINCLUDE)
                     LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME)d.lib
		     LIBS += $(CAQTDM_COLLECT)/debug/qtcontrols.lib
		     DESTDIR = $(CAQTDM_COLLECT)/debug/designer
		     
	     }

	     CONFIG( ReleaseBuild, DebugBuild|ReleaseBuild) {
	             INCLUDEPATH += $(QWTINCLUDE)
                     LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME).lib
		     LIBS += $(CAQTDM_COLLECT)/qtcontrols.lib
		     DESTDIR = $(CAQTDM_COLLECT)/designer
		    
	     }
     }  
}

unix:!ios {
   DESTDIR = $(CAQTDM_COLLECT)/designer
   unix {
     INCLUDEPATH += $(QWTINCLUDE)
     MOC_DIR = moc
     OBJECTS_DIR = obj
   }

   unix:!macx {
      LIBS += -L$(QWTLIB) -Wl,-rpath,$(QWTLIB) -l$$(QWTLIBNAME)
      LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
   }

   macx: {
      INCLUDEPATH += $(QWTINCLUDE)
      QMAKE_LFLAGS_PLUGIN -= -dynamiclib
      QMAKE_LFLAGS_PLUGIN += -bundle
      LIBS += -F$(QWTLIB) -framework $$(QWTLIBNAME)
      LIBS += -L $(CAQTDM_COLLECT) -lqtcontrols

   }

}

INCLUDEPATH += ../../caQtDM_Lib/src
INCLUDEPATH += ../src
INCLUDEPATH += .
