caQtDM_Installation{

isEmpty(INSTALLTIONSUBDIR){
    _CAQTDM_COLLECT = $$(CAQTDM_COLLECT)
}else{
    _CAQTDM_COLLECT = $$(CAQTDM_COLLECT)/$$INSTALLTIONSUBDIR
}



        unix:!macx:!ios:!android {
                DESTDIR = $(CAQTDM_COLLECT)/controlsystems
	}
        macx:!ios {
                DESTDIR = $(CAQTDM_COLLECT)/controlsystems
        }

        ios | android {
             android {
                DESTDIR = $(CAQTDM_COLLECT)/controlsystems
             }
        }
        win32 {
                message("adl2ui configuration win32")
                win32-msvc* || msvc{
		    DebugBuild {
			#DESTDIR = $$(CAQTDM_COLLECT)/debug/controlsystems
                        caqtdm_dll.files = debug/*.dll
                        caqtdm_dll.path = $$(CAQTDM_COLLECT)/debug/controlsystems
                        caqtdm_lib.files = debug/*.lib
                        caqtdm_lib.path = $$(CAQTDM_COLLECT)/debug/controlsystems
                        caqtdm_exe.files = debug/*.exe
                        caqtdm_exe.path = $$(CAQTDM_COLLECT)/debug/controlsystems
                        INSTALLS += caqtdm_dll caqtdm_lib caqtdm_exe

                     }
		    ReleaseBuild {
			#DESTDIR = $$(CAQTDM_COLLECT)/controlsystems
                        caqtdm_dll.files = release/*.dll
                        caqtdm_dll.path = $$_CAQTDM_COLLECT
                        caqtdm_lib.files = release/*.lib
                        caqtdm_lib.path = $$_CAQTDM_COLLECT
                        caqtdm_exe.files = release/*.exe
                        caqtdm_exe.path = $$_CAQTDM_COLLECT
                        INSTALLS += caqtdm_dll caqtdm_lib caqtdm_exe
		    }
                }
                win32-g++ {
                        DESTDIR = $$_CAQTDM_COLLECT/controlsystems
                }
        }
}
