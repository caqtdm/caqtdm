
#==========================================================================================================
demo_plugin {
        CONFIG += Define_Build_objDirs
        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
        CONFIG += Define_library_settings

        unix:!macx:!ios:!android {
                message("demo_plugin configuration unix:!macx:!ios:!android")
	}

        macx {
                message("demo_plugin configuration macx")
                CAQTDM_POSTWORKFILE = lib$${CAQTDM_INSTALL_LIB}.dylib
        }

        ios | android {
                message("demo_plugin configuration : ios or android")
                message( $$OUT_PWD )
                CONFIG += staticlib
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                android {
                    INCLUDEPATH += $$OUT_PWD/../caQtDM_AndroidFunctions/src
                }
        }

        win32 {
                message("demo_plugin configuration win32")
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32

                win32-msvc* || msvc{

                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}

#==========================================================================================================
