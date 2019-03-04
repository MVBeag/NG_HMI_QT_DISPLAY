QT += core quick qml serialport

CONFIG -= app_bundle

TARGET = creaderasync
TEMPLATE = app

HEADERS += \
    serialportreader.h \
    textdata.h \
    serialportwriter.h \
    Protocole_LE/lib/mem/ucBuffer.h \
    Protocole_LE/lib/prot/protocol.h \
    Protocole_LE/lib/prot/services/generator.h

SOURCES += \
    main.cpp \
    serialportreader.cpp \
    textdata.cpp \
    serialportwriter.cpp \
    Protocole_LE/lib/mem/ucBuffer.c \
    Protocole_LE/lib/prot/protocol.c \
    Protocole_LE/lib/prot/services/generator.c

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/creaderasync
INSTALLS += target

RESOURCES += \
    qmake_qmake_immediate.qrc

INCLUDEPATH += "Protocole_LE/lib"
