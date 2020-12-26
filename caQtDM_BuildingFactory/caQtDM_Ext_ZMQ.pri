
#==========================================================================================================
Define_ZMQ_Lib{
	
        ZEROMQ_INCLUDECHECK=$$(QTHOME)/include
        !equals(ZEROMQ_INCLUDECHECK,$$(ZMQINC)){
            message("No collision with Qt detected, include ZMQ include")
            INCLUDEPATH += $$(ZMQINC)
        }

        unix:!macx {
                 LIBS += -L$$(ZMQLIB) -Wl,-rpath,$$(ZMQLIB) -lzmq
                 #LIBS += $$(ZMQLIB)/libzmq.a


	}
        macx {
                LIBS += $$(ZMQLIB)/libzmq.5.dylib
                QMAKE_POST_LINK += \
                   install_name_tool -change $$(ZMQLIB)/libzmq.5.dylib @rpath/libzmq.5.dylib $$CAQTDM_POSTWORKFILE $$RETURN


        }
        win32 {
	    DebugBuild {
                LIBS += $$(ZMQLIB)/libzmq.lib
	     }
	    ReleaseBuild {
                LIBS += $$(ZMQLIB)/libzmq.lib
	    }
	}
}
