CAQTDM_TOP = ../..
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
CONFIG += Define_Build_caQtDM_QtControls
CONFIG += caQtDM_Installation
INSTALLTIONSUBDIR = designer
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_internal_Libraries.pri)

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

TEMPLATE = lib

ios | android {
  CONFIG += static
  LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME).a
  LIBS += ../$(CAQTDM_COLLECT)/libqtcontrols.a
  INCLUDEPATH += $(QWTINCLUDE)
  INCLUDEPATH += $$(QWTHOME)/src
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
	     CONFIG += Define_Build_OutputDir
	     INCLUDEPATH += $(QWTINCLUDE)
	     CONFIG(DebugBuild, DebugBuild|ReleaseBuild) { 
                     INCLUDEPATH += $(QWTINCLUDE)
                     LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME)d.lib
                    #LIBS += $(CAQTDM_COLLECT)/debug/qtcontrols.lib
	     }

	     ReleaseBuild {
	     CONFIG( ReleaseBuild, DebugBuild|ReleaseBuild) {
	             INCLUDEPATH += $(QWTINCLUDE)
                     LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME).lib
                     #LIBS += $(CAQTDM_COLLECT)/qtcontrols.lib
	     }
     }  
}

Define_Build_OutputDir {
   
   win32 {
        DebugBuild {
            
		caqtdm_dll.files = release/*.dll
		caqtdm_dll.path = $$(CAQTDM_COLLECT)/designer/debug
		caqtdm_lib.files = release/*.lib
		caqtdm_lib.path = $$(CAQTDM_COLLECT)/designer/debug
		caqtdm_exe.files = release/*.exe
		caqtdm_exe.path = $$(CAQTDM_COLLECT)/designer/debug
		INSTALLS += caqtdm_dll caqtdm_lib caqtdm_exe

        }
        ReleaseBuild {
        	message("Plugin Release Installation")
            	caqtdm_dll.files = release/*.dll
            	caqtdm_dll.path = $$(CAQTDM_COLLECT)/designer
		caqtdm_lib.files = release/*.lib
		caqtdm_lib.path = $$(CAQTDM_COLLECT)/designer
		caqtdm_exe.files = release/*.exe
		caqtdm_exe.path = $$(CAQTDM_COLLECT)/designer
		INSTALLS += caqtdm_dll caqtdm_lib caqtdm_exe
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
      LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
   }

   macx: {
      INCLUDEPATH += $(QWTINCLUDE)
      QMAKE_LFLAGS_PLUGIN -= -dynamiclib
      QMAKE_LFLAGS_PLUGIN += -bundle
      LIBS += -F$(QWTLIB) -framework $$(QWTLIBNAME)
      LIBS += -L $(CAQTDM_COLLECT) -lqtcontrols
      QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
   }
}

INCLUDEPATH += ../../caQtDM_Lib/src
INCLUDEPATH += ../src
INCLUDEPATH += .
}
