
#==========================================================================================================
epics3_plugin {
        CONFIG += Define_Build_objDirs
        CONFIG += Define_Build_caQtDM_Lib
        CONFIG += Define_Build_caQtDM_QtControls
        CONFIG += Define_Build_epics_controls


        unix:!macx:!ios:!android  {
                message("epics3_plugin configuration unix:!macx:!ios:!android ")
        }

        macx {
                message("epics3_plugin configuration macx")

        }

        ios | android {
                message("epics3_plugin configuration : ios or android")
                CONFIG += staticlib
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
                message("epics3_plugin configuration win32")
                INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
                INCLUDEPATH += $$(EPICS_BASE)/include/compiler/msvc
                win32-msvc* || msvc{
                        DEFINES +=_CRT_SECURE_NO_WARNINGS
                        CONFIG += Define_Build_epics_controls 
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += $${EPICS_LIBS}/ca.lib
                        LIBS += $${EPICS_LIBS}/COM.lib
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
	}
}
