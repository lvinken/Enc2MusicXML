QT      += core
QT      -= gui

CONFIG  += c++11

TARGET   = Enc2MusicXML
TEMPLATE = app
CONFIG  += console
CONFIG  -= app_bundle

SOURCES += main.cpp \
           encfile.cpp \
           mxmlfile.cpp \
           textfile.cpp

HEADERS += encfile.h \
           mxmlfile.h \
           textfile.h
