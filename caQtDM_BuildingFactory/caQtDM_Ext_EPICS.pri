
Define_Build_epics_controls {
     DebugBuild {
        CONFIG += console
        EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
        LIBS += $${EPICS_LIBS}/ca.lib
        LIBS += $${EPICS_LIBS}/COM.lib
        #LIBS += $$(CAQTDM_COLLECT)/debug/qtcontrols.lib
     }
    ReleaseBuild {
        INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
        EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
        LIBS += $${EPICS_LIBS}/ca.lib
        LIBS += $${EPICS_LIBS}/COM.lib
        #LIBS += $$(CAQTDM_COLLECT)/qtcontrols.lib
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
                message(“EPICS 7 Windows”)
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
