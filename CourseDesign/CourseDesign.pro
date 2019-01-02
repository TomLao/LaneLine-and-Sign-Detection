
##D:\\app\\opencv\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt2.xml

#TEMPLATE = app
#CONFIG += console c++11
#CONFIG -= app_bundle
#CONFIG -= qt

#SOURCES += \
        main.cpp
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CourseDesign
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

SOURCES += \
    main.cpp \
    lanedetector.cpp

INCLUDEPATH += D:\app\opencv\opencv\build\include \
            D:\app\opencv\opencv\build\include\opencv \
            D:\app\opencv\opencv\build\include\opencv2

LIBS += D:\app\opencv\opencv\build\x64\vc15\lib\opencv_world341d.lib

HEADERS += \
    lanedetector.h

DISTFILES +=

