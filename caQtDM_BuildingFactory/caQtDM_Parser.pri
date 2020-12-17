caQtDM_xdl2ui{
        CONFIG += console
        CONFIG += Define_Symbols
        DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
        C_QMAKESPEC = $$QMAKESPEC
        DEFINES += BUILDARCH=\\\"$$replace(C_QMAKESPEC, \\\\, /)\\\"
        OBJECTS_DIR = obj

        unix:!macx {
                message("adl2ui configuration unix!macx")
	}

        macx {
                message("adl2ui configuration macx")
        }

        win32 {
                message("adl2ui configuration win32")
                win32-msvc* || msvc{
                }

                win32-g++ {
                }
        }
}

