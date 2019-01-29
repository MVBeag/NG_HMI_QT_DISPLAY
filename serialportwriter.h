#ifndef SERIALPORTWRITER_H
#define SERIALPORTWRITER_H

#include <QtSerialPort/QSerialPort>

#include <QTextStream>
#include <QTimer>
#include <QByteArray>
#include <QObject>

class SerialPortWriter : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortWriter(QSerialPort *serialPort, QObject *parent = nullptr);
    ~SerialPortWriter();
    void write(const char *writeData, qint64 len);
    Q_INVOKABLE void writeTest();
signals:

public slots:

private:
    QSerialPort     *m_serialPort;
    QTextStream     m_standardOutput;
    QTimer          m_timer;
};

#endif // SERIALPORTWRITER_H
