
#==========================================================================================================
caQtDM_QtControls {
        CONFIG += Define_Build_qwt
        CONFIG += caQtDM_xdl2ui_Lib
        CONFIG += Define_Build_objDirs
        CONFIG += Define_Build_OutputDir
        CONFIG += Define_library_settings
        unix:!macx:!ios:!android {
                message("caQtDM_QtControls configuration : unix:!macx:!ios:!android")


  	}

        macx {
                message("caQtDM_QtControls configuration : macx")
                #QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/
                #QMAKE_SONAME_PREFIX = @loader_path/../Frameworks/libadlParser.dylib
                #QMAKE_LFLAGS_PLUGIN += -Wl,-install_name,@executable_path/../Frameworks/$${TARGET}.$${QMAKE_EXTENSION_SHLIB}
                modify_adl.depends = $$TARGET
                modify_adl.commands = $$shell_quote(install_name_tool -change libadlParser.dylib @executable_path/../Frameworks/libadlParser.dylib libqtcontrols.dylib $$escape_expand(\n\t))
                modify_edl.depends = $$TARGET
                modify_edl.commands = $$shell_quote(install_name_tool -change libedlParser.dylib @executable_path/../Frameworks/libedlParser.dylib libqtcontrols.dylib $$escape_expand(\n\t))
                RETURN=$$escape_expand(\\n\\t)
                QMAKE_POST_LINK += \
                   install_name_tool -change libadlParser.dylib @executable_path/../Frameworks/libadlParser.dylib libqtcontrols.dylib $$RETURN \
                   install_name_tool -change libedlParser.dylib @executable_path/../Frameworks/libedlParser.dylib libqtcontrols.dylib
  	}

	ios | android {
                message("caQtDM_QtControls configuration : ios or android")
		OBJECTS_DIR = obj

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


     		win32-msvc* || msvc{
                        DEFINES += QTCON_MAKEDLL _CRT_SECURE_NO_WARNINGS

     		}
	}
}
