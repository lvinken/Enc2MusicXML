QT      += core
QT      -= gui

CONFIG  += c++11

TARGET   = Enc2MusicXMLdev
TEMPLATE = app
CONFIG  += console
CONFIG  -= app_bundle

SOURCES += encfile.cpp \
           main.cpp \
           mxmlconvert.cpp \
           mxmlwriter.cpp \
           textfile.cpp

HEADERS += commondefs.h \
           encfile.h \
           mxmlconvert.h \
           mxmlwriter.h \
           textfile.h
