Define_Build_Python {
     PYTHONCALC: {
        warning("for image and visibility calculation, python will be build in")
	!ios {
	!android {
	   unix:!macx {
	      DEFINES += PYTHON
	      INCLUDEPATH += $(PYTHONINCLUDE)
	      LIBS += -L$(PYTHONLIB) -Wl,-rpath,$(PYTHONLIB) -lpython$(PYTHONVERSION)
	    }
	    unix:macx {
	       DEFINES += PYTHON
	       INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/$(PYTHONVERSION)/include/python$(PYTHONVERSION)/
	       LIBS += -L/System/Library/Frameworks/Python.framework/Versions/$(PYTHONVERSION)/lib/ -lpython$(PYTHONVERSION)
	    }
        }
        }
    }
}
