/****************************************************************************
**
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "serialportreader.h"
#include "serialportwriter.h"
#include "textdata.h"
#include <QtSerialPort/QSerialPort>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QStringList>
#include <QtQuick>
#include <QObject>
#include <QDebug>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QString>

QT_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    view.setSource(QUrl("qrc:/Display.qml"));
    view.show();

    QObject *object = view.rootObject();
    QObject *energy = object->findChild<QObject*>("myenergy"); /*Return the Child of the Object*/

    /* To display message on the terminal */
    QTextStream standardOutput(stdout); /*interface to write text*/

    /* Port configuration */
    QSerialPort serialPort;
    QString serialPortName = "/dev/ttymxc1";
    serialPort.setPortName(serialPortName);
    int serialPortBaudRate = QSerialPort::Baud115200;
    serialPort.setBaudRate(serialPortBaudRate);



    serialPort.open(QIODevice::WriteOnly);

    SerialPortWriter serialPortWriter(&serialPort);
    const char t_data[] = {0xa5,0x05,0x00,0x00,0x10,0x18,0xff,0xd7,'I'};
    view.rootContext()->setContextProperty("myserialPortWriter", &serialPortWriter);
    view.rootContext()->setContextProperty("myt_data", &t_data);
    serialPortWriter.write(t_data, 9);
    

    if (energy)
    {
        TextData *ttext = new TextData(energy, nullptr);
        //QObject::connect(&serialPortReader, SIGNAL(newValueReady(QString)),ttext , SLOT(updateValue(QString)));
    }

    return app.exec();
}
