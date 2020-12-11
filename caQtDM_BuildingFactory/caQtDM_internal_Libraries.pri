Define_Build_caQtDM_QtControls {
	INCLUDEPATH += $$CAQTDM_TOP/caQtDM_QtControls/src

	unix:!macx:!ios:!android {
		LIBS += -L$(CAQTDM_COLLECT) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
	}
        macx {
        	LIBS += $(CAQTDM_COLLECT)/libqtcontrols.dylib
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
			
			LIBS += $$CAQTDM_TOP/caQtDM_QtControls/release/qtcontrols.lib
			
		    }

                }

                win32-g++ {
                	LIBS += libqtcontrols.a
                }
        }
}

Define_Build_caQtDM_Lib {
     DebugBuild {
       
	LIBS += $$CAQTDM_TOP/caQtDM_Lib/debug/caQtDM_Lib.lib
     }
    ReleaseBuild {
     
	LIBS += $$CAQTDM_TOP/caQtDM_Lib/release/caQtDM_Lib.lib
    }

}
