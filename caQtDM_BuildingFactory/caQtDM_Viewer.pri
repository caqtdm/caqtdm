
#==========================================================================================================
caQtDM_Viewer {
        C_QMAKESPEC = $$QMAKESPEC
        DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
        DEFINES += BUILDARCH=\\\"$$replace(C_QMAKESPEC, \\\\, /)\\\"
        CONFIG += Define_Build_objDirs
        CONFIG += Define_Build_caQtDM_Lib
        CONFIG += Define_Build_caQtDM_QtControls
        CONFIG += caQtDM_xdl2ui_Lib
        CONFIG += Define_Build_qwt
        CONFIG += caQtDM_Plugin_Interface

        unix: {
                message("caQtDM_viewer configuration : unix")
                DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
                DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"

                !ios:!android {
                        message("caQtDM_viewer configuration : !ios!android (all unixes + mac)")

                 !macx:{CONFIG += x11}
                }
        }
        macx:!ios {
                message("caQtDM_viewer configuration : macx (only mac)")

#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc
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
                plugins.files += $(CAQTDM_COLLECT)/designer/libqtcontrols_utilities_plugin.dylib
                caqtdmlibs.path = Contents/Frameworks/
                caqtdmlibs.files = $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib $(CAQTDM_COLLECT)/libqtcontrols.dylib
                ADL_EDL_FILES {
                           caqtdmlibs.files += $(CAQTDM_COLLECT)/libadlParser.dylib
                           caqtdmlibs.files += $(CAQTDM_COLLECT)/libedlParser.dylib
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
                plugins_epics3.files += $(CAQTDM_COLLECT)/controlsystems/libepics3_plugin.dylib
                QMAKE_BUNDLE_DATA += plugins_epics3
                plugins_demo.path = Contents/PlugIns/controlsystems
                plugins_demo.files += $(CAQTDM_COLLECT)/controlsystems/libdemo_plugin.dylib
                QMAKE_BUNDLE_DATA += plugins_demo
                plugins_environment.path = Contents/PlugIns/controlsystems
                plugins_environment.files += $(CAQTDM_COLLECT)/controlsystems/libenvironment_plugin.dylib
                QMAKE_BUNDLE_DATA += plugins_environment

                epics4: {
                                    plugins_epics4.path = Contents/PlugIns/controlsystems
                                    plugins_epics4.files += $(CAQTDM_COLLECT)/controlsystems/libepics4_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_epics4
                                }
                bsread:{
                                    plugins_bsread.path = Contents/PlugIns/controlsystems
                                    plugins_bsread.files += $(CAQTDM_COLLECT)/controlsystems/libbsread_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_bsread
                                    zmqlibrary.path = Contents/Frameworks
                                    zmqlibrary.files += $$(ZMQLIB)/libzmq.5.dylib
                                    QMAKE_BUNDLE_DATA += zmqlibrary
                                }
                archiveSF:{
                                    plugins_archiveSF.path = Contents/PlugIns/controlsystems
                                    plugins_archiveSF.files += $(CAQTDM_COLLECT)/controlsystems/libarchiveSF_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_archiveSF
                                }
                modbus: {
                                    plugins_modbus.path = Contents/PlugIns/controlsystems
                                    plugins_modbus.files += $(CAQTDM_COLLECT)/controlsystems/libmodbus_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_modbus
                                }

                gps: {
                                    plugins_gps.path = Contents/PlugIns/controlsystems
                                    plugins_gps.files += $(CAQTDM_COLLECT)/controlsystems/libgps_plugin.dylib
                                    QMAKE_BUNDLE_DATA += plugins_gps
                                }
        }

        ios {
                    message("caQtDM_viewer configuration : ios")
                    #DESTDIR = $(CAQTDM_COLLECT)

                    message( $$OUT_PWD )

                    CONFIG += staticlib
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
                    target.value = 12.0

                    QMAKE_MAC_XCODE_SETTINGS += target
                    assetIcon.name = ASSETCATALOG_COMPILER_APPICON_NAME
                    assetIcon.value = AppIcon
                    launchImage.name = ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME
                    launchImage.value = LaunchImage
                    QMAKE_MAC_XCODE_SETTINGS += assetIcon launchImage

                    QMAKE_INFO_PLIST += $$PWD/caQtDM_Viewer/src/IOS/Info.plist
                    QMAKE_ASSET_CATALOGS += $$PWD/caQtDM_Viewer/src/IOS/Assets.xcassets

#actually, I have a problem with ios, it seems that iphonesimulator is defined too and wants than the library of simlator
# I comment it out now
                    iphonesimulator {
                         message("caQtDM_viewer configuration : iphonesimulator")
                         # when .dylib and .a in same directory, macos takes .dylib, so separate the libraries
                         LIBS += $$(EPICSLIB)/static/libca.a
                         LIBS += $$(EPICSLIB)/static/libCom.a
                         LIBS += $$(QWTHOME)/lib/lib$$(QWTLIBNAME).a
                         #LIBS += $$(QWTHOME)/lib/lib$$(QWTLIBNAME)_iphonesimulator.a
                         # build simulator only for 32 bit
                         INCLUDEPATH += $$(QWTHOME)/src
                    }
                    iphoneos {
                        message("caQtDM_viewer configuration : iphoneos")

                         LIBS += $$(EPICSLIB)/static/libca.a
                         LIBS += $$(EPICSLIB)/static/libCom.a
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

                         #provisioning_profile_spec.name=PROVISIONING_PROFILE_SPECIFIER
                         #provisioning_profile_spec.value = caQtDM Distribution
                         #provisioning_profile_spec.value = caQtDM Development
                         #QMAKE_MAC_XCODE_SETTINGS += provisioning_profile_spec

                         #signing_identity.name = CODE_SIGN_IDENTITY
                         #signing_identity.value = $$(CODE_SIGN_IDENTITY)
                         #QMAKE_MAC_XCODE_SETTINGS += signing_identity

                         #setting.name = DEVELOPMENT_TEAM
                         #setting.value = $$(CERTIFICATNUMBER)
                         #QMAKE_MAC_XCODE_SETTINGS += setting

                         #payload_dir.target = $$OUT_PWD/Release-iphoneos/Payload
                         #payload_dir.commands = @test -d $$OUT_PWD/Release-iphoneos/Payload || mkdir -p $$OUT_PWD/Release-iphoneos/Payload
                         #payload_dir.depends = $$OUT_PWD/Release-iphoneos

                         #QMAKE_EXTRA_TARGETS +=payload_dir
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
