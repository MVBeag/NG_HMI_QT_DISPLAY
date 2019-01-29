QT += core quick qml serialport

CONFIG -= app_bundle

TARGET = creaderasync
TEMPLATE = app

HEADERS += \
    serialportreader.h \
    textdata.h \
    serialportwriter.h

SOURCES += \
    main.cpp \
    serialportreader.cpp \
    textdata.cpp \
    serialportwriter.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/creaderasync
INSTALLS += target

RESOURCES += \
    qmake_qmake_immediate.qrc
