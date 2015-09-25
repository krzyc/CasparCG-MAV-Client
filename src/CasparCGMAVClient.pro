#-------------------------------------------------
#
# Project created by QtCreator 2015-03-27T12:48:46
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CasparCGMACClient
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    casparosclistener.cpp \
    ccgmavcconfig.cpp

HEADERS  += mainwindow.h \
    casparosclistener.h \
    ccgmavcconfig.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    Images/CasparCG.ico \
    resources.rc

win32:RC_FILE = resources.rc

RESOURCES += \
    resources.qrc

CONFIG(system-oscpack) {
    LIBS += -loscpack
} else {
    DEPENDPATH += $$PWD/../lib/oscpack/include
    INCLUDEPATH += $$PWD/../lib/oscpack/include
    win32:LIBS += -L$$PWD/../lib/oscpack/lib/win32/ -loscpack
    else:macx:LIBS += -L$$PWD/../lib/oscpack/lib/macx/ -loscpack
    else:unix:LIBS += -L$$PWD/../lib/oscpack/lib/linux/ -loscpack
}
