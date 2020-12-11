Define_Symbols{
    ReleaseBuild {
        QMAKE_CXXFLAGS += /Z7
        QMAKE_CFLAGS   += /Z7
        QMAKE_LFLAGS   += /DEBUG /OPT:REF /OPT:ICF
    }
}

Define_Build_objDirs {
        unix:!macx!ios:!android {
                OBJECTS_DIR = obj
        }
        macx {
                OBJECTS_DIR = obj
        }
        win32 {
            DebugBuild {
                OBJECTS_DIR = debug/obj
             }
            ReleaseBuild {
                OBJECTS_DIR = release/obj
            }
        }
}

