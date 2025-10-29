#include "communication_client.h"

#include <QDebug>
#include <QTcpSocket>
#include <QHostAddress>

#include "comm/message_router.h"
#include "client_coupling_handler.h"
#include "comm/actions/action_handler.h"

CommunicationClient::CommunicationClient(ClientCouplingHandlerCallbacks callbacks,
                                         ActionHandlerCallback actionsCallback,
                                         QObject* parent)
    : QObject(parent)
    , m_actionsCallback(actionsCallback)
    , m_couplingCallbacks(std::move(callbacks))
{}

CommunicationClient::~CommunicationClient()
{
    disconnectFromHost();
}

bool CommunicationClient::sendAction(const msg::ActionRequest& msg,
                                     ActionResultCallback callback)
{
    if (!m_actionHandler)
    {
        callback(false);
        return false;
    }

    return m_actionHandler->sendAction(msg, std::move(callback));
}

void CommunicationClient::connectToHost(const QString& host, quint16 port)
{
    qDebug() << "--- Comm_Client: connecting ...";

    disconnectFromHost();
    m_socket = std::make_unique<QTcpSocket>();

    using std::placeholders::_1;
    auto callback = std::bind(&CommunicationClient::processMessage, this, _1);

    if (!m_transceiver.listen(m_socket.get(), callback))
    {
        disconnectFromHost();
        return;
    }

    connect(m_socket.get(), &QAbstractSocket::disconnected, this, [=]()
    {
        qDebug() << "--- Comm_Client: disconnected";
        disconnectFromHost();
    }, Qt::QueuedConnection);

    connect(m_socket.get(), &QAbstractSocket::connected, this, [=]()
    {
        qDebug() << "--- Comm_Client: connected to"
                 << m_socket->peerAddress()
                 << "on port:" << m_socket->peerPort();
        startCoupling();
    }, Qt::QueuedConnection);

    m_socket->connectToHost(host, port);
}

void CommunicationClient::disconnectFromHost()
{
    m_actionHandler.reset();
    m_couplingHandler.reset();

    if (m_socket)
    {
        qDebug() << "--- Comm_Client: disconnecting ...";
        m_socket.reset();
        emit disconnected();
    }
}

void CommunicationClient::startCoupling()
{
    Q_ASSERT(m_socket);
    Q_ASSERT(!m_couplingHandler);

    qDebug() << "--- Comm_Client: start coupling ...";

    m_couplingHandler = std::make_unique<ClientCouplingHandler>(
        m_transceiver, m_couplingCallbacks);

    connect(m_couplingHandler.get(), &ClientCouplingHandler::failed,
            this, [=](int res)
    {
        qDebug() << "--- Comm_Client: failed with result:" << res;
        m_socket->abort();
    });

    connect(m_couplingHandler.get(), &ClientCouplingHandler::coupled,
            this, &CommunicationClient::onCoupled, Qt::QueuedConnection);

    m_couplingHandler->startCoupling();
}

void CommunicationClient::onCoupled()
{
    Q_ASSERT(m_couplingHandler);
    Q_ASSERT(!m_actionHandler);

    m_actionHandler = std::make_unique<ActionHandler>(
        m_transceiver, m_actionsCallback);

    qDebug() << "--- Comm_Client: coupled";

    emit connected(m_couplingHandler->peerDataPort());
}

bool CommunicationClient::processMessage(const lib::Message& msg)
{
    ROUTE_MESSAGE(msg, CouplingMessage, m_couplingHandler)
    ROUTE_MESSAGE(msg, ActionMessage, m_actionHandler)

    Q_ASSERT(false);
    return false;
}
