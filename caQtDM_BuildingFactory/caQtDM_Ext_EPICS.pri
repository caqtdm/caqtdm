
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
                QMAKE_POST_LINK += \
                   install_name_tool -change $$(EPICSLIB)/libca.dylib @rpath/libca.dylib $$CAQTDM_POSTWORKFILE $$RETURN \
                   install_name_tool -change $$(EPICSLIB)/libCom.dylib @rpath/libCom.dylib $$CAQTDM_POSTWORKFILE $$RETURN


        }

        ios | android {



        }

        win32 {

             DebugBuild {
                CONFIG += console

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
        LIBS += $$(EPICSLIB)/nt.lib
        LIBS += $$(EPICSLIB)/pvAccess.lib
        LIBS += $$(EPICSLIB)/pvAccessCA.lib
        LIBS += $$(EPICSLIB)/pvData.lib
        LIBS += $$(EPICSLIB)/pvaClient.lib

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
Define_install_epics{
    macx {
        EPICS = $$(EPICSINCLUDE)
        EPICSPATH1=$$(EPICS_BASE)/lib/darwin-x86/libca.
        EPICSPATH2=$$(EPICS_BASE)/lib/darwin-x86/libCom.
        EPICSVERSION = $$split(EPICS, "/")
        $$take_last(EPICSVERSION)
        EPICSVERSION = $$split(EPICSVERSION, "-")
        EPICSVERSION=$$last(EPICSVERSION)

        EPICSDYLIB1 = $$join(EPICSVERSION,"", $$EPICSPATH1, ."dylib")
        EPICSDYLIB2 = $$join(EPICSVERSION,"", $$EPICSPATH2, ."dylib")
        message($$EPICSDYLIB1)
        message($$EPICSDYLIB2)
        calib.files  = $$(EPICSDYLIB1)
        comlib.files = $$(EPICSDYLIB2)

        calib.path = Contents/Frameworks
        comlib.path = Contents/Frameworks

        QMAKE_BUNDLE_DATA += calib comlib
    }
}
