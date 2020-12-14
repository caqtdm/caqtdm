#==========================================================================================================
gps_plugin {
        CONFIG += Define_Build_objDirs

        unix:!macx:!ios:!android {
                message("gps_plugin configuration unix:!macx:!ios:!android")
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                CONFIG += release
        }

        macx {
                message("gps_plugin configuration macx")
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
                CONFIG += release
        }

        ios | android {
                message("gps_plugin configuration : ios or android")
                message( $$OUT_PWD )
                CONFIG += staticlib
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                android {
                    INCLUDEPATH += $$OUT_PWD/../caQtDM_AndroidFunctions/src
                }
        }

        win32 {
                message("gps_plugin configuration win32")
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32

                win32-msvc* || msvc{
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}
