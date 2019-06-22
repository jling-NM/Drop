#-------------------------------------------------
#
# Project created by QtCreator 2019-01-27T18:44:53
#
#-------------------------------------------------

QT += core gui

CONFIG += qwt

unix:INCLUDEPATH +=/home/josef/apps/qwt-6.1.4/include
unix:LIBS += -L/home/josef/apps/qwt-6.1.4/lib -lqwt

win32:INCLUDEPATH += C:\Qwt-6.1.4\include
win32:LIBS += C:\Qwt-6.1.4\lib\qwt.dll
win32:INCLUDEPATH += "C:\Program Files (x86)\National Instruments\NI-DAQ\DAQmx ANSI C Dev\include"
win32:LIBS += "C:\Windows\System32\nicaiu.dll"


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Drop
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

# disable unused parameter warnings
QMAKE_CXXFLAGS = -Wno-unused-parameter


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        experiment.cpp \
        sensorsettingsdialog.cpp \
        newexperimentdialog.cpp \
        spinintaction.cpp

HEADERS += \
        mainwindow.h \
        experiment.h \
        sensorsettingsdialog.h \
        newexperimentdialog.h \
        spinintaction.h

FORMS += \
        mainwindow.ui \
        sensorsettingsdialog.ui \
        newexperimentdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    application.qrc
