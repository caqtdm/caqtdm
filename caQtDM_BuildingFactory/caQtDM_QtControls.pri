
#==========================================================================================================
caQtDM_QtControls {
        CONFIG += Define_Build_qwt
        CONFIG += caQtDM_xdl2ui_Lib

        unix:!macx:!ios:!android {
                message("caQtDM_QtControls configuration : unix:!macx:!ios:!android")
 		OBJECTS_DIR = obj

  	}

        macx {
                message("caQtDM_QtControls configuration : macx")

    		OBJECTS_DIR = obj
		DESTDIR = $$(CAQTDM_COLLECT)
    		CONFIG += lib_bundle

                ADL_EDL_FILES {
                   LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -ladlParser
                   LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -ledlParser
                }
  	}

	ios | android {
                message("caQtDM_QtControls configuration : ios or android")
		OBJECTS_DIR = obj

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


     		win32-msvc* || msvc{
                        DEFINES += QTCON_MAKEDLL _CRT_SECURE_NO_WARNINGS
                        CONFIG += Define_Build_objDirs Define_Symbols Define_Build_OutputDir
     		}
	}
}
