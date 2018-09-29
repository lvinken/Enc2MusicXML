QT      += core
QT      -= gui

CONFIG  += c++11

TARGET   = Enc2MusicXML
TEMPLATE = app
CONFIG  += console
CONFIG  -= app_bundle

SOURCES += analysisfile.cpp \
           encfile.cpp \
           main.cpp \
           mxmlconverter.cpp \
           mxmlwriter.cpp \
           noteconnector.cpp \
           textfile.cpp

HEADERS += analysisfile.h \
           commondefs.h \
           encfile.h \
           mxmlconverter.h \
           mxmlwriter.h \
           noteconnector.h \
           textfile.h
