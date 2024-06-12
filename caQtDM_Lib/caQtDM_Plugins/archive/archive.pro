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

# Only build archiveHTTP for Qt 5.15 and upwards
greaterThan(QT_VERSION, "5 14 9") {
    SUBDIRS += archiveHTTP
	message("Building ArchiveHTTP")
} else {
    message("Qt version $${QT_VERSION} is less than Qt 5.15, plugin archiveHTTP will NOT be built.")
}
