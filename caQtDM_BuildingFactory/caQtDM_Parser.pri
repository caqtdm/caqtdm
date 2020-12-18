caQtDM_xdl2ui{
        CONFIG += console
        DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
        C_QMAKESPEC = $$QMAKESPEC
        DEFINES += BUILDARCH=\\\"$$replace(C_QMAKESPEC, \\\\, /)\\\"

        unix:!macx {
                message("adl2ui configuration unix!macx")
                QMAKE_CXXFLAGS += "-g -Wno-write-strings"
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

