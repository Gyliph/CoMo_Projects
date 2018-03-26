#ifndef ATTRIBUTETABLE_H
#define ATTRIBUTETABLE_H

#include <QObject>
#include <QList>
#include <QVariantMap>

class AttributeTable: public QObject
{
    Q_OBJECT

public:
    AttributeTable();
    ~AttributeTable();

    void appendElement(QVariantMap el);
    void deleteElement(QVariantMap el);
    void clearList();

signals:
    void dataAppended(QVariantMap row);
    void dataRemoved(QVariantMap row);
    void listCleared();

private:
    QList<QVariantMap> data;

};

#endif // ATTRIBUTETABLE_H
