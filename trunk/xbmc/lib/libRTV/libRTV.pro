include(../../../boxee.pri)

TEMPLATE = lib
CONFIG += lib_bundle staticlib
CONFIG -= app_bundle
QT += core gui opengl

MOC_DIR=build
OBJECTS_DIR=build
DEFINES +=  _LINUX __unix__
SOURCES += \


HEADERS += \


INCLUDEPATH += ../../../
