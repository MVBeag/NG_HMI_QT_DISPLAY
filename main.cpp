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

/*
ubgel200@ubgel200-laptop:~/workspace/QT/creaderasync$ sudo ./creaderasync
Data successfully received from port ttyACM1
iiiiidddDdddddD

it also work with QT but before:
ubgel200@ubgel200-laptop:/dev$ sudo chmod 666 ttyACM1

the next setp is the read the Data from USB and to read or to toggler something
*/

int main(int argc, char *argv[])
{


    /* QML application */
    /*fermer le terminale ferme le qml*/
    QGuiApplication app(argc, argv);
    QQuickView view;
    view.setSource(QUrl("Display.qml"));
    view.show();
    /**/

    /*c'est un peu le même principe qu'avec serial, dans serial on créer un objet dans la classe de base
    QSerialport et à l'aide du référencement on utilise cet objet dans la classe SerialPortReader.
    Ici on créer un pointer sur "myenergy" puis à l'aide du constructeur de ma classe TextData on prend "myenergy" comme
    référence pour la création de mon objet ttext.
    */
    QObject *object = view.rootObject();
    QObject *energy = object->findChild<QObject*>("myenergy"); /*la recherche dans qml*/


#if 0
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/Display.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

//    QTimer::singleShot is used together with a Lambda do find out how to access the dial "myDial"
//    QTimer::singleShot(1000, [&engine]{    qDebug() << engine.rootObjects()[0]->findChild<QObject *>("myDial"); });

    //QObject *dial = engine.rootObjects()[0]->findChild<QObject *>("myDial");
    QObject *text = engine.rootObjects()[0]->findChild<QObject *>("myenergy");
    if(text){
        // make objects dynamic, otherwise they are destroyed after the code block :-)
        //DialData *ddata = new DialData(dial, nullptr);
        TextData *ttext = new TextData(text, nullptr);

        QTimer *timer = new QTimer();
        //QObject::connect(timer, SIGNAL(timeout()), ddata, SLOT(updateValue()));
        QObject::connect(timer, SIGNAL(timeout()), ttext, SLOT(updateValue()));
        timer->start(50);
    }
#endif

#if 0
    QObject *object = view.rootObject();
    QObject *energy = object->findChild<QObject*>("myenergy");
    if (energy)
    {
        energy->setProperty("text", "2");

    }
    /* Quick test to look how to chqnge informations in QML */
#endif


    #if 0
    QObject *object = view.rootObject();
    QObject *rect = object->findChild<QObject*>("rect");
    if (rect)
        rect->setProperty("color", "red");
    QObject *energy = object->findChild<QObject*>("energy");
    if (energy)
        energy->setProperty("text", "1");
    #endif

/***********COM************/
/*dans ce sens l'affichage du QML le fonctionne pas bien*/
    /*sans cela il ne cherche pas à lire ni même à lancer le code lié au COM*/
    //QCoreApplication coreApplication(argc, argv);
/*
    int argumentCount = QCoreApplication::arguments().size();
    QStringList argumentList = QCoreApplication::arguments();
*/

    /* With the Terminal for the error message */
    QTextStream standardOutput(stdout); /*interface to write text*/

    /* Port conf */
    QSerialPort serialPort; /*creation de l'objet*/
    QString serialPortName = "ttyACM1";//argumentList.at(1);
    serialPort.setPortName(serialPortName);
    int serialPortBaudRate = QSerialPort::Baud115200;//(argumentCount > 2) ? argumentList.at(2).toInt() : QSerialPort::Baud115200;
    serialPort.setBaudRate(serialPortBaudRate);
    if (!serialPort.open(QIODevice::ReadOnly)) {
        standardOutput << QObject::tr("Failed to open port %1, error: %2").arg(serialPortName).arg(serialPort.errorString()) << endl;
        return 1;
    }

    /* Ok a crée un objet de type QSerialPort et là on prend ça référence pour crée un objet
    dans SerialPortReader, le but étant de manipulant seriaPort avec les methode de SerialPortReader
    */
    /*création de l'objet serialPortReader en appelant le constructeur*/
    SerialPortReader serialPortReader(&serialPort); /* Class SerialPortReader, object seiralPortReader*/


    if (energy)
    {
        TextData *ttext = new TextData(energy, nullptr); /*le constructeur, définition du pointeur avec * */
        QObject::connect(&serialPortReader, SIGNAL(newValueReady(QString)),ttext , SLOT(updateValue(QString)));
    }
/***********COM************/




    /*sans cela il ne cherche pas à lire ni même à lancer le code lié au COM*/
    /*l'affichage est pas bon*/
    //return coreApplication.exec();
    return app.exec();

}