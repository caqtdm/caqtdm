Define_Build_caQtDM_QtControls {
	INCLUDEPATH += $$CAQTDM_TOP/caQtDM_QtControls/src

	unix:!macx:!ios:!android {
                LIBS += -L$$CAQTDM_TOP/caQtDM_QtControls -Wl,-rpath,$$CAQTDM_TOP/caQtDM_QtControls -lqtcontrols
	}
        macx {
                LIBS += $$CAQTDM_TOP/caQtDM_QtControls/libqtcontrols.dylib
                QMAKE_RPATHDIR += $$CAQTDM_TOP/caQtDM_QtControls
        }

        ios | android {
        	LIBS += $$OUT_PWD/../../../caQtDM_QtControls/libqtcontrols.a
        }

        win32 {
                win32-msvc* || msvc{
		     DebugBuild {
			#LIBS += $$(CAQTDM_COLLECT)/debug/qtcontrols.lib
		     }
		    ReleaseBuild {
			
                        LIBS += $$OUT_PWD/$$CAQTDM_TOP/caQtDM_QtControls/release/qtcontrols.lib
			
		    }

                }

                win32-g++ {
                	LIBS += libqtcontrols.a
                }
        }
}

Define_Build_caQtDM_Lib {
    CONFIG += Define_Build_caQtDM_Lib_include
    unix:!macx {
        LIBS += -L$$CAQTDM_TOP/caQtDM_Lib -Wl,-rpath,$$CAQTDM_TOP/caQtDM_Lib -lcaQtDM_Lib
    }
    macx {
            LIBS += $$CAQTDM_TOP/caQtDM_Lib/libcaQtDM_Lib.dylib
            QMAKE_RPATHDIR += $$CAQTDM_TOP/caQtDM_Lib
    }

    ios | android {

    }



    win32 {
         DebugBuild {

            LIBS += $$CAQTDM_TOP/caQtDM_Lib/debug/caQtDM_Lib.lib
         }
        ReleaseBuild {

            LIBS +=$$OUT_PWD/$$CAQTDM_TOP/caQtDM_Lib/release/caQtDM_Lib.lib
        }
    }
}
Define_Build_caQtDM_Lib_include {
    INCLUDEPATH += $$CAQTDM_TOP/caQtDM_Lib/src
}


caQtDM_Plugin_Interface{
    INCLUDEPATH += $$CAQTDM_TOP/caQtDM_Lib/caQtDM_Plugins
}



caQtDM_xdl2ui_Lib{
        unix:!macx {

            ADL_EDL_FILES {
                LIBS += -L$$CAQTDM_TOP/caQtDM_Parsers/adlParserSharedLib -Wl,-rpath,$$CAQTDM_TOP/caQtDM_Parser/adlParserSharedLib -ladlParser
                LIBS += -L$$CAQTDM_TOP/caQtDM_Parsers/edlParserSharedLib -Wl,-rpath,$$CAQTDM_TOP/caQtDM_Parser/edlParserSharedLib -ledlParser
            }
            #LIBS += $$CAQTDM_TOP/caQtDM_Parsers/adlParserStaticLib/libadlParser.a
            #ADL_EDL_FILES {
            #   LIBS += $$CAQTDM_TOP/caQtDM_Parsers/edlParserStaticLib/libedlParser.a
            #}


        }

        macx {
            ADL_EDL_FILES {
                LIBS += $$CAQTDM_TOP/caQtDM_Parsers/adlParserSharedLib/libadlParser.dylib
                LIBS += $$CAQTDM_TOP/caQtDM_Parsers/edlParserSharedLib/libedlParser.dylib
                QMAKE_RPATHDIR += $$CAQTDM_TOP/caQtDM_Parsers/adlParserSharedLib/
                QMAKE_RPATHDIR += $$CAQTDM_TOP/caQtDM_Parsers/edlParserSharedLib/
            }
        }

        win32 {
                win32-msvc* || msvc{
                        #LIBS += $(CAQTDM_COLLECT)/adlParser.lib
                        LIBS += $$OUT_PWD/$$CAQTDM_TOP/caQtDM_Parsers/adlParserStaticLib/release/adlParser.lib
                }

                win32-g++ {
                        LIBS += $(CAQTDM_COLLECT)/libadlParser.a
                }
        }

}
