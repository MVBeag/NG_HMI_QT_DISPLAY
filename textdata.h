#ifndef TEXTDATA_H
#define TEXTDATA_H

#include <QObject>
#include <QVariant>
#include <QDebug>
#include <QString>

class TextData : public QObject
{
    Q_OBJECT
public:
    explicit TextData(QObject *text, QObject *parent = nullptr);

signals:
    void testSignal(QString m_testSignal);
public slots:
    void updateValue(QString m_updateValue);
private:
    QObject *m_text;
    float m_val{0.0f};
    QString m_ttext;
    bool m_increase{true};
};


inline void TextData::updateValue(QString newValue){
    m_text->setProperty("text", newValue);
}


#endif // TEXTDATA_H
