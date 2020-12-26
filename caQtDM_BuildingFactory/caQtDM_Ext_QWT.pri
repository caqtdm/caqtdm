Define_Build_qwt {

    INCLUDEPATH += $$(QWTINCLUDE)
        unix:!macx:!ios:!android {
                LIBS += -L$$(QWTLIB) -Wl,-rpath,$(QWTLIB) -l$$(QWTLIBNAME)
  	}

        macx {
		LIBS += -F$$(QWTLIB) -framework $$(QWTLIBNAME)
                QMAKE_POST_LINK += install_name_tool -change qwt.framework/Versions/6/qwt @rpath/qwt.framework/Versions/6/qwt $$CAQTDM_POSTWORKFILE $$RETURN
  	}

	ios | android {
        }

	win32 {
	
    		win32-g++ {
      			INCLUDEPATH = $(QWTHOME)/src
                        LIBS += $$(QWTLIB)/lib$$(QWTLIBNAME).a
     		}
     		win32-msvc* || msvc{
		     DebugBuild {
			 message("QWT Debug configuration : win32")
			 LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME)d.lib
		     }
		    ReleaseBuild {
			message("QWT Release configuration : win32")
			LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME).lib
		    }
	    }
	}
}
Define_install_qwt{
    macx {
        qwtframework.path = Contents/Frameworks
        qwtframework.files = $$(QWTHOME)/lib/$$(QWTLIBNAME).framework
        QMAKE_BUNDLE_DATA += qwtframework
    }
}
