CAQTDM_TOP = ../..
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_BuildControl_Version.pri)
CONFIG += Define_Build_caQtDM_QtControls
CONFIG += Define_Build_caQtDM_Lib_include
CONFIG += Define_Build_qwt
CONFIG += Define_Build_objDirs
CONFIG += Define_Symbols
CONFIG += caQtDM_Installation
INSTALLTIONSUBDIR = designer
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_internal_Libraries.pri)
include($$CAQTDM_TOP/caQtDM_BuildingFactory/caQtDM_Compiler_Linker.pri)
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
     
     win32-g++ {
             LIBS += $$(QWTLIB)/lib/lib$$(QWTLIBNAME).a
	     LIBS += $$(QTCONTROLS_LIBS)/release/libqtcontrols.a
     }
     win32-msvc* || msvc{



}
}



unix:!ios {

   macx: {
      QMAKE_LFLAGS_PLUGIN -= -dynamiclib
      QMAKE_LFLAGS_PLUGIN += -bundle

   }
}


