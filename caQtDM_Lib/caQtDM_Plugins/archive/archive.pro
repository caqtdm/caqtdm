#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.6.")
}
include (../../../caQtDM_Viewer/qtdefs.pri)


TEMPLATE = subdirs
archiveSF: {
   SUBDIRS += archiveSF
}
!MOBILE {
  archiveHIPA: {
   SUBDIRS += archiveHIPA
  }
  archivePRO: {
   SUBDIRS += archivePRO
  }
  archiveCA: {
   SUBDIRS += archiveCA
  }
}
