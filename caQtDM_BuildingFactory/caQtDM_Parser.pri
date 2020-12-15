caQtDM_xdl2ui{
        CONFIG += console
        DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
        C_QMAKESPEC = $$QMAKESPEC
        DEFINES += BUILDARCH=\\\"$$replace(C_QMAKESPEC, \\\\, /)\\\"
	unix:!macx {
                message("adl2ui configuration unix!macx")
		OBJECTS_DIR = obj
		DESTDIR = $(CAQTDM_COLLECT)
	}

        macx {
                message("adl2ui configuration macx")
		OBJECTS_DIR = obj
		DESTDIR = $(CAQTDM_COLLECT)
        }

        win32 {
                message("adl2ui configuration win32")
                win32-msvc* || msvc{
                        CONFIG += Define_Build_OutputDir Define_Symbols
                }

                win32-g++ {
			OBJECTS_DIR = obj
                        #DESTDIR = $(CAQTDM_COLLECT)
                }
        }
}

