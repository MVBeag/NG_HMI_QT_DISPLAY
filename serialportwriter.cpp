#include "serialportwriter.h"

#include <QCoreApplication>

QT_USE_NAMESPACE

SerialPortWriter::SerialPortWriter(QSerialPort *serialPort, QObject *parent)
    : QObject(parent)
    , m_serialPort(serialPort)
    , m_standardOutput(stdout)
{
    m_timer.setSingleShot(true);
}

SerialPortWriter::~SerialPortWriter()
{
}

void SerialPortWriter::write(const char *writeData, qint64 len)
{
    const qint64 bytesWritten = m_serialPort->write(writeData, len);

    if (bytesWritten == -1) {
        m_standardOutput << QObject::tr("Failed to write the data to port %1, error: %2")
                          .arg(m_serialPort->portName()).arg(m_serialPort->errorString()) << endl;
        QCoreApplication::exit(1);
    } else if (!m_serialPort->waitForBytesWritten(5000)) {
        m_standardOutput << QObject::tr("Operation timed out or an error "
                                      "occurred for port %1, error: %2")
                          .arg(m_serialPort->portName()).arg(m_serialPort->errorString()) << endl;
        QCoreApplication::exit(1);
    }

    m_standardOutput << QObject::tr("Data successfully sent to port %1")
                      .arg(m_serialPort->portName()) << endl;

    m_timer.start(5000);
}

void SerialPortWriter::writeTest()
{
    const char t_data[] = {0xa5,0x05,0x00,0x00,0x10,0x18,0xff,0xd7,'I'};
    const qint64 bytesWritten = m_serialPort->write(t_data, 9);

    if (bytesWritten == -1) {
        m_standardOutput << QObject::tr("Failed to write the data to port %1, error: %2")
                          .arg(m_serialPort->portName()).arg(m_serialPort->errorString()) << endl;
        QCoreApplication::exit(1);
    } else if (!m_serialPort->waitForBytesWritten(5000)) {
        m_standardOutput << QObject::tr("Operation timed out or an error "
                                      "occurred for port %1, error: %2")
                          .arg(m_serialPort->portName()).arg(m_serialPort->errorString()) << endl;
        QCoreApplication::exit(1);
    }

    m_standardOutput << QObject::tr("Data successfully sent to port %1")
                      .arg(m_serialPort->portName()) << endl;

    m_timer.start(5000);
}
