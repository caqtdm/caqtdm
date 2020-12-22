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
        QMAKE_DISTCLEAN += moc
        unix:!macx!ios:!android {
                QMAKE_DEL_FILE = rm -rf
                OBJECTS_DIR = obj
                QMAKE_DISTCLEAN += obj
        }
        macx {
                QMAKE_DEL_FILE = rm -rf
                OBJECTS_DIR = obj
                QMAKE_DISTCLEAN += obj
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
                QMAKE_DEL_FILE = $$QMAKE_DEL_DIR
                OBJECTS_DIR = release/obj
                QMAKE_DISTCLEAN += release/obj
            }
        }
}
Define_library_settings{
        unix:!macx!ios:!android {
        }
        macx {
            QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/
        }
        ios{

        }
        android{

        }

        win32 {
            DebugBuild {
             }
            ReleaseBuild {
            }
        }

}
