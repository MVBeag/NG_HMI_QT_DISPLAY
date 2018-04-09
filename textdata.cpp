#include "textdata.h"
#include "serialportreader.h"

TextData::TextData(QObject *text, QObject *parent)
    : QObject(parent)
    , m_text(text){
    emit testSignal("testSignal");
}
