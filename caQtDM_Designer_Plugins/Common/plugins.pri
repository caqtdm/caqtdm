CAQTDM_TOP = ../..
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
CONFIG += Define_Build_caQtDM_QtControls
CONFIG += Define_Build_caQtDM_Lib
CONFIG += Define_Build_qwt
CONFIG += caQtDM_Installation
INSTALLTIONSUBDIR = designer
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_internal_Libraries.pri)
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_Ext_QWT.pri)
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_Installation.pri)

contains(QT_VER_MAJ, 4) {
      CONFIG += plugin qt thread warn_on
      CONFIG += designer
}
contains(QT_VER_MAJ, 5) {
      CONFIG += plugin qt thread warn_on
      QT += widgets uitools opengl
      ios | android {
         greaterThan(QT_MINOR_VERSION, 4) {
            QT += uiplugin
         } else {
            QT += designer
         }
      } else {
          QT += designer
      }
}

TEMPLATE = lib
INCLUDEPATH += ../Common
INCLUDEPATH += ../src
INCLUDEPATH += .




ios | android {
  CONFIG += static
  LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME).a
  LIBS += ../$(CAQTDM_COLLECT)/libqtcontrols.a
  INCLUDEPATH += $(QWTINCLUDE)
  INCLUDEPATH += $$(QWTHOME)/src
  OBJECTS_DIR = obj
}


win32 {
     INCLUDEPATH += $$(QWTHOME)/src
     
     win32-g++ {
             LIBS += $$(QWTLIB)/lib/lib$$(QWTLIBNAME).a
	     LIBS += $$(QTCONTROLS_LIBS)/release/libqtcontrols.a
     }
     win32-msvc* || msvc{
	     CONFIG += Define_Build_OutputDir
	     INCLUDEPATH += $(QWTINCLUDE)
	     CONFIG(DebugBuild, DebugBuild|ReleaseBuild) { 
                     INCLUDEPATH += $(QWTINCLUDE)
                     LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME)d.lib
                    #LIBS += $(CAQTDM_COLLECT)/debug/qtcontrols.lib
	     }

	     ReleaseBuild {
	     CONFIG( ReleaseBuild, DebugBuild|ReleaseBuild) {
	             INCLUDEPATH += $(QWTINCLUDE)
                     LIBS += $$(QWTHOME)/lib/$$(QWTLIBNAME).lib
                     #LIBS += $(CAQTDM_COLLECT)/qtcontrols.lib
	     }
     }  
}



unix:!ios {

   macx: {
      INCLUDEPATH += $(QWTINCLUDE)
      QMAKE_LFLAGS_PLUGIN -= -dynamiclib
      QMAKE_LFLAGS_PLUGIN += -bundle
      LIBS += -F$(QWTLIB) -framework $$(QWTLIBNAME)
      LIBS += -L $(CAQTDM_COLLECT) -lqtcontrols
      QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
   }
}

}
