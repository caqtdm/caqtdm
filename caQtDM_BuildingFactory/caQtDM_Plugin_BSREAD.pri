#==========================================================================================================
bsread_Plugin {
        message(“bsread_plugin configuration”)
        CONFIG += Define_Build_caQtDM_Lib
        #CONFIG += Define_ControlsysTargetDir
        CONFIG += Define_Build_caQtDM_QtControls Define_Build_objDirs Define_ZMQ_Lib

        unix:!macx:!ios:!android {
                message(“bsread_plugin configuration unix:!macx:!ios:!android”)
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
 		CONFIG += release
	}

        macx {
                message(“bsread_plugin configuration macx”)
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	plugins.path = Contents/PlugIns/controlsystems
                plugins.files += $(CAQTDM_COLLECT)/controlsystems/libbsread_plugin.dylib
        	CONFIG += release

        }

        win32 {
                message(“bsread_plugin configuration win32”)
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
		INCLUDEPATH += $$(EPICS_BASE)/include
                win32-msvc* || msvc{

                        CONFIG += Define_Build_epics_controls

                        CONFIG += Define_Symbols

                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}

#==========================================================================================================
