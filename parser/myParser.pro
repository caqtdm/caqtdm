TEMPLATE = app
INCLUDEPATH += .

MOC_DIR = moc
OBJECTS_DIR = obj
DESTDIR = .

# Input
HEADERS += XmlWriter.h parser.h \
    myParser.h \
    QtProperties.h \
    dmsearchfile.h
SOURCES += myParser.cpp XmlWriter.cpp parser.c \
    QtProperties.c \
    dmsearchfile.cpp

TARGET = adl2ui

OTHER_FILES += \
    stylesheet.qss
