#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
message("Cannot build package with Qt version $${QT_VERSION}.")
error("Use at least Qt 4.6.")
}

TEMPLATE = subdirs
SUBDIRS = demo epics3 epics4

!MOBILE {
SUBDIRS += bsread
}
