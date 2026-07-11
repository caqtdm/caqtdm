include(../tst_plugins.pri)

QT += testlib
contains(QT_VER_MAJ, 5) {
    CONFIG += c++11
}
contains(QT_VER_MAJ, 6) {
    CONFIG += c++17
}

QMAKE_RPATHDIR += $$(PVXS)/lib/$$(EPICS_HOST_ARCH)
QMAKE_RPATHDIR += $$(EPICSLIB)

SOURCES += tst_pvxs.cpp \
    tst_pvxsValueMapping.cpp

HEADERS += \
    tst_pvxsValueMapping.h

# --- Tested classes below (compiled directly - no network/Context/server involved) ---

HEADERS += ../../../caQtDM_Plugins/pvxs/pvxsValueMapping.h

SOURCES += ../../../caQtDM_Plugins/pvxs/pvxsValueMapping.cpp

INCLUDEPATH += ../../../caQtDM_Lib/src \
    ../../../caQtDM_Plugins/pvxs \
    $$(PVXS)/include \
    $$(EPICSINCLUDE)

LIBS += -L$$(PVXS)/lib/$$(EPICS_HOST_ARCH) -lpvxs
LIBS += -L$$(EPICSLIB) -lCom

unix:!macx {
    INCLUDEPATH += $$(EPICSINCLUDE)/os/Linux \
        $$(EPICSINCLUDE)/compiler/gcc
    LIBS += -L$$(PVXS)/lib/$$(EPICS_HOST_ARCH) -levent_core -levent_pthreads
}
macx {
    INCLUDEPATH += $$(EPICSINCLUDE)/os/Darwin \
        $$(EPICSINCLUDE)/compiler/clang
    LIBS += $$(PVXS)/lib/$$(EPICS_HOST_ARCH)/libevent_core.dylib
    LIBS += $$(PVXS)/lib/$$(EPICS_HOST_ARCH)/libevent_pthreads.dylib
}
