QT += core serialport
QT -= gui

CONFIG += c++11

TARGET = qtNet485v4
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ThreadSlave.cpp \
    Net485.cpp

HEADERS += \
    ThreadSlave.h \
    Net485.h
