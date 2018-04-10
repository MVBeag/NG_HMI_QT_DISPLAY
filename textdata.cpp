#include "textdata.h"
#include "serialportreader.h"

/*on charge dans m_text la valeur de text qui est un pointeur sur l'objet QML*/
TextData::TextData(QObject *text, QObject *parent)
    : QObject(parent)
    , m_text(text){}
