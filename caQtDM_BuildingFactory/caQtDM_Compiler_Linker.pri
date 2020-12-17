Define_Symbols{
        unix:!macx!ios:!android {
        }
        macx {
            QMAKE_MACOSX_DEPLOYMENT_TARGET=10.13

        }
        ios{

        }
        android{

        }

    win32 {
        ReleaseBuild {
            QMAKE_CXXFLAGS += /Z7
            QMAKE_CFLAGS   += /Z7
            QMAKE_LFLAGS   += /DEBUG /OPT:REF /OPT:ICF
        }
    }
}

Define_Build_objDirs {
        MOC_DIR = moc
        unix:!macx!ios:!android {
                OBJECTS_DIR = obj
        }
        macx {
                OBJECTS_DIR = obj
        }
        ios{

        }
        android{

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

