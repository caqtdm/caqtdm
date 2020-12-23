
#==========================================================================================================
caQtDM_Lib {
        CONFIG += Define_Build_qwt Define_Build_objDirs Define_Build_Python
        CONFIG += Define_Build_epics_controls Define_Symbols Define_Build_OutputDir Define_Build_caQtDM_QtControls
        CONFIG += caQtDM_Plugin_Interface
        CONFIG += Define_library_settings
        unix:!macx:!ios:!android  {
                message("caQtDM_Lib configuration : unix:!macx:!ios:!android")

                QMAKE_CXXFLAGS += "-g"
                QMAKE_CFLAGS_RELEASE += "-g"

   	}

        macx {
                message("caQtDM_Lib configuration : macx")
                QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/


   	}

	ios | android {
                message("caQtDM_Lib configuration : ios or android")
                message( $$OUT_PWD )
                CONFIG += staticlib console
                CONFIG += release
                SOURCES +=     fingerswipegesture.cpp
      		HEADERS +=     fingerswipegesture.h
                INCLUDEPATH += ./caQtDM_Plugins
		ios {
      			INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                        INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
# this instruction is actually needed for me while the build does not work for multiple architectures
                        QMAKE_CXXFLAGS_RELEASE -= -O2
		}

		android {
      			INCLUDEPATH += $(EPICSINCLUDE)/os/android
                        DESTDIR = $(CAQTDM_COLLECT)
                }
	}

	win32 {
                message("caQtDM_Lib configuration : win32")

  		win32-msvc* || msvc{
        		DEFINES +=_CRT_SECURE_NO_WARNINGS
        		DEFINES += CAQTDM_LIB_LIBRARY
        		TEMPLATE = lib
   		}
   		win32-g++ {
       			EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += $$(QWTLIB)/lib$$(QWTLIBNAME).a
			LIBS += $$(QTCONTROLS_LIBS)/release//libqtcontrols.a
			LIBS += $${EPICS_LIBS}/ca.lib
			LIBS += $${EPICS_LIBS}/COM.lib
			QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM_Lib.dll ..\caQtDM_Binaries
   		}		
	}
}
#==========================================================================================================
