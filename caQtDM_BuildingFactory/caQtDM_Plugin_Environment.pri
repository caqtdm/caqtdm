#==========================================================================================================
environment_Plugin {
        message(“environment_plugin configuration”)
        CONFIG += Define_Build_objDirs
        CONFIG += Define_Build_caQtDM_Lib
        CONFIG += Define_Build_epics_controls

        unix:!macx:!ios:!android {
                message(“environment_plugin configuration unix:!macx:!ios:!android”)
                INCLUDEPATH   += $(EPICSINCLUDE)
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                CONFIG += release
        }

        macx {
                message(“environment_plugin configuration macx”)
                INCLUDEPATH   += $(EPICSINCLUDE)
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += -L$(CAQTDM_COLLECT) -lqtcontrols
		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
		LIBS += $$(EPICSLIB)/libCom.dylib


                !modbus:!gps:{
                    plugins.path = Contents/PlugIns/controlsystems
                    plugins.files += $(CAQTDM_COLLECT)/controlsystems/libenvironment_plugin.dylib
                }

                modbus: {
                    plugins.path = Contents/PlugIns/controlsystems
                    plugins.files += $(CAQTDM_COLLECT)/controlsystems/libmodbus_plugin.dylib
                }
                gps: {
                    plugins.path = Contents/PlugIns/controlsystems
                    plugins.files += $(CAQTDM_COLLECT)/controlsystems/libgps_plugin.dylib
                }

                CONFIG += release
         }
        ios | android {
                message("epics3_plugin configuration : ios or android")
                CONFIG += staticlib
                INCLUDEPATH   += $(EPICSINCLUDE)
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                        INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                        INCLUDEPATH += $$OUT_PWD/../caQtDM_AndroidFunctions/src
                }
        }

        win32 {
                message(“environment_plugin configuration win32”)
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
                INCLUDEPATH += $$(EPICS_BASE)/include
                win32-msvc* || msvc{
                        CONFIG += Define_Build_epics_controls
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}
#==========================================================================================================
