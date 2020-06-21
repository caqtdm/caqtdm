#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.6.")
}
include (../../caQtDM_Viewer/qtdefs.pri)


TEMPLATE = subdirs
SUBDIRS = demo epics3 archive environment
  contains(QT_VER_MAJ, 5) {
    greaterThan(QT_VER_MIN, 8){
        # the reason is the modbus impementation in Qt
        modbus{
          SUBDIRS += modbus
        }
        gps{
          SUBDIRS += gps
        }
    }

}




!MOBILE {
    epics4: {
     SUBDIRS += epics4
    }
    bsread: {
      SUBDIRS += bsread
    }
}
