
#==========================================================================================================
epics4_plugin {
        CONFIG += Define_Build_objDirs
        CONFIG +=Define_Build_caQtDM_Lib Define_Build_qtcontrols
        CONFIG +=Define_Build_epicsPV_controls Define_Build_epics_controls
        CONFIG +=Define_Symbols
        CONFIG += Define_Build_objDirs
        CONFIG += Define_library_settings

        unix:!macx:!ios:!android {

        epics7 {
                message("epics4_plugin (with epics version 7) configuration unix:!macx:!ios:!android")

                INCLUDEPATH   += $(EPICSINCLUDE)
                INCLUDEPATH   += $(EPICSINCLUDE)/pv
                INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc

                !EPICS4_STATICBUILD {
                   message( "epics4_plugin build with shared object libraries of epics4" )
                   LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom -lpvAccess -lpvData -lpvaClient -lnt
                   LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                }
                EPICS4_STATICBUILD  {
                   message( "epics4_plugin build with static libraries of epics4" )
                   LIBS += $(EPICSLIB)/libpvAccess.a
                   LIBS += $(EPICSLIB)/libpvData.a
                   LIBS += $(EPICSLIB)/libpvaClient.a
                   LIBS += $(EPICSLIB)/libnt.a
                   LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
                }
                CONFIG += release
          }
          else {
                message("epics4_plugin (with epics version 3) configuration unix:!macx:!ios:!android")
                
                INCLUDEPATH   += $(EPICS4LOCATION)/pvDataCPP/include
                INCLUDEPATH   += $(EPICS4LOCATION)/pvAccessCPP/include
                INCLUDEPATH   += $(EPICS4LOCATION)/pvaClientCPP/include
                INCLUDEPATH   += $(EPICS4LOCATION)/normativeTypesCPP/include
                INCLUDEPATH   += $(EPICSINCLUDE)
                INCLUDEPATH += $(EPICSINCLUDE)/os/Linux
#for epics 3.15 and gcc we need this
                INCLUDEPATH   += $(EPICSINCLUDE)/compiler/gcc
                
                EPICS4LOC1 = $(EPICS4LOCATION)/pvAccessCPP/lib/$(EPICS_HOST_ARCH)
                EPICS4LOC2 = $(EPICS4LOCATION)/pvDataCPP/lib/$(EPICS_HOST_ARCH)
                EPICS4LOC3 = $(EPICS4LOCATION)/pvaClientCPP/lib/$(EPICS_HOST_ARCH)
                EPICS4LOC4 = $(EPICS4LOCATION)/normativeTypesCPP/lib/$(EPICS_HOST_ARCH)

                !EPICS4_STATICBUILD {
                   message( "epics4_plugin build with shared object libraries of epics4" )
                   LIBS += -L$${EPICS4LOC1} -Wl,-rpath,$${EPICS4LOC1} -lpvAccess
                   LIBS += -L$${EPICS4LOC2} -Wl,-rpath,$${EPICS4LOC2} -lpvData
                   LIBS += -L$${EPICS4LOC3} -Wl,-rpath,$${EPICS4LOC3} -lpvaClient
                   LIBS += -L$${EPICS4LOC4} -Wl,-rpath,$${EPICS4LOC4} -lnt
                   LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
                }
                EPICS4_STATICBUILD  {
                   message( "epics4_plugin build with static libraries of epics4" )
                   LIBS += $${EPICS4LOC1}/libpvAccess.a
                   LIBS += $${EPICS4LOC2}/libpvData.a
                   LIBS += $${EPICS4LOC3}/libpvaClient.a
                   LIBS += $${EPICS4LOC4}/libnt.a
                   LIBS += -L$(EPICSLIB) -Wl,-rpath,$(EPICSLIB) -lca -lCom
                }
 		CONFIG += release
            }
	}
	
        macx {
                message("epics4_plugin configuration macx")
                INCLUDEPATH   += $(EPICSINCLUDE)
		INCLUDEPATH += $(EPICSINCLUDE)/os/Darwin

                INCLUDEPATH   += $(EPICS4LOCATION)/pvDataCPP/include
                INCLUDEPATH   += $(EPICS4LOCATION)/pvAccessCPP/include
                INCLUDEPATH   += $(EPICS4LOCATION)/pvaClientCPP/include
                INCLUDEPATH   += $(EPICS4LOCATION)/normativeTypesCPP/include

                EPICS4LOC1 = $(EPICS4LOCATION)/pvAccessCPP/lib/$(EPICS_HOST_ARCH)
                EPICS4LOC2 = $(EPICS4LOCATION)/pvDataCPP/lib/$(EPICS_HOST_ARCH)
                EPICS4LOC3 = $(EPICS4LOCATION)/pvaClientCPP/lib/$(EPICS_HOST_ARCH)
                EPICS4LOC4 = $(EPICS4LOCATION)/normativeTypesCPP/lib/$(EPICS_HOST_ARCH)

                LIBS += $${EPICS4LOC1}/libpvAccess.a
                LIBS += $${EPICS4LOC2}/libpvData.a
                LIBS += $${EPICS4LOC3}/libpvaClient.a
                LIBS += $${EPICS4LOC4}/libnt.a

        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib
        	LIBS += $$(EPICSLIB)/libca.dylib
        	LIBS += $$(EPICSLIB)/libCom.dylib
                CONFIG += release
                CAQTDM_POSTWORKFILE = lib$${CAQTDM_INSTALL_LIB}.dylib
        }

        ios | android {
                message("epics4_plugin configuration : ios or android")
                CONFIG += staticlib
                LIBS += $$OUT_PWD/../../libcaQtDM_Lib.a
                ios {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/iOS
                }
                android {
                        INCLUDEPATH += $(EPICSINCLUDE)/os/android
                }
        }

	win32-msvc* || msvc {
                message("epics4_plugin configuration win32")
		 epics7 {
                    message("epics4_plugin (with epics version 7) configuration win32")
                    QMAKE_CXXFLAGS     += /Zc:twoPhase-
		  }
 		
 
	}
}
