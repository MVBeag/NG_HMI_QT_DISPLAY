QT = core
QT += \
    serialport \
    quick

#CONFIG += console
CONFIG -= app_bundle

TARGET = creaderasync
TEMPLATE = app

HEADERS += \
    serialportreader.h \
    textdata.h

SOURCES += \
    main.cpp \
    serialportreader.cpp \
    textdata.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/creaderasync
INSTALLS += target

DISTFILES += \
    Display.qml
