QT      += core
QT      += qml
QT      += quick

CONFIG  += c++11

TARGET   = Enc2MusicXML
TEMPLATE = app
CONFIG  += console
CONFIG  -= app_bundle

SOURCES += analysisfile.cpp \
           converter.cpp \
           encfile.cpp \
           main.cpp \
           mxmlconverter.cpp \
           mxmlwriter.cpp \
           noteconnector.cpp \
           textfile.cpp

HEADERS += analysisfile.h \
           converter.h \
           commondefs.h \
           encfile.h \
           mxmlconverter.h \
           mxmlwriter.h \
           noteconnector.h \
           textfile.h

RESOURCES += qml.qrc
