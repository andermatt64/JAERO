#ifndef SBS1_H
#define SBS1_H

#include <QObject>
#include "tcpserver.h"
#include "tcpclient.h"
#include "arincparse.h"

class SBS1 : public QObject
{
    Q_OBJECT
public:
    explicit SBS1(QObject *parent = 0);
    void starttcpconnections(const QList<QHostAddress> &addresses, const QList<quint16> &ports, bool behaveasclient);
    void stoptcpconnections();
signals:
    void SendBAViaTCP(QByteArray &ba);
public slots:
    void DownlinkBasicReportGroupSlot(DownlinkBasicReportGroup &message);
    void DownlinkEarthReferenceGroupSlot(DownlinkEarthReferenceGroup &message);
    void DownlinkGroupsSlot(DownlinkGroups &groups);
private:
    Tcpserver *tcpserver;
    QList<QPointer<Tcpclient>> tcpclients;
    bool running;
};

#endif // SBS1_H
