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
  INCLUDEPATH += $$(QWTHOME)/include
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
	     	     INCLUDEPATH += $$(QWTHOME)/include
		     LIBS += $$(QWTHOME)/lib/qwtd.lib
		     LIBS += $$(QTCONTROLS_LIBS)/debug/qtcontrols.lib
	     }

	     ReleaseBuild {
	             INCLUDEPATH += $$(QWTHOME)/include
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
      INCLUDEPATH += $$(QTHOME)/include 
      QMAKE_LFLAGS_PLUGIN -= -dynamiclib
      QMAKE_LFLAGS_PLUGIN += -bundle
      LIBS += -F$(QWTLIB) -framework qwt
      LIBS += -L $(QTBASE) -lqtcontrols
      QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
   }
}

INCLUDEPATH += ../src
INCLUDEPATH += .

configA {
   SOURCES	+= qtcontrols_monitors_plugin.cpp
   HEADERS	+= qtcontrols_monitors_plugin.h
   RESOURCES += qtcontrolsplugin.qrc
   TARGET = qtcontrols_monitors_plugin
   QMAKE_CLEAN += libqtcontrols_monitors_plugin*

   !ios {
      unix:!macx {
         QMAKE_POST_LINK = cp libqtcontrols_monitors_plugin.so $(QTBASE)/designer
      }
      macx: {
         QMAKE_POST_LINK = cp libqtcontrols_monitors_plugin.dylib $(QTBASE)/designer
      }
   }

   ios {
      QMAKE_POST_LINK = cp libqtcontrols_monitors_plugin.a ../../caQtDM_Binaries/
   }

   win32 {
      ReleaseBuild {
        QMAKE_POST_LINK = copy /Y .\release\qtcontrols_monitors_plugin.dll ..\..\caQtDM_Binaries\designer
      }
    }
}

configB {
    SOURCES	+= qtcontrols_controllers_plugin.cpp
    HEADERS	+= qtcontrols_controllers_plugin.h
    RESOURCES += qtcontrolsplugin.qrc
    TARGET = qtcontrols_controllers_plugin
    QMAKE_CLEAN += libqtcontrols_controllers_plugin*

   !ios {
      unix:!macx  {
         QMAKE_POST_LINK = cp libqtcontrols_controllers_plugin.so $(QTBASE)/designer
      }
      macx: {
         QMAKE_POST_LINK = cp libqtcontrols_controllers_plugin.dylib $(QTBASE)/designer
      }
  }

  ios {
      QMAKE_POST_LINK = cp libqtcontrols_controllers_plugin.a ../../caQtDM_Binaries/
  }

  win32 {
    ReleaseBuild {
       QMAKE_POST_LINK = copy /Y .\release\qtcontrols_controllers_plugin.dll ..\..\caQtDM_Binaries\designer
     }
  }
}

configC {
    SOURCES	+= qtcontrols_graphics_plugin.cpp
    HEADERS	+= qtcontrols_graphics_plugin.h
    RESOURCES += qtcontrolsplugin.qrc
    TARGET = qtcontrols_graphics_plugin
    QMAKE_CLEAN += libqtcontrols_graphics_plugin*

    !ios {
       unix:!macx {
          QMAKE_POST_LINK = cp libqtcontrols_graphics_plugin.so $(QTBASE)/designer
       }
       macx: {
          QMAKE_POST_LINK = cp libqtcontrols_graphics_plugin.dylib $(QTBASE)/designer
       }
    }

    ios {
        QMAKE_POST_LINK = cp libqtcontrols_graphics_plugin.a ../../caQtDM_Binaries/    
    }

    win32 {
       ReleaseBuild {
         QMAKE_POST_LINK = copy /Y .\release\qtcontrols_graphics_plugin.dll ..\..\caQtDM_Binaries\designer
       }
    }
}
