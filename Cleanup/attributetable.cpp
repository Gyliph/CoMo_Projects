#include "attributetable.h"

#include <QQmlEngine>
#include <QDebug>

using namespace std;

AttributeTable::AttributeTable(){
    /*qDebug() << "Creating table object";*/
}

AttributeTable::~AttributeTable(){
    /*qDebug() << "Destroying table object";*/
}

void AttributeTable::appendElement(QVariantMap el){
    data.append(el);
    emit dataAppended(el);
}

void AttributeTable::deleteElement(QVariantMap el){
    data.removeOne(el);
    emit dataRemoved(el);
}

void AttributeTable::clearList(){
    data.clear();
    emit listCleared();
}



