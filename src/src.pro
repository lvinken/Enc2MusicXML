QT      += core
QT      -= gui

CONFIG  += c++11

TARGET   = Enc2MusicXML
TEMPLATE = app
CONFIG  += console
CONFIG  -= app_bundle

SOURCES += encfile.cpp \
           main.cpp \
           mxmlconverter.cpp \
           mxmlwriter.cpp \
           textfile.cpp

HEADERS += commondefs.h \
           encfile.h \
           mxmlconverter.h \
           mxmlwriter.h \
           textfile.h
