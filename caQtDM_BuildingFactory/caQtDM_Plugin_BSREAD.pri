#==========================================================================================================
bsread_Plugin {
        message(“bsread_plugin configuration”)
        CONFIG += Define_Build_caQtDM_Lib Define_Build_epics_controls Define_Symbols
        CONFIG += Define_Build_caQtDM_QtControls Define_Build_objDirs Define_ZMQ_Lib
        CONFIG += Define_library_settings
        unix:!macx:!ios:!android {
                message(“bsread_plugin configuration unix:!macx:!ios:!android”)
        }

        macx {
                message(“bsread_plugin configuration macx”)

        }

        win32 {
                message(“bsread_plugin configuration win32”)
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
		INCLUDEPATH += $$(EPICS_BASE)/include
                win32-msvc* || msvc{
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}

#==========================================================================================================
