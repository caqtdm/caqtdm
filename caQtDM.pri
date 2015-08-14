#==========================================================================================================
demo_plugin {
        message(“demo_plugin configuration”)
        unix:!macx:!ios:!android {
                message(“demo_plugin configuration unix:!macx:!ios:!android”)
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
	}

        macx {
                message(“demo_plugin configuration macx”)
                INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	plugins.path = Contents/PlugIns/controlsystems
        	plugins.files += $(CAQTDM_COLLECT)/controlsystems/libdemo_plugin.dylib
        }

        win32 {
                message(“demo_plugin configuration win32”)
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
        message(“epics3_plugin configuration”)
        unix:!macx:!ios:!android  {
                message(“epics3_plugin configuration unix:!macx:!ios:!android ”)
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 		LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
	}

        macx {
                message(“epics3_plugin configuration macx”)
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Darwin
 		LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
 		LIBS += $$(EPICSLIB)/libca.dylib
 		LIBS += $$(EPICSLIB)/libCom.dylib
 		plugins.path = Contents/PlugIns/controlsystems
 		plugins.files += $(CAQTDM_COLLECT)/controlsystems/libepics3_plugin.dylib
        }

        ios | android {
                message(“demo_plugin configuration : ios or android”)
                CONFIG += staticlib
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                }
        }


	win32 {
                message(“epics3_plugin configuration win32”)
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
        message(“epics4_plugin configuration”)
        unix:!macx:!ios:!android {
                message(“epics4_plugin configuration unix:!macx:!ios:!android”)
 		INCLUDEPATH   += $(EPICSINCLUDE)/os/Linux
 		LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
 		LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
	}
	
        macx {
                message(“epics4_plugin configuration macx”)
		INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	LIBS += $$(EPICSLIB)/libca.dylib
        	LIBS += $$(EPICSLIB)/libCom.dylib
        	plugins.path = Contents/PlugIns/controlsystems
        	plugins.files += $(CAQTDM_COLLECT)/controlsystems/libepics4_plugin.dylib
        }

        ios | android {
                message(“epics4_plugin configuration : ios or android”)
                CONFIG += staticlib
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                }
        }

	win32 {
                message(“epics4_plugin configuration win32”)
  		
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
        message("caQtDM_QtControls configuration")
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
        message("caQtDM_Lib configuration")
        unix:!macx:!ios:!android  {
                message("caQtDM_Lib configuration : unix:!macx:!ios:!android ")
      		LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
      		LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
      		INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
      		INCLUDEPATH += ./caQtDM_Plugins
      		DESTDIR = $(CAQTDM_COLLECT)

   	}

        macx {
                message("caQtDM_Lib configuration : macx")
      		INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin
      		INCLUDEPATH += ./caQtDM_Plugins
      		QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
      		LIBS += -F$(QWTLIB) -framework qwt
      		LIBS += -L$(CAQTDM_COLLECT) -lqtcontrols
      		LIBS += ${EPICSLIB}/libca.dylib
      		LIBS += ${EPICSLIB}/libCom.dylib
      		DESTDIR = $(CAQTDM_COLLECT)

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
        message("caQtDM_viewer configuration")
        DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
        DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"

	unix {
                message("caQtDM_viewer configuration : unix")
  		CONFIG += x11
		DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
		DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"
        }

        !ios:!android {
                message("caQtDM_viewer configuration : !ios!android (all unixes + mac)")
                LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
                LIBS += -L$(CAQTDM_COLLECT) -L$(CAQTDM_COLLECT)/designer
	}

 	macx {
                message("caQtDM_viewer configuration : macx (only mac)")
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
                include(../../qwt-6.1.1_android/qwt.prf)
   		CONFIG += staticlib
   		LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.a
   		LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.a
   		LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.a
   		LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
   		LIBS += $(CAQTDM_COLLECT)/libqtcontrols.a
   		LIBS += $$(QWTHOME)/lib/libqwt.a
   		LIBS += $$(EPICSLIB)/libca.a
   		LIBS += $$(EPICSLIB)/libCom.a
   		LOCAL_LDLIBS += -llog

   		ICON = ../src/caQtDM.icns$$PWD/src/caQtDM.png
   		APP_ICON.files += $$PWD/src/caqtdm762x.png
   		APP_ICON.path = /res/drawable

   		APP1_ICON.files = $$PWD/src/caQtDM.icns
   		APP_XML_FILES.files = $$PWD/caQtDM_IOS_Config.xml
   		StartScreen.files += $$PWD/src/startscreenlandscape.png
   		StartScreen.path = /res/drawable
   		#StartScreen.files += $$PWD/src/StartScreen-568h@2x.png
   		APP-FONTS.files = $$PWD/lucida-sans-typewriter.ttf
   		APP-FONTS.path = /fonts
   		QMAKE_BUNDLE_DATA += APP_XML_FILES APP_ICON APP1_ICON StartScreen APP-FONTS

   		deployment.files += $$PWD/caQtDM_IOS_Config.xml
   		deployment.path = /assets
   		INSTALLS += deployment APP-FONTS APP_ICON StartScreen

		DESTDIR =/Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Binaries_SIM_ANDROID
		DISTFILES += /Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Viewer/caQtDM1/caQtDM_Viewer/caQtDM1/android/AndroidManifest.xml \
    		caQtDM_Viewer/caQtDM1/android/gradle/wrapper/gradle-wrapper.jar \
    		caQtDM_Viewer/caQtDM1/android/res/values/libs.xml \
    		caQtDM_Viewer/caQtDM1/android/build.gradle \
    		caQtDM_Viewer/caQtDM1/android/gradle/wrapper/gradle-wrapper.properties \
    		caQtDM_Viewer/caQtDM1/android/gradlew \
    		caQtDM_Viewer/caQtDM1/android/gradlew.bat
		ANDROID_PACKAGE_SOURCE_DIR = /Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Viewer/		caQtDM1/caQtDM_Viewer/caQtDM1/android
		#ANDROID_PACKAGE_SOURCE_DIR = $$PWD/caQtDM_Viewer/caQtDM1/android
	}

	win32 {
                message("caQtDM_viewer configuration : win32")
		DEFINES += BUILDTIME=\\\"\\\"
		DEFINES += BUILDDATE=\\\"\\\"
		DEFINES += SUPPORT=\\\"\\\"

		TARGET = caQtDM
		TEMPLATE = app
		include (./caQtDM.pri)
		MOC_DIR = moc
		VPATH += ./src
		UI_DIR += ./
		INCLUDEPATH += .
		INCLUDEPATH += $(QWTHOME)/src
		INCLUDEPATH += $$(EPICS_BASE)/include
		INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
		INCLUDEPATH += ../caQtDM_Lib/src
		INCLUDEPATH += ../caQtDM_QtControls/src
		INCLUDEPATH += $(QWTINCLUDE)
		INCLUDEPATH += $(EPICSINCLUDE)
		DESTDIR = ../$(CAQTDM_COLLECT)
		OTHER_FILES += ./src/caQtDM.ico
		DEFINES +=_CRT_SECURE_NO_WARNINGS
		RESOURCES += ./src/caQtDM.qrc
                RC_FILE = ./src/caQtDM.rc

		win32-msvc* {
                        CONFIG += Define_Build_qwt Define_Build_objDirs
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
caQtDM_adl2ui{
        message("adl2ui configuration")
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
     DebugBuild {
        OBJECTS_DIR = debug/obj
     }
    ReleaseBuild {
        OBJECTS_DIR = release/obj
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
		
