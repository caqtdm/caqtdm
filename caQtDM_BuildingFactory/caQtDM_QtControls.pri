
#==========================================================================================================
caQtDM_QtControls {
        unix:!macx:!ios:!android {
                message("caQtDM_QtControls configuration : unix:!macx:!ios:!android")
 		OBJECTS_DIR = obj
		DESTDIR = $$(CAQTDM_COLLECT)
 		INCLUDEPATH += $$(QWTINCLUDE)
                LIBS += -L$$(QWTLIB) -Wl,-rpath,$(QWTLIB) -l$$(QWTLIBNAME)
  	}

        macx {
                message("caQtDM_QtControls configuration : macx")
 		INCLUDEPATH += $$(QWTINCLUDE)
    		OBJECTS_DIR = obj
		DESTDIR = $$(CAQTDM_COLLECT)
    		CONFIG += lib_bundle
                LIBS += -F$$(QWTLIB) -framework $$(QWTLIBNAME)
                        ADL_EDL_FILES {
                           LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -ladlParser
                           LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -ledlParser
                        }
  	}

	ios | android {
                message("caQtDM_QtControls configuration : ios or android")
		OBJECTS_DIR = obj
		INCLUDEPATH += $$(QWTINCLUDE)
                CONFIG += staticlib
                CONFIG += release
                CONFIG -= debug
                CONFIG -= debug_and_release
                android {
                   DESTDIR = $(CAQTDM_COLLECT)
                }
        }

	win32 {
                message("caQtDM_QtControls configuration : win32")
		message($$CONFIG)
                INCLUDEPATH += $$(QWTINCLUDE)
    		win32-g++ {
      			INCLUDEPATH = $(QWTHOME)/src
                        LIBS += $$(QWTLIB)/lib$$(QWTLIBNAME).a
     		}
     		win32-msvc* || msvc{
        		DEFINES += QTCON_MAKEDLL _CRT_SECURE_NO_WARNINGS
                        CONFIG += Define_Build_qwt Define_Build_objDirs Define_Symbols Define_Build_OutputDir
     		}
	}
}
