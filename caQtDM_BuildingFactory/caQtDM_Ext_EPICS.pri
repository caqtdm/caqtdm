
Define_Build_epics_controls {
    INCLUDEPATH  += $$(EPICS_BASE)/include

        unix:!macx:!ios:!android {
            INCLUDEPATH += $$(EPICS_BASE)/include/os/Linux
            INCLUDEPATH += $$(EPICS_BASE)/include/compiler/gcc
            LIBS += -L$$(EPICSLIB) -Wl,-rpath,$$(EPICSLIB) -lCom
            LIBS += -L$$(EPICSLIB) -Wl,-rpath,$$(EPICSLIB) -lca
        }

        macx {
                INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc
                LIBS += $$(EPICSLIB)/libca.dylib
                LIBS += $$(EPICSLIB)/libCom.dylib

        }

        ios | android {



        }

        win32 {

             DebugBuild {
                CONFIG += console
                EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
                LIBS += $$(EPICSLIB)/ca.lib
                LIBS += $$(EPICSLIB)/COM.lib

             }
            ReleaseBuild {
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
                INCLUDEPATH += $$(EPICS_BASE)/include/compiler/msvc

                LIBS += $$(EPICSLIB)/ca.lib
                LIBS += $$(EPICSLIB)/COM.lib

            }
       }

}

Define_Build_epicsPV_controls {

	INCLUDEPATH += $$(EPICSINCLUDE)
	INCLUDEPATH += $$(EPICSINCLUDE)/pv
    ReleaseBuild {
        EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
        LIBS += $${EPICS_LIBS}/nt.lib
        LIBS += $${EPICS_LIBS}/pvAccess.lib
        LIBS += $${EPICS_LIBS}/pvAccessCA.lib
        LIBS += $${EPICS_LIBS}/pvData.lib
        LIBS += $${EPICS_LIBS}/pvaClient.lib

    }
       win32 {

		INCLUDEPATH += $$(EPICSINCLUDE)/os/WIN32
		INCLUDEPATH += $$(EPICSINCLUDE)/compiler/msvc

                win32-msvc* || msvc{
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }



}
