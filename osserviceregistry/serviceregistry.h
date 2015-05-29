#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include <QObject>

class ServiceRegistry : public QObject
{
    Q_OBJECT
public:
    explicit ServiceRegistry(QObject *parent = 0);

signals:

public slots:

};

#endif // SERVICEREGISTRY_H
