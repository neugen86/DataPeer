#include "client_peer.h"

#include <QDebug>

#include "comm/client/communication_client.h"

ClientPeer::ClientPeer(quint16 dataPort, QObject* parent)
    : DataPeer(dataPort)
{
    m_callbacks.getTimeout = []{ return 1000; };
    m_callbacks.getDataPort = [=](){ return dataPort; };

    m_callbacks.getCredentials = [=](auto& credentials)
    {
        credentials.name = "name";
        credentials.pass = "pass";
    };
}

ClientPeer::~ClientPeer()
{
    disconnectFromHost();
}

void ClientPeer::setActionsCallback(ActionHandlerCallback callback)
{
    m_actionsCallback = std::move(callback);
}

bool ClientPeer::sendAction(const msg::ActionRequest& msg, ActionResultCallback callback)
{
    return m_commClient && m_commClient->sendAction(msg, std::move(callback));
}

bool ClientPeer::connectToHost(const QString& host, quint16 port)
{
    qDebug() << "- ClientPeer: connecting to" << host << "on port" << port << "...";

    disconnectFromHost();

    if (!createDataChannel())
    {
        qDebug() << "- ClientPeer: can't create data channel";
        return false;
    }

    m_commClient = std::make_unique<CommunicationClient>(
        m_callbacks, m_actionsCallback);

    connect(m_commClient.get(), &CommunicationClient::disconnected,
            this, &ClientPeer::disconnectFromHost);

    connect(m_commClient.get(), &CommunicationClient::connected,
            this, [=](quint16 peerDataPort)
    {
        qDebug() << "- ClientPeer: establishing data connection with host:"
                 << host << " on port" << peerDataPort << "...";

        if (!connectDataPeer(host, peerDataPort))
        {
            qDebug() << "- ClientPeer: can't establish data connection";
            disconnectFromHost();
            return;
        }
        qDebug() << "- ClientPeer: data connection established";
    });

    m_commClient->connectToHost(host, port);

    return true;
}

void ClientPeer::disconnectFromHost()
{
    if (!m_commClient)
        return;

    qDebug() << "- ClientPeer: disconnecting ...";

    disconnectDataPeer();
    m_commClient.reset();

    qDebug() << "- ClientPeer: disconnected";
}
