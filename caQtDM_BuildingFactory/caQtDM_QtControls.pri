
#==========================================================================================================
caQtDM_QtControls {
        CONFIG += Define_Build_qwt
        CONFIG += caQtDM_xdl2ui_Lib
        CONFIG += Define_Build_objDirs
        CONFIG += Define_Build_OutputDir
        CONFIG += Define_library_settings
        CONFIG += Define_Symbols
        unix:!macx:!ios:!android {
                message("caQtDM_QtControls configuration : unix:!macx:!ios:!android")


  	}

        macx {
                message("caQtDM_QtControls configuration : macx")
                CAQTDM_POSTWORKFILE=libqtcontrols.dylib

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

     		}
	}
}
