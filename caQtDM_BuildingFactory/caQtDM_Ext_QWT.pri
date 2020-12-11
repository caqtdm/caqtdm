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
