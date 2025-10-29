#include "server_peer.h"

#include <QDebug>
#include <QUuid>

#include "comm/server/communication_server.h"

ServerPeer::ServerPeer(quint16 dataPort, QObject* parent)
    : DataPeer(dataPort)
{
    m_couplingCallbacks.getTimeout = [](){ return 1000; };
    m_couplingCallbacks.getDataPort = [=](){ return dataPort; };
    m_couplingCallbacks.getHeartbeatInterval = [](){ return 2000; };

    m_couplingCallbacks.authorize = [](const msg::LoginRequest::Credentials& credentials)
    {
        return (credentials.name == "name" && credentials.pass == "pass");
    };
}

ServerPeer::~ServerPeer()
{
    stop();
}

void ServerPeer::setActionsCallback(ActionHandlerCallback callback)
{
    m_actionsCallback = std::move(callback);
}

bool ServerPeer::sendAction(const msg::ActionRequest& msg, ActionResultCallback callback)
{
    return m_commServer && m_commServer->sendAction(msg, std::move(callback));
}

bool ServerPeer::start(quint16 port)
{
    qDebug() << "- ServerPeer: opening port" << port << "...";

    stop();

    if (!createDataChannel())
    {
        qDebug() << "- ServerPeer: can't create DataChannel";
        return false;
    }

    m_commServer = std::make_unique<CommunicationServer>(
        m_couplingCallbacks, m_actionsCallback);

    connect(m_commServer.get(), &CommunicationServer::peerDisconnected,
            this, [=](){ disconnectDataPeer(); });

    connect(m_commServer.get(), &CommunicationServer::peerConnected,
            this, [=](const QString& peerAddress, quint16 peerDataPort)
    {
        qDebug() << "- ServerPeer: establishing data connection with host:"
                 << peerAddress << " on port" << peerDataPort << "...";

        if (!connectDataPeer(peerAddress, peerDataPort))
        {
            qDebug() << "- ServerPeer: can't establish data connection";
            m_commServer->dropClient();
            return;
        }
        qDebug() << "- ServerPeer: data connection established";
    });

    if (!m_commServer->start(port))
    {
        qDebug() << "- ServerPeer: can't open port" << port;
        m_commServer.reset();
        return false;
    }

    qDebug() << "- ServerPeer: port is opened";

    return true;
}

void ServerPeer::stop()
{
    if (!m_commServer)
        return;

    qDebug() << "- ServerPeer: stopping ...";

    disconnectDataPeer();
    m_commServer.reset();

    qDebug() << "- ServerPeer: stopped";
}

void ServerPeer::dropClient()
{
    if (!m_commServer)
        return;

    disconnectDataPeer();
    m_commServer->dropClient();
}
