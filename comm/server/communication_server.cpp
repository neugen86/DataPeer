#include "communication_server.h"

#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>

#include "comm/message_router.h"
#include "server_coupling_handler.h"
#include "comm/actions/action_handler.h"

CommunicationServer::CommunicationServer(ServerCouplingHandlerCallbacks couplingCallbacks,
                                         ActionHandlerCallback actionsCallback,
                                         QObject* parent)
    : QObject(parent)
    , m_couplingCallbacks(couplingCallbacks)
    , m_actionsCallback(actionsCallback)
{}

CommunicationServer::~CommunicationServer()
{
    stop();
}

bool CommunicationServer::sendAction(const msg::ActionRequest& msg,
                                     ActionResultCallback callback)
{
    if (!m_actionHandler)
    {
        callback(false);
        return false;
    }

    return m_actionHandler->sendAction(msg, std::move(callback));
}

bool CommunicationServer::start(quint16 port)
{
    if (m_server)
        return false;

    m_server = std::make_unique<QTcpServer>();

    m_server->setMaxPendingConnections(1);
    connect(m_server.get(), &QTcpServer::newConnection,
            this, &CommunicationServer::onNewConnection);

    qDebug() << "--- Comm_Server: opening port:" << port << "...";

    if (!m_server->listen(QHostAddress::LocalHost, port))
    {
        qDebug() << "--- Comm_Server: can't open port" << port;
        stop();
        return false;
    }

    qDebug() << "--- Comm_Server: port is opened";

    return true;
}

void CommunicationServer::stop()
{
    qDebug() << "--- Comm_Server: stopping ...";
    dropClient();
    m_server.reset();
    qDebug() << "--- Comm_Server: stopped";
}

void CommunicationServer::dropClient()
{
    m_actionHandler.reset();
    m_couplingHandler.reset();

    if (m_socket)
    {
        qDebug() << "--- Comm_Server: dropping client ...";

        m_socket->deleteLater();
        m_socket.clear();

        emit peerDisconnected();
    }
}

void CommunicationServer::onNewConnection()
{
    if (m_couplingHandler)
    {
        m_server->nextPendingConnection()->abort();
        return;
    }

    m_socket = m_server->nextPendingConnection();
    qDebug() << "--- Comm_Server: handling new connection ...";

    using std::placeholders::_1;
    auto callback = std::bind(&CommunicationServer::processMessage, this, _1);

    if (!m_transceiver.listen(m_socket, callback))
    {
        dropClient();
        return;
    }

    connect(m_socket, &QAbstractSocket::disconnected, this, [=]()
    {
        qDebug() << "--- Comm_Server: client disconnected";
        dropClient();
    }, Qt::QueuedConnection);

    qDebug() << "--- Comm_Server: connected with client"
             << m_socket->peerAddress()
             << "on port:" << m_socket->peerPort();

    startCoupling();
}

void CommunicationServer::startCoupling()
{
    Q_ASSERT(m_socket);
    Q_ASSERT(!m_couplingHandler);

    qDebug() << "--- Comm_Server: waiting for coupling ...";

    m_couplingHandler = std::make_unique<ServerCouplingHandler>(
        m_transceiver, m_couplingCallbacks);

    connect(m_couplingHandler.get(), &ServerCouplingHandler::failed,
            this, &CommunicationServer::dropClient, Qt::QueuedConnection);

    connect(m_couplingHandler.get(), &ServerCouplingHandler::coupled,
            this, &CommunicationServer::onCoupled, Qt::QueuedConnection);
}

void CommunicationServer::onCoupled()
{
    Q_ASSERT(m_couplingHandler);
    Q_ASSERT(!m_actionHandler);

    m_actionHandler = std::make_unique<ActionHandler>(
        m_transceiver, m_actionsCallback);

    qDebug() << "--- Comm_Server: coupled";

    emit peerConnected(m_socket->peerAddress().toString(),
                       m_couplingHandler->peerDataPort());
}

bool CommunicationServer::processMessage(const lib::Message& msg)
{
    ROUTE_MESSAGE(msg, CouplingMessage, m_couplingHandler)
    ROUTE_MESSAGE(msg, ActionMessage, m_actionHandler)

    Q_ASSERT(false);
    return false;
}
