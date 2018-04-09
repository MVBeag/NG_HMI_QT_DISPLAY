#ifndef NYDUMMY_H
#define NYDUMMY_H

#include <QObject>

class nydummy : public QObject
{
    Q_OBJECT
public:
    explicit nydummy(QObject *parent = nullptr);

signals:

public slots:
};

#endif // NYDUMMY_H