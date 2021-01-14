caQtDM_Installation{

isEmpty(INSTALLTIONSUBDIR){
    _CAQTDM_COLLECT = $$(CAQTDM_COLLECT)
}else{
    _CAQTDM_COLLECT = $$(CAQTDM_COLLECT)/$$INSTALLTIONSUBDIR
}

        #CAQTDM_INSTALL_LIB

        #CAQTDM_INSTALL_EXE
        unix:!macx:!ios:!android {
                        for(a, CAQTDM_INSTALL_LIB):{
                            caqtdm_so.files += lib$${a}.so
                        }
                        caqtdm_so.path = $$_CAQTDM_COLLECT
                        for(a, CAQTDM_INSTALL_LIB):{
                            caqtdm_a.files += lib$${a}.a
                        }
                        caqtdm_a.path = $$_CAQTDM_COLLECT
                        caqtdm_execute.files = caQtDM adl2ui edl2ui
                        caqtdm_execute.path = $$_CAQTDM_COLLECT
                        INSTALLS += caqtdm_so caqtdm_a caqtdm_execute

	}
        macx:!ios {
                        caqtdm_execute.files = caQtDM.app
                        caqtdm_execute.path = $$_CAQTDM_COLLECT
                        INSTALLS += caqtdm_execute

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
                        for(a, CAQTDM_INSTALL_LIB):{
                            caqtdm_dll.files = release/$${a}.dll
                        }
                        caqtdm_dll.path = $$_CAQTDM_COLLECT

                        for(a, CAQTDM_INSTALL_LIB):{
                            caqtdm_lib.files = release/$${a}.lib
                        }
                        caqtdm_lib.path = $$_CAQTDM_COLLECT

                        for(a, CAQTDM_INSTALL_LIB):{
                            caqtdm_exe.files = release/$${a}.exe
                        }
                        caqtdm_exe.path = $$_CAQTDM_COLLECT
                        INSTALLS += caqtdm_dll caqtdm_lib caqtdm_exe
		    }
                }
                win32-g++ {
                        DESTDIR = $$_CAQTDM_COLLECT/controlsystems
                }
        }
}
