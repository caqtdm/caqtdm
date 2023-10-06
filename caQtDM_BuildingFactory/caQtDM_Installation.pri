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

                win32-msvc* || msvc{
                     target.path = $$_CAQTDM_COLLECT
                     INSTALLS += target

                }
                win32-g++ {
                        DESTDIR = $$_CAQTDM_COLLECT/controlsystems
                }
        }
}
