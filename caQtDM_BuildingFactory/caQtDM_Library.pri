
#==========================================================================================================
caQtDM_Lib {
        unix:!macx:!ios:!android  {
                message("caQtDM_Lib configuration : unix:!macx:!ios:!android")
                LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lCom
      		LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
      		INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc

      		OBJECTS_DIR = ./obj
      		DESTDIR = $(CAQTDM_COLLECT)
                QMAKE_CXXFLAGS += "-g"
                QMAKE_CFLAGS_RELEASE += "-g"
                CONFIG += Define_Build_Python
   	}

        macx {
                message("caQtDM_Lib configuration : macx")
      		INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc

      		QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
                LIBS += -F$(QWTLIB) -framework $$(QWTLIBNAME)
                LIBS += -L$(CAQTDM_COLLECT) -lqtcontrols
      		LIBS += ${EPICSLIB}/libCom.dylib
      		DESTDIR = $(CAQTDM_COLLECT)
      		OBJECTS_DIR = ./obj
      		CONFIG += Define_Build_Python

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
   		INCLUDEPATH += $$(EPICS_BASE)/include
   		INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
                INCLUDEPATH += $$(EPICS_BASE)/include/compiler/msvc


  		win32-msvc* || msvc{
        		DEFINES +=_CRT_SECURE_NO_WARNINGS
        		DEFINES += CAQTDM_LIB_LIBRARY
        		TEMPLATE = lib
                        CONFIG += Define_Build_qwt Define_Build_objDirs
                        CONFIG += Define_Build_epics_controls Define_Symbols Define_Build_OutputDir Define_Build_caQtDM_QtControls
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
