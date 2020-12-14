
#==========================================================================================================
epics3_plugin {
        CONFIG += Define_Build_objDirs

        unix:!macx:!ios:!android  {
                message("epics3_plugin configuration unix:!macx:!ios:!android ")
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc

                LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
 		CONFIG += release
	}

        macx {
                message("epics3_plugin configuration macx")
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Darwin
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
 		LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
 		LIBS += $$(EPICSLIB)/libca.dylib
 		LIBS += $$(EPICSLIB)/libCom.dylib
                CONFIG += release
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
