#==========================================================================================================
archive_plugin {
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs

        unix:!macx:!ios:!android {
                message("archive_plugin configuration unix:!macx:!ios:!android")
                LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                CONFIG += release
        }

        macx {
                message("archive_plugin configuration macx")
                LIBS += $$(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
                LIBS += $$(CAQTDM_COLLECT)/libqtcontrols.dylib
                LIBS += -lz
                CONFIG += release
        }

        ios | android {
                message("archive_plugin configuration : ios or android")
                message( $$OUT_PWD )
                CONFIG += staticlib
                LIBS += $$OUT_PWD/../../../caQtDM_Lib/libcaQtDM_Lib.a
                LIBS += $$OUT_PWD/../../../caQtDM_QtControls/libqtcontrols.a
        }

        win32 {
                message("archive_plugin configuration win32")
                win32-msvc* || msvc{
                        CONFIG += Define_Build_caQtDM_Lib Define_Build_qtcontrols Define_Symbols
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}

#==========================================================================================================
demo_plugin {
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs
        
        unix:!macx:!ios:!android {
                message("demo_plugin configuration unix:!macx:!ios:!android")
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
 		CONFIG += release
	}

        macx {
                message("demo_plugin configuration macx")
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += $$(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	CONFIG += release
        }

        ios | android {
                message("demo_plugin configuration : ios or android")
                message( $$OUT_PWD )
                CONFIG += staticlib
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                android {
                    INCLUDEPATH += $$OUT_PWD/../caQtDM_AndroidFunctions/src
                }
        }

        win32 {
                message("demo_plugin configuration win32")
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32

                win32-msvc* || msvc{
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}

#==========================================================================================================
gps_plugin {
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs

        unix:!macx:!ios:!android {
                message("gps_plugin configuration unix:!macx:!ios:!android")
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                CONFIG += release
        }

        macx {
                message("gps_plugin configuration macx")
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += $$(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
                CONFIG += release
        }

        ios | android {
                message("gps_plugin configuration : ios or android")
                message( $$OUT_PWD )
                CONFIG += staticlib
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                android {
                    INCLUDEPATH += $$OUT_PWD/../caQtDM_AndroidFunctions/src
                }
        }

        win32 {
                message("gps_plugin configuration win32")
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32

                win32-msvc* || msvc{
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}
#==========================================================================================================
bsread_Plugin {
        message(“bsread_plugin configuration”)
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs Define_ZMQ_Lib
        
        unix:!macx:!ios:!android {
                message(“bsread_plugin configuration unix:!macx:!ios:!android”)
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
 		CONFIG += release
	}

        macx {
                message(“bsread_plugin configuration macx”)
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += $$(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
                LIBS += $$(CAQTDM_COLLECT)/libqtcontrols.dylib
        	plugins.path = Contents/PlugIns/controlsystems
                plugins.files += $$(CAQTDM_COLLECT)/controlsystems/libbsread_plugin.dylib
        	CONFIG += release

        }

        win32 {
                message(“bsread_plugin configuration win32”)
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
		INCLUDEPATH += $$(EPICS_BASE)/include
                win32-msvc* || msvc{
                        CONFIG += Define_Build_epics_controls Define_ControlsysTargetDir
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols Define_ZMQ_Lib
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}

#==========================================================================================================
epics3_plugin {
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs

        unix:!macx:!ios:!android  {
                message("epics3_plugin configuration unix:!macx:!ios:!android ")
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc

                LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
 		CONFIG += release
	}

        macx {
                message("epics3_plugin configuration macx")
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Darwin
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
                LIBS += $$(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
 		LIBS += $$(EPICSLIB)/libca.dylib
 		LIBS += $$(EPICSLIB)/libCom.dylib
                CONFIG += release
        }

        ios | android {
                message("epics3_plugin configuration : ios or android")
                CONFIG += staticlib
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                        INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                        INCLUDEPATH += $$OUT_PWD/../caQtDM_AndroidFunctions/src
                }
        }

	win32 {
                message("epics3_plugin configuration win32")
                INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
                INCLUDEPATH += $$(EPICS_BASE)/include/compiler/msvc
                win32-msvc* || msvc{
                        DEFINES +=_CRT_SECURE_NO_WARNINGS
                        CONFIG += Define_Build_epics_controls 
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += $${EPICS_LIBS}/ca.lib
                        LIBS += $${EPICS_LIBS}/COM.lib
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
	}
}
#==========================================================================================================
environment_Plugin {
        message(“environment_plugin configuration”)
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs

        unix:!macx:!ios:!android {
                message(“environment_plugin configuration unix:!macx:!ios:!android”)
                INCLUDEPATH   += $(EPICSINCLUDE)
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                CONFIG += release
        }

        macx {
                message(“environment_plugin configuration macx”)
                INCLUDEPATH   += $(EPICSINCLUDE)
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
                LIBS += -L$(CAQTDM_COLLECT) -lqtcontrols
		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
		LIBS += $$(EPICSLIB)/libCom.dylib


                !modbus:!gps:{
                    plugins.path = Contents/PlugIns/controlsystems
                    plugins.files += $$(CAQTDM_COLLECT)/controlsystems/libenvironment_plugin.dylib
                }

                modbus: {
                    plugins.path = Contents/PlugIns/controlsystems
                    plugins.files += $$(CAQTDM_COLLECT)/controlsystems/libmodbus_plugin.dylib
                }
                gps: {
                    plugins.path = Contents/PlugIns/controlsystems
                    plugins.files += $$(CAQTDM_COLLECT)/controlsystems/libgps_plugin.dylib
                }

                CONFIG += release
         }
        ios | android {
                message("epics3_plugin configuration : ios or android")
                CONFIG += staticlib
                INCLUDEPATH   += $(EPICSINCLUDE)
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                        INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                        INCLUDEPATH += $$OUT_PWD/../caQtDM_AndroidFunctions/src
                }
        }

        win32 {
                message(“environment_plugin configuration win32”)
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
                INCLUDEPATH += $$(EPICS_BASE)/include
                win32-msvc* || msvc{
                        CONFIG += Define_Build_epics_controls Define_ControlsysTargetDir
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                }
        }
}
#==========================================================================================================
epics4_plugin {
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs

        unix:!macx:!ios:!android {

        epics7 {
                message("epics4_plugin (with epics version 7) configuration unix:!macx:!ios:!android")
                INCLUDEPATH   += $(EPICSINCLUDE)
                INCLUDEPATH   += $(EPICSINCLUDE)/pv
                INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc

                !EPICS4_STATICBUILD {
                   message( "epics4_plugin build with shared object libraries of epics4" )
                   LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom -lpvAccess -lpvAccessCA -lpvData -lpvaClient -lnt
                   LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                }
                EPICS4_STATICBUILD  {
                   message( "epics4_plugin build with static libraries of epics4" )
                   LIBS += $(EPICSLIB)/libpvAccess.a
                   LIBS += $(EPICSLIB)/libpvData.a
                   LIBS += $(EPICSLIB)/libpvaClient.a
                   LIBS += $(EPICSLIB)/libnt.a
                   LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
                }
                CONFIG += release
                CAQTDM_PSI_SPECIAL_EPICS7_C11 {
                    contains(QT_VER_MAJ, 6) {
                        CONFIG += c++11
                    }

                    contains(QT_VER_MAJ, 5) {
                        CONFIG += c++11
                    }
                    contains(QT_VER_MAJ, 4) {
                        QMAKE_CXXFLAGS += -std=c++11
                    }
                }
          }

	}
	
        macx {
                message("epics4_plugin configuration macx")
                INCLUDEPATH   += $(EPICSINCLUDE)
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Darwin
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang

                #INCLUDEPATH   += $(EPICS4LOCATION)/pvDataCPP/include
                #INCLUDEPATH   += $(EPICS4LOCATION)/pvAccessCPP/include
                #INCLUDEPATH   += $(EPICS4LOCATION)/pvaClientCPP/include
                #INCLUDEPATH   += $(EPICS4LOCATION)/normativeTypesCPP/include

                #EPICS4LOC1 = $(EPICS4LOCATION)/pvAccessCPP/lib/$(EPICS_HOST_ARCH)
                #EPICS4LOC2 = $(EPICS4LOCATION)/pvDataCPP/lib/$(EPICS_HOST_ARCH)
                #EPICS4LOC3 = $(EPICS4LOCATION)/pvaClientCPP/lib/$(EPICS_HOST_ARCH)
                #EPICS4LOC4 = $(EPICS4LOCATION)/normativeTypesCPP/lib/$(EPICS_HOST_ARCH)

                #LIBS += $${EPICS4LOC1}/libpvAccess.a
                #LIBS += $${EPICS4LOC2}/libpvData.a
                #LIBS += $${EPICS4LOC3}/libpvaClient.a
                #LIBS += $${EPICS4LOC4}/libnt.a

        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	LIBS += $$(EPICSLIB)/libca.dylib
        	LIBS += $$(EPICSLIB)/libCom.dylib
                LIBS += $$(EPICSLIB)/libpvAccess.dylib
                LIBS += $$(EPICSLIB)/libpvAccessCA.dylib
                LIBS += $$(EPICSLIB)/libpvData.dylib
                LIBS += $$(EPICSLIB)/libpvaClient.dylib
                LIBS += $$(EPICSLIB)/libnt.dylib
                CONFIG += release
        }

        ios | android {
                message("epics4_plugin configuration : ios or android")
                CONFIG += staticlib
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                INCLUDEPATH   += $(EPICSINCLUDE)
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                        INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                }
        }

	win32-msvc* || msvc {
                message("epics4_plugin configuration win32")
		 epics7 {
                    message("epics4_plugin (with epics version 7) configuration win32")
                    CONFIG +=Define_Build_caQtDM_Lib Define_Build_qtcontrols
                    CONFIG +=Define_Build_epicsPV_controls Define_Build_epics_controls
                    CONFIG +=Define_Symbols
                    QMAKE_CXXFLAGS     -= -Zc:externConstexpr
                    # by some reason this gets into the centos+RHL compilation ?????
                    !unix: {
                        QMAKE_CXXFLAGS     += /Zc:twoPhase-
                        QMAKE_CXXFLAGS     += -Zc:referenceBinding
                    }
		  }
 		
 
	}
}

#==========================================================================================================
caQtDM_QtControls {
        unix:!macx:!ios:!android {
                message("caQtDM_QtControls configuration : unix:!macx:!ios:!android")
 		OBJECTS_DIR = obj
		DESTDIR = $$(CAQTDM_COLLECT)
 		INCLUDEPATH += $$(QWTINCLUDE)
                LIBS += -L$$(QWTLIB) -Wl,-rpath,$(QWTLIB) -l$$(QWTLIBNAME)
  	}

        macx {
                message("caQtDM_QtControls configuration : macx")
 		INCLUDEPATH += $$(QWTINCLUDE)
    		OBJECTS_DIR = obj
		DESTDIR = $$(CAQTDM_COLLECT)
    		CONFIG += lib_bundle
                LIBS += -lz
                LIBS += -F$$(QWTLIB) -framework $$(QWTLIBNAME)
                        ADL_EDL_FILES {
                           LIBS += $(CAQTDM_COLLECT)/libadlParser.dylib
                           LIBS += $(CAQTDM_COLLECT)/libedlParser.dylib
                        }
  	}

	ios | android {
                message("caQtDM_QtControls configuration : ios or android")
		OBJECTS_DIR = obj
		INCLUDEPATH += $$(QWTINCLUDE)
                CONFIG += staticlib
                CONFIG += release
                CONFIG -= debug
                CONFIG -= debug_and_release
                android {
                   DESTDIR = $(CAQTDM_COLLECT)
                }
        }

	win32 {
                message("caQtDM_QtControls configuration : win32")
		message($$CONFIG)
                INCLUDEPATH += $$(QWTINCLUDE)
    		win32-g++ {
      			INCLUDEPATH = $(QWTHOME)/src
                        LIBS += $$(QWTLIB)/lib$$(QWTLIBNAME).a
     		}
     		win32-msvc* || msvc{
        		DEFINES += QTCON_MAKEDLL _CRT_SECURE_NO_WARNINGS
                        CONFIG += Define_Build_qwt Define_Build_objDirs Define_Symbols Define_Build_OutputDir
     		}
	}
}

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


                LIBS += -F$(QWTLIB) -framework $$(QWTLIBNAME)
                LIBS += -L$$(CAQTDM_COLLECT) -lqtcontrols
      		LIBS += ${EPICSLIB}/libCom.dylib
                DESTDIR = $$(CAQTDM_COLLECT)
      		OBJECTS_DIR = ./obj
                # removed no standard python under Mac
                # CONFIG += Define_Build_Python

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
                        CONFIG += Define_Build_epics_controls Define_Symbols Define_Build_OutputDir
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
caQtDM_Viewer {
        C_QMAKESPEC = $$QMAKESPEC
        DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
        DEFINES += BUILDARCH=\\\"$$replace(C_QMAKESPEC, \\\\, /)\\\"
        CONFIG += Define_Build_objDirs
        unix: {
                message("caQtDM_viewer configuration : unix")
                DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
                DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"

                !ios:!android {
                        message("caQtDM_viewer configuration : !ios!android (all unixes + mac)")
                        DESTDIR = $(CAQTDM_COLLECT)
                 !macx: {CONFIG += x11}
                        LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                        LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
                        ADL_EDL_FILES {
                           LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -ladlParser
                           LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -ledlParser
                        }
                        LIBS += -L$(CAQTDM_COLLECT) -L$(CAQTDM_COLLECT)/designer
                }
        }
        macx:!ios {
                message("caQtDM_viewer configuration : macx (only mac)")

#for epics 3.15 and gcc we need this
                exists( $(EPICSINCLUDE)/compiler/gcc/compilerSpecific.h) {
                    INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc
                }
                exists( $(EPICSINCLUDE)/compiler/clang/compilerSpecific.h) {
                    INCLUDEPATH   += $(EPICSINCLUDE)/compiler/clang
                }

                DESTDIR = $$(CAQTDM_COLLECT)
                QMAKE_INFO_PLIST = ./src/Mac/Info.plist
                QMAKE_TARGET_BUNDLE_PREFIX=ch.psi
                QMAKE_BUNDLE=caQtDM.osx
                APP-FONTS.files = lucida-sans-typewriter.ttf
                APP-FONTS.path = Contents/Resources/fonts
                QMAKE_BUNDLE_DATA += APP-FONTS
                #QMAKE_BUNDLE_NAME = ch.psi.caQtDM.osx
                CONFIG += app_bundle
                ICON = ./src/caQtDM.icns

                LIBS += $$(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
                LIBS += $$(CAQTDM_COLLECT)/libqtcontrols.dylib

                plugins.path = Contents/PlugIns/designer
                plugins.files = $$(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.dylib
                plugins.files += $$(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.dylib
                plugins.files += $$(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.dylib
                plugins.files += $$(CAQTDM_COLLECT)/designer/libqtcontrols_utilities_plugin.dylib
                caqtdmlibs.path = Contents/Frameworks/
                caqtdmlibs.files = $$(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib $$(CAQTDM_COLLECT)/libqtcontrols.dylib
                ADL_EDL_FILES {
                           caqtdmlibs.files += $$(CAQTDM_COLLECT)/libadlParser.dylib
                           caqtdmlibs.files += $$(CAQTDM_COLLECT)/libedlParser.dylib
                }
                QMAKE_BUNDLE_DATA += plugins caqtdmlibs
                calib.path = Contents/Frameworks
#                calib.files = $$(EPICS_BASE)/lib/darwin-x86/libca.3.14.12.dylib
                comlib.path = Contents/Frameworks
#                comlib.files = $$(EPICS_BASE)/lib/darwin-x86/libCom.3.14.12.dylib

#compute dylib library from epicsinclude
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

                qwtframework.path = Contents/Frameworks
                qwtframework.files = $$(QWTHOME)/lib/$$(QWTLIBNAME).framework
                QMAKE_BUNDLE_DATA += calib comlib qwtframework
                plugins_epics3.path = Contents/PlugIns/controlsystems
                plugins_epics3.files += $$(CAQTDM_COLLECT)/controlsystems/libepics3_plugin.dylib
                QMAKE_BUNDLE_DATA += plugins_epics3
                plugins_demo.path = Contents/PlugIns/controlsystems
                plugins_demo.files += $$(CAQTDM_COLLECT)/controlsystems/libdemo_plugin.dylib
                QMAKE_BUNDLE_DATA += plugins_demo
                plugins_environment.path = Contents/PlugIns/controlsystems
                plugins_environment.files += $$(CAQTDM_COLLECT)/controlsystems/libenvironment_plugin.dylib
                QMAKE_BUNDLE_DATA += plugins_environment

                epics4: {
                                    plugins_epics4.path = Contents/PlugIns/controlsystems
                                    plugins_epics4.files += $$(CAQTDM_COLLECT)/controlsystems/libepics4_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_epics4
                                }
                bsread:{
                                    plugins_bsread.path = Contents/PlugIns/controlsystems
                                    plugins_bsread.files += $$(CAQTDM_COLLECT)/controlsystems/libbsread_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_bsread
                                    zmqlibrary.path = Contents/Frameworks
                                    zmqlibrary.files += $$(ZMQLIB)/libzmq.5.dylib
                                    QMAKE_BUNDLE_DATA += zmqlibrary
                                }
                archiveSF:{
                                    plugins_archiveSF.path = Contents/PlugIns/controlsystems
                                    plugins_archiveSF.files += $$(CAQTDM_COLLECT)/controlsystems/libarchiveSF_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_archiveSF
                                }
                modbus: {
                                    plugins_modbus.path = Contents/PlugIns/controlsystems
                                    plugins_modbus.files += $$(CAQTDM_COLLECT)/controlsystems/libmodbus_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_modbus
                                }

                gps: {
                                    plugins_gps.path = Contents/PlugIns/controlsystems
                                    plugins_gps.files += $$(CAQTDM_COLLECT)/controlsystems/libgps_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_gps
                                }
        }

        ios {
                    message("caQtDM_viewer configuration : ios")
                    #DESTDIR = $(CAQTDM_COLLECT)

                    message( $$OUT_PWD )
                    QMAKE_LFLAGS += -Wl,-e,_qt_main_wrapper
                    CONFIG += static
                    CONFIG += release
                    LIBS += $$OUT_PWD/../caQtDM_Lib/libcaQtDM_Lib.a
                    LIBS += $$OUT_PWD/../caQtDM_QtControls/libqtcontrols.a

                    LIBS += $$OUT_PWD/../caQtDM_QtControls/plugins/libqtcontrols_controllers_plugin.a
                    LIBS += $$OUT_PWD/../caQtDM_QtControls/plugins/libqtcontrols_monitors_plugin.a
                    LIBS += $$OUT_PWD/../caQtDM_QtControls/plugins/libqtcontrols_graphics_plugin.a
                    LIBS += $$OUT_PWD/../caQtDM_QtControls/plugins/libqtcontrols_utilities_plugin.a
                    LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/demo/libdemo_plugin.a
                    LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/epics3/libepics3_plugin.a
                    LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/archive/archiveSF/libarchiveSF_plugin.a
                    LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/environment/libenvironment_plugin.a

                    modbus {
                        LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/modbus/libmodbus_plugin.a
                    }
                    gps {
                        LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/gps/libgps_plugin.a
                    }
                    epics7{
                        LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/epics4/libepics4_plugin.a
                        LIBS += $(EPICSLIB)/libpvAccess.a
                        LIBS += $(EPICSLIB)/libpvAccessCA.a
                        LIBS += $(EPICSLIB)/libpvData.a
                        LIBS += $(EPICSLIB)/libpvaClient.a
                        LIBS += $(EPICSLIB)/libnt.a
                    }

                    ICON = $$PWD/caQtDM_Viewer/src/caQtDM.icns

                    APP_ICON.files = $$files($$PWD/caQtDM_Viewer/src/caQtDM*.png)

                    APP1_ICON.files = $$PWD/caQtDM_Viewer/src/caQtDM.icns
                    APP_XML_FILES.files = $$PWD/caQtDM_Viewer/caQtDM_IOS_Config.xml

                    #StartScreen.files = $$files($$PWD/caQtDM_Viewer/src/IOS/LaunchImage*.png)
                    #StartScreen.files += $$PWD/caQtDM_Viewer/src/StartScreen-Landscape.png
                    #StartScreen.files += $$PWD/caQtDM_Viewer/src/StartScreen-568h@2x.png

                    APP-FONTS.files = $$PWD/caQtDM_Viewer/lucida-sans-typewriter.ttf
                    APP-FONTS.path = fonts
                    #assets_catalogs.files = $$files($$PWD/caQtDM_Viewer/src/IOS/*.xcassets)
                    #QMAKE_BUNDLE_DATA += assets_catalogs

                    QMAKE_BUNDLE_DATA += APP_XML_FILES APP-FONTS #APP_ICON APP1_ICON
                    QMAKE_CFLAGS += -gdwarf-2
                    QMAKE_CXXFLAGS += -gdwarf-2
                    QMAKE_TARGET_BUNDLE_PREFIX=ch.psi

                    QMAKE_BUNDLE_NAME = ch.psi.caQtDM
                    bundle_identifier.name = PRODUCT_BUNDLE_IDENTIFIER
                    bundle_identifier.value = ch.psi.caQtDM
                    QMAKE_MAC_XCODE_SETTINGS += bundle_identifier
                    target.name=IPHONEOS_DEPLOYMENT_TARGET
                    target.value = 16.0

                    QMAKE_MAC_XCODE_SETTINGS += target
                    assetIcon.name = ASSETCATALOG_COMPILER_APPICON_NAME
                    assetIcon.value = AppIcon
                    launchImage.name = ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME
                    launchImage.value = LaunchImage
                    searchheader.name=ALWAYS_SEARCH_USER_PATHS
                    searchheader.value=NO
                    weakobjc.name=CLANG_ENABLE_OBJC_WEAK
                    weakobjc.value=YES
                    QMAKE_MAC_XCODE_SETTINGS += assetIcon launchImage searchheader weakobjc

                    QMAKE_INFO_PLIST += $$PWD/caQtDM_Viewer/src/IOS/Info.plist
                    QMAKE_ASSET_CATALOGS += $$PWD/caQtDM_Viewer/src/IOS/Assets.xcassets
                    QMAKE_IOS_LAUNCH_SCREEN += $$PWD/caQtDM_Viewer/src/IOS/LaunchScreen.storyboard
#actually, I have a problem with ios, it seems that iphonesimulator is defined too and wants than the library of simlator
# I comment it out now
                    iphonesimulator {
                         message("caQtDM_viewer configuration : iphonesimulator")
                         # when .dylib and .a in same directory, macos takes .dylib, so separate the libraries
                         LIBS += $$(EPICSLIB)/libca.a
                         LIBS += $$(EPICSLIB)/libCom.a
                         LIBS += $$(QWTHOME)/lib/lib$$(QWTLIBNAME).a
                         #LIBS += $$(QWTHOME)/lib/lib$$(QWTLIBNAME)_iphonesimulator.a
                         # build simulator only for 32 bit
                         INCLUDEPATH += $$(QWTHOME)/src
                         QMAKE_LFLAGS += -all_load
                    }
                    iphoneos {
                        message("caQtDM_viewer configuration : iphoneos")

                         LIBS += $$(EPICSLIB)/libca.a
                         LIBS += $$(EPICSLIB)/libCom.a
                         LIBS += $$(QWTHOME)/lib/lib$$(QWTLIBNAME).a

                         ###############################################################################
                         # Code Signing settings needed only for iOS App Store
                         ###############################################################################

                         #bitcode.name = ENABLE_BITCODE
                         #bitcode.value = YES
                         #QMAKE_MAC_XCODE_SETTINGS += bitcode

                         #bitcode_generator.name = BITCODE_GENERATION_MODE
                         #bitcode_generator.value = bitcode
                         #QMAKE_MAC_XCODE_SETTINGS += bitcode_generator

                         provisioning_profile_spec.name=PROVISIONING_PROFILE_SPECIFIER
                         provisioning_profile_spec.value = caQtDM Distribution
                         #provisioning_profile_spec.value = caQtDM Development
                         QMAKE_MAC_XCODE_SETTINGS += provisioning_profile_spec

                         #signing_identity.name = CODE_SIGN_IDENTITY
                         #signing_identity.value = $$(CODE_SIGN_IDENTITY)
                         #QMAKE_MAC_XCODE_SETTINGS += signing_identity

                         signing_identity.name = CODE_SIGN_IDENTITY
                         signing_identity.value = Apple Distribution: Helge Brands (Q6CFPW364S)
                         QMAKE_MAC_XCODE_SETTINGS += signing_identity



                         #setting.name = DEVELOPMENT_TEAM
                         #setting.value = $$(CERTIFICATNUMBER)
                         #QMAKE_MAC_XCODE_SETTINGS += setting
                         #setting.name = DEVELOPMENT_TEAM
                         #setting.value = Q6CFPW364S
                         #QMAKE_MAC_XCODE_SETTINGS += setting

                         payload_dir.target = $$OUT_PWD/Release-iphoneos/Payload
                         payload_dir.commands = @test -d $$OUT_PWD/Release-iphoneos/Payload || mkdir -p $$OUT_PWD/Release-iphoneos/Payload
                         payload_dir.depends = $$OUT_PWD/Release-iphoneos

                         QMAKE_EXTRA_TARGETS +=payload_dir
                    }
                    epics4: {
                                    LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/epics4/libepics4_plugin.a
                    }
                    archiveSF:{
                                    LIBS += $$OUT_PWD/../caQtDM_Lib/caQtDM_Plugins/archive/archiveSF/libarchiveSF_plugin.a
                    }
        }
        android {
                        message("caQtDM_viewer configuration : android")
                        message($$PWD)
                        DESTDIR = $$(CAQTDM_COLLECT)
                        CONFIG += staticlib
                        CONFIG += console
                        LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.a
                        LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.a
                        LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.a
                        LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_utilities_plugin.a
                        LIBS += $(CAQTDM_COLLECT)/controlsystems/libdemo_plugin.a
                        LIBS += $(CAQTDM_COLLECT)/controlsystems/libepics3_plugin.a
                        LIBS += $(CAQTDM_COLLECT)/libAndroidFunctions.a
                epics4: {
                                LIBS += $(CAQTDM_COLLECT)/controlsystems/libepics4_plugin.a
                                }
                archiveSF: {
                                LIBS += $(CAQTDM_COLLECT)/controlsystems/libarchiveSF_plugin.a
                        }

                        LIBS += $(CAQTDM_COLLECT)/controlsystems/libenvironment_plugin.a

                        modbus {
                            LIBS += $(CAQTDM_COLLECT)/controlsystems/libmodbus_plugin.a
                        }
                        gps {
                            LIBS += $(CAQTDM_COLLECT)/controlsystems/libgps_plugin.a
                        }

                        LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
                        LIBS += $(CAQTDM_COLLECT)/libqtcontrols.a
                        LIBS += $$(QWTHOME)/lib/lib$$(QWTLIBNAME).a
                        LIBS += $$(EPICSLIB)/libca.a
                        LIBS += $$(EPICSLIB)/libCom.a
                        LOCAL_LDLIBS += -llog

                        ICON = $$PWD/caQtDM_Viewer/src/caQtDM.icns
                        APP_ICON.files = $$PWD/caQtDM_Viewer/src/caqtdm762x.png
                        APP_ICON.path = /res/drawable

                        APP1_ICON.files = $$PWD/caQtDM_Viewer/src/caQtDM.icns
                        APP_XML_FILES.files = $$PWD/caQtDM_Viewer/caQtDM_IOS_Config.xml
                        StartScreen.files += $$PWD/caQtDM_Viewer/src/startscreenlandscape.png
                        StartScreen.path = /res/drawable
                        APP-FONTS.files = $$PWD/caQtDM_Viewer/lucida-sans-typewriter.ttf
                        APP-FONTS.path = /fonts
                        QMAKE_BUNDLE_DATA += APP_XML_FILES APP_ICON APP1_ICON StartScreen APP-FONTS

                        deployment.files += $$PWD/caQtDM_Viewer/caQtDM_IOS_Config.xml
                        deployment.path = /assets
                        INSTALLS += deployment APP-FONTS APP_ICON StartScreen

                        DISTFILES += /Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Viewer/src/Android/AndroidManifest.xml
                        DISTFILES += src/Android/res/values/libs.xml src/Android/build.gradle
                        ANDROID_PACKAGE_SOURCE_DIR = $$PWD/caQtDM_Viewer/src/Android
                }

        win32 {
                message("caQtDM_viewer configuration : win32")
                DEFINES += BUILDTIME=\\\"\\\"
                DEFINES += BUILDDATE=\\\"\\\"
                DEFINES += SUPPORT=\\\"\\\"

                TARGET = caQtDM
                TEMPLATE = app
                MOC_DIR = moc
                VPATH += ./src

                INCLUDEPATH += .
                INCLUDEPATH += $(QWTHOME)/src
                INCLUDEPATH += $$(EPICS_BASE)/include
                INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
                INCLUDEPATH += $$(EPICS_BASE)/include/compiler/msvc
                INCLUDEPATH += ../caQtDM_Lib/src
                INCLUDEPATH += ../caQtDM_QtControls/src
                INCLUDEPATH += $(QWTINCLUDE)
                INCLUDEPATH += $(EPICSINCLUDE)

                OTHER_FILES += ./src/caQtDM.ico
                DEFINES +=_CRT_SECURE_NO_WARNINGS
                RESOURCES += ./src/caQtDM.qrc
                RC_FILE = ./src/caQtDM.rc

                win32-msvc* || msvc{
                        CONFIG += Define_Build_qwt
                        CONFIG += Define_Build_epics_controls
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
                        CONFIG += Define_Build_OutputDir
                        contains(QT_VER_MAJ, 6) {
                            QMAKE_LFLAGS += /entry:mainCRTStartup
                            QMAKE_LFLAGS += /STACK:32000000
                        }

                }

                win32-g++ {
                        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
                        LIBS += $$(QWTLIB)/lib$$(QWTLIBNAME).a
                        LIBS += $$(QTCONTROLS_LIBS)/release/libqtcontrols.a
                        LIBS += $${EPICS_LIBS}/ca.lib
                        LIBS += $${EPICS_LIBS}/COM.lib
                        LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
                        QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM.exe ..\caQtDM_Binaries
                }
        }

}
#==========================================================================================================
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
			DESTDIR = $(CAQTDM_COLLECT)
                }
        }
}
caQtDM_xdl2ui_Lib{
        unix:!macx {
            LIBS += $(CAQTDM_COLLECT)/libadlParser.a
            LIBS += $(CAQTDM_COLLECT)/libedlParser.a
        }

        macx {
            LIBS += $(CAQTDM_COLLECT)/libadlParser.a
        }

        win32 {
                win32-msvc* || msvc{
                        LIBS += $(CAQTDM_COLLECT)/adlParser.lib
                }

                win32-g++ {
                        LIBS += $(CAQTDM_COLLECT)/libadlParser.a
                }
        }

}
#==========================================================================================================
Define_ZMQ_Lib{
	
        ZEROMQ_INCLUDECHECK=$$(QTHOME)/include
        !equals(ZEROMQ_INCLUDECHECK,$$(ZMQINC)){
            message("No collision with Qt detected, include ZMQ include")
            INCLUDEPATH += $$(ZMQINC)
        }

        unix:!macx {
                 LIBS += -L$$(ZMQLIB) -Wl,-rpath,$$(ZMQLIB) -lzmq
                 #LIBS += $$(ZMQLIB)/libzmq.a
	}
        macx {
                LIBS += $$(ZMQLIB)/libzmq.5.dylib
        }
        win32 {
	    DebugBuild {
                LIBS += $$(ZMQLIB)/libzmq.lib
	     }
	    ReleaseBuild {
                LIBS += $$(ZMQLIB)/libzmq.lib
	    }
	}
}

Define_Build_Python {
     PYTHONCALC: {
        warning("for image and visibility calculation, python will be build in")
	!ios {
	!android {
	   unix:!macx {
	      DEFINES += PYTHON
	      INCLUDEPATH += $(PYTHONINCLUDE)
	      LIBS += -L$(PYTHONLIB) -Wl,-rpath,$(PYTHONLIB) -lpython$(PYTHONVERSION)
	    }
	    unix:macx {
	       DEFINES += PYTHON
               #INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/$(PYTHONVERSION)/include/python$(PYTHONVERSION)/
               #LIBS += -L/System/Library/Frameworks/Python.framework/Versions/$(PYTHONVERSION)/lib/ -lpython$(PYTHONVERSION)

               #this has to be done in a better way, but works!
               LIBS += -L/usr/local/Cellar/python/3.7.1/Frameworks/Python.framework/Versions/3.7/lib/ -lpython3.7
               INCLUDEPATH += /usr/local/Cellar/python/3.7.1/Frameworks/Python.framework/Versions/3.7/include/python3.7m/
	    }
        }
        }
    }
}

Define_Symbols{
    ReleaseBuild {
        QMAKE_CXXFLAGS += /Z7
        QMAKE_CFLAGS   += /Z7
        QMAKE_LFLAGS   += /DEBUG /OPT:REF /OPT:ICF
    }
}

Define_Build_qwt {
     DebugBuild {
         message("QWT Debug configuration : win32")
         LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME)d.lib
     }
    ReleaseBuild {
        message("QWT Release configuration : win32")
        LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME).lib
    }
}
Define_Build_objDirs {
	unix:!macx!ios:!android {
		OBJECTS_DIR = obj
	}
        macx {
		OBJECTS_DIR = obj
        }
        win32 {
	    DebugBuild {
		OBJECTS_DIR = debug/obj
	     }
	    ReleaseBuild {
		OBJECTS_DIR = release/obj
	    }
	}
}

Define_Build_epics_controls {
     DebugBuild {
        CONFIG += console
        EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
        LIBS += $${EPICS_LIBS}/ca.lib
        LIBS += $${EPICS_LIBS}/COM.lib
        LIBS += $$(CAQTDM_COLLECT)/debug/qtcontrols.lib
     }
    ReleaseBuild {
        INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32
        EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
        LIBS += $${EPICS_LIBS}/ca.lib
        LIBS += $${EPICS_LIBS}/COM.lib
        LIBS += $$(CAQTDM_COLLECT)/qtcontrols.lib
    }
}
Define_Build_epicsPV_controls {

	INCLUDEPATH += $(EPICSINCLUDE)
	INCLUDEPATH += $(EPICSINCLUDE)/pv
	INCLUDEPATH += $(EPICSINCLUDE)/os/WIN32
	INCLUDEPATH += $(EPICSINCLUDE)/compiler/msvc
     DebugBuild {
        CONFIG += console
        EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
        LIBS += $${EPICS_LIBS}/pvAccess.lib
        LIBS += $${EPICS_LIBS}/pvData.lib
        LIBS += $${EPICS_LIBS}/pvaClient.lib

     }
    ReleaseBuild {
        EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
        LIBS += $${EPICS_LIBS}/nt.lib
        LIBS += $${EPICS_LIBS}/pvAccess.lib
        LIBS += $${EPICS_LIBS}/pvAccessCA.lib
        LIBS += $${EPICS_LIBS}/pvData.lib
        LIBS += $${EPICS_LIBS}/pvaClient.lib

    }
}

Define_Build_caQtDM_Lib {
     DebugBuild {
	LIBS += $$(CAQTDM_COLLECT)/debug/caQtDM_Lib.lib
     }
    ReleaseBuild {
	LIBS += $$(CAQTDM_COLLECT)/caQtDM_Lib.lib
    }

}

Define_Build_qtcontrols {
     DebugBuild {
        LIBS += $$(CAQTDM_COLLECT)/debug/qtcontrols.lib
     }
    ReleaseBuild {
        LIBS += $$(CAQTDM_COLLECT)/qtcontrols.lib
    }

}

Define_Build_OutputDir {
    win32 {
        DebugBuild {
            DESTDIR = $$(CAQTDM_COLLECT)/debug
        }
        ReleaseBuild {
            DESTDIR = $$(CAQTDM_COLLECT)
        }
    }
}
Define_ControlsysTargetDir{
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
			DESTDIR = $$(CAQTDM_COLLECT)/debug/controlsystems
		     }
		    ReleaseBuild {
			DESTDIR = $$(CAQTDM_COLLECT)/controlsystems
		    }
                }
                win32-g++ {
			DESTDIR = $(CAQTDM_COLLECT)/controlsystems
                }
        }
}
		

SOURCES +=

HEADERS +=
