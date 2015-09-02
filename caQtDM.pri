#==========================================================================================================
demo_plugin {
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs
        
        unix:!macx:!ios:!android {
                message("demo_plugin configuration unix:!macx:!ios:!android")
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
 		CONFIG += release
	}

        macx {
                message("demo_plugin configuration macx")
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	plugins.path = Contents/PlugIns/controlsystems
        	plugins.files += $(CAQTDM_COLLECT)/controlsystems/libdemo_plugin.dylib
        	CONFIG += release
        }

        ios | android {
                message("demo_plugin configuration : ios or android")
                CONFIG += staticlib
                LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
        }

        win32 {
                message("demo_plugin configuration win32")
                INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32

                win32-msvc* {
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols
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
 		LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
 		CONFIG += release
	}

        macx {
                message("epics3_plugin configuration macx")
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Darwin
 		LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
 		LIBS += $$(EPICSLIB)/libca.dylib
 		LIBS += $$(EPICSLIB)/libCom.dylib
 		plugins.path = Contents/PlugIns/controlsystems
 		plugins.files += $(CAQTDM_COLLECT)/controlsystems/libepics3_plugin.dylib
 		CONFIG += release
        }

        ios | android {
                message("epics3_plugin configuration : ios or android")
                CONFIG += staticlib
                LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                }
        }

	win32 {
                message("epics3_plugin configuration win32")
  		INCLUDEPATH  += $$(EPICS_BASE)/include/os/win32

                win32-msvc* {
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
epics4_plugin {
        CONFIG += Define_ControlsysTargetDir Define_Build_objDirs

        unix:!macx:!ios:!android {
                message("epics4_plugin configuration unix:!macx:!ios:!android")
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 		LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
 		CONFIG += release
	}
	
        macx {
                message("epics4_plugin configuration macx")
		INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	LIBS += $$(EPICSLIB)/libca.dylib
        	LIBS += $$(EPICSLIB)/libCom.dylib
        	plugins.path = Contents/PlugIns/controlsystems
        	plugins.files += $(CAQTDM_COLLECT)/controlsystems/libepics4_plugin.dylib
        	CONFIG += release
        }

        ios | android {
                message("epics4_plugin configuration : ios or android")
                CONFIG += staticlib
                LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                }
        }

	win32 {
                message("epics4_plugin configuration win32")
  		
                win32-msvc* {
                        CONFIG += Define_Build_epics_controls Define_Build_objDirs
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
caQtDM_QtControls {
        unix:!macx:!ios:!android {
                message("caQtDM_QtControls configuration : unix:!macx:!ios:!android")
 		OBJECTS_DIR = obj
		DESTDIR = $$(CAQTDM_COLLECT)
 		INCLUDEPATH += $$(QWTINCLUDE)
    		LIBS += -L$$(QWTLIB) -Wl,-rpath,$(QWTLIB) -lqwt
  	}

        macx {
                message("caQtDM_QtControls configuration : macx")
 		INCLUDEPATH += $$(QWTINCLUDE)
    		OBJECTS_DIR = obj
		DESTDIR = $$(CAQTDM_COLLECT)
    		CONFIG += lib_bundle
    		LIBS += -F$$(QWTLIB) -framework qwt
  	}

	ios | android {
                message("caQtDM_QtControls configuration : ios or android")
		OBJECTS_DIR = obj
		DESTDIR = $$(CAQTDM_COLLECT)
		INCLUDEPATH += $$(QWTINCLUDE)
   		CONFIG += staticlib
	}

	win32 {
                message("caQtDM_QtControls sconfiguration : win32")
                INCLUDEPATH += $$(QWTINCLUDE)
    		win32-g++ {
      			INCLUDEPATH = $(QWTHOME)/src
      			LIBS += $$(QWTLIB)/libqwt.a
     		}
     		win32-msvc* {
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
      		OBJECTS_DIR = ./obj
      		DESTDIR = $(CAQTDM_COLLECT)
		QMAKE_CXXFLAGS += "-g"
		QMAKE_CFLAGS_RELEASE += "-g"
		CONFIG += Define_Build_Python
   	}

        macx {
                message("caQtDM_Lib configuration : macx")
      		INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
      		QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
      		LIBS += -F$(QWTLIB) -framework qwt
                LIBS += -L$(CAQTDM_COLLECT) -lqtcontrols
      		LIBS += ${EPICSLIB}/libCom.dylib
      		DESTDIR = $(CAQTDM_COLLECT)
      		OBJECTS_DIR = ./obj
      		CONFIG += Define_Build_Python

   	}

	ios | android {
                message("caQtDM_Lib configuration : !os or android")
   		CONFIG += staticlib
                SOURCES +=     fingerswipegesture.cpp
      		HEADERS +=     fingerswipegesture.h
		DESTDIR = $(CAQTDM_COLLECT)
                INCLUDEPATH += ./caQtDM_Plugins
		ios {
      			INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
		}

		android {
      			INCLUDEPATH += $(EPICSINCLUDE)/os/android
                }
	}

	win32 {
                message("caQtDM_Lib configuration : win32")
   		INCLUDEPATH += $$(EPICS_BASE)/include
   		INCLUDEPATH += $$(EPICS_BASE)/include/os/win32

  		win32-msvc* {
        		DEFINES +=_CRT_SECURE_NO_WARNINGS
        		DEFINES += CAQTDM_LIB_LIBRARY
        		TEMPLATE = lib
                        CONFIG += Define_Build_qwt Define_Build_objDirs
                        CONFIG += Define_Build_epics_controls Define_Symbols Define_Build_OutputDir
   		}
   		win32-g++ {
       			EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
			LIBS += $$(QWTLIB)/libqwt.a
			LIBS += $$(QTCONTROLS_LIBS)/release//libqtcontrols.a
			LIBS += $${EPICS_LIBS}/ca.lib
			LIBS += $${EPICS_LIBS}/COM.lib
			QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM_Lib.dll ..\caQtDM_Binaries
   		}		
	}
}
#==========================================================================================================
caQtDM_Viewer {
        DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
        DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"
	CONFIG += Define_Build_objDirs
	unix {
                message("caQtDM_viewer configuration : unix")
                DESTDIR = $(CAQTDM_COLLECT)
  		CONFIG += x11
		DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
		DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"

		!ios:!android {
			message("caQtDM_viewer configuration : !ios!android (all unixes + mac)")
			LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
			LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
			LIBS += -L$(CAQTDM_COLLECT) -L$(CAQTDM_COLLECT)/designer
		}
	}
 	macx {
                message("caQtDM_viewer configuration : macx (only mac)")
                DESTDIR = $(CAQTDM_COLLECT)
   		QMAKE_INFO_PLIST = ./src/Mac/Info.plist
   		APP-FONTS.files = lucida-sans-typewriter.ttf
                APP-FONTS.path = Contents/Resources/fonts
                QMAKE_BUNDLE_DATA += APP-FONTS
   		CONFIG += app_bundle
   		ICON = ./src/caQtDM.icns
                plugins.path = Contents/PlugIns/designer
                plugins.files = $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.dylib
                plugins.files += $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.dylib
                plugins.files += $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.dylib
                caqtdmlibs.path = Contents/Frameworks/
                caqtdmlibs.files = $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib $(CAQTDM_COLLECT)/libqtcontrols.dylib
                QMAKE_BUNDLE_DATA += plugins caqtdmlibs
                calib.path = Contents/Frameworks
                calib.files = $$(EPICS_BASE)/lib/darwin-x86/libca.3.14.12.dylib
                comlib.path = Contents/Frameworks
                comlib.files = $$(EPICS_BASE)/lib/darwin-x86/libCom.3.14.12.dylib
                QMAKE_BUNDLE_DATA += calib comlib
  	}

        ios {
                message("caQtDM_viewer configuration : ios")
                DESTDIR = $(CAQTDM_COLLECT)
   		CONFIG += staticlib
   		LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
   		LIBS += $(CAQTDM_COLLECT)/libqtcontrols.a
   		LIBS += $$(QWTHOME)/lib/libqwt.a
   		LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.a
   		LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.a
   		LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.a
                LIBS += $(CAQTDM_COLLECT)/controlsystems/libdemo_plugin.a
                LIBS += $(CAQTDM_COLLECT)/controlsystems/libepics3_plugin.a
                LIBS += $(CAQTDM_COLLECT)/controlsystems/libepics4_plugin.a

   		LIBS += $$(EPICSLIB)/libca.a
   		LIBS += $$(EPICSLIB)/libCom.a

                QMAKE_INFO_PLIST = $$PWD/caQtDM_Viewer/src/IOS/Info.plist
                ICON = $$PWD/caQtDM_Viewer/src/caQtDM.icns
                APP_ICON.files = $$PWD/caQtDM_Viewer/src/caQtDM.png
                APP_ICON.files += $$PWD/caQtDM_Viewer/src/caQtDM-57.png
                APP_ICON.files += $$PWD/caQtDM_Viewer/src/caQtDM-60@2x.png
                APP_ICON.files += $$PWD/caQtDM_Viewer/src/caQtDM-72.png
                APP_ICON.files += $$PWD/caQtDM_Viewer/src/caQtDM-76.png
                APP_ICON.files += $$PWD/caQtDM_Viewer/src/caQtDM-76@2x.png

                APP1_ICON.files = $$PWD/caQtDM_Viewer/src/caQtDM.icns
                APP_XML_FILES.files = $$PWD/caQtDM_Viewer/caQtDM_IOS_Config.xml
                StartScreen.files += $$PWD/caQtDM_Viewer/src/StartScreen-Landscape.png
                StartScreen.files += $$PWD/caQtDM_Viewer/src/StartScreen-568h@2x.png
                APP-FONTS.files = $$PWD/caQtDM_Viewer/lucida-sans-typewriter.ttf
   		APP-FONTS.path = fonts
   		QMAKE_BUNDLE_DATA += APP_XML_FILES APP_ICON APP1_ICON StartScreen APP-FONTS
   		QMAKE_CFLAGS += -gdwarf-2
   		QMAKE_CXXFLAGS += -gdwarf-2
	}

	android {
                message("caQtDM_viewer configuration : android")
                message($$PWD)
                DESTDIR = $$(CAQTDM_COLLECT)
                CONFIG += staticlib
   		LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.a
   		LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.a
                LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.a
                LIBS += $(CAQTDM_COLLECT)/controlsystems/libdemo_plugin.a
                LIBS += $(CAQTDM_COLLECT)/controlsystems/libepics3_plugin.a
                LIBS += $(CAQTDM_COLLECT)/controlsystems/libepics4_plugin.a
   		LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
   		LIBS += $(CAQTDM_COLLECT)/libqtcontrols.a
   		LIBS += $$(QWTHOME)/lib/libqwt.a
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
		INCLUDEPATH += ../caQtDM_Lib/src
		INCLUDEPATH += ../caQtDM_QtControls/src
		INCLUDEPATH += $(QWTINCLUDE)
		INCLUDEPATH += $(EPICSINCLUDE)
		
		OTHER_FILES += ./src/caQtDM.ico
		DEFINES +=_CRT_SECURE_NO_WARNINGS
		RESOURCES += ./src/caQtDM.qrc
                RC_FILE = ./src/caQtDM.rc

		win32-msvc* {
                        CONFIG += Define_Build_qwt 
                        CONFIG += Define_Build_epics_controls
                        CONFIG += Define_Build_caQtDM_Lib Define_Symbols 
                        CONFIG += Define_Build_OutputDir
		}

                win32-g++ {
	        	EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
			LIBS += $$(QWTLIB)/libqwt.a
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
        DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"
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
                win32-msvc* {
                        CONFIG += Define_Build_OutputDir
                }

                win32-g++ {
			OBJECTS_DIR = obj
			DESTDIR = $(CAQTDM_COLLECT)
                }
        }
}
#==========================================================================================================
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
	       INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/$(PYTHONVERSION)/include/python$(PYTHONVERSION)/
	       LIBS += -L/System/Library/Frameworks/Python.framework/Versions/$(PYTHONVERSION)/lib/ -lpython$(PYTHONVERSION)
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
         LIBS += $$(QWTHOME)/lib/qwtd.lib
     }
    ReleaseBuild {
        message("QWT Release configuration : win32")
        LIBS += $$(QWTHOME)/lib/qwt.lib
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

Define_Build_caQtDM_Lib {
     DebugBuild {
	LIBS += $$(CAQTDM_COLLECT)/debug/caQtDM_Lib.lib
     }
    ReleaseBuild {
	LIBS += $$(CAQTDM_COLLECT)/caQtDM_Lib.lib
    }

}
Define_Build_OutputDir {
     DebugBuild {
	DESTDIR = $$(CAQTDM_COLLECT)/debug
     }
    ReleaseBuild {
	DESTDIR = $$(CAQTDM_COLLECT)
    }

}
Define_ControlsysTargetDir{
	unix:!macx!ios:!android {
		DESTDIR = $(CAQTDM_COLLECT)/controlsystems
	}
        macx {
		DESTDIR = $(CAQTDM_COLLECT)/controlsystems
        }

        ios | android {
                DESTDIR = $(CAQTDM_COLLECT)/controlsystems
        }
        win32 {
                message("adl2ui configuration win32")
                win32-msvc* {
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
		
