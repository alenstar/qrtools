#-------------------------------------------------
#
# Project created by QtCreator 2017-12-23T21:05:47
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(qzxing/src/QZXing.pri)

TARGET = qrtools
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += ELPP_QT_LOGGING    \
          ELPP_FEATURE_ALL \
          ELPP_STL_LOGGING   \
          ELPP_STRICT_SIZE_CHECK ELPP_UNICODE \
          ELPP_MULTI_LOGGER_SUPPORT \
          ELPP_THREAD_SAFE

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += easyloggingpp/src \
                qrcodegen/cpp \
                qzxing/src

SOURCES += \
        main.cpp \
        qrdialog.cpp \
    easyloggingpp/src/easylogging++.cc \
    qrcodegen/cpp/BitBuffer.cpp \
    qrcodegen/cpp/QrCode.cpp \
    qrcodegen/cpp/QrSegment.cpp \
    utils.c

HEADERS += \
        qrdialog.h \
    easyloggingpp/src/easylogging++.h \
    qrcodegen/cpp/BitBuffer.hpp \
    qrcodegen/cpp/QrCode.hpp \
    qrcodegen/cpp/QrSegment.hpp \
    utils.h

FORMS += \
        qrdialog.ui
