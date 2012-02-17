TEMPLATE = app
INCLUDEPATH += .

# Input
HEADERS += XmlWriter.h parser.h \
    myParser.h \
    QtProperties.h
SOURCES += myParser.cpp XmlWriter.cpp parser.c \
    QtProperties.c

TARGET = myParser

OTHER_FILES += \
    stylesheet.qss
