#include "client_coupling_handler.h"

#define CHECK_RESPONSE(Value) \
if (Value.result != msg::CouplingResponse::Accepted) \
{ \
    emit failed(Value.result); \
    return; \
}

#define SEND_REQUEST(Value) \
if (!MessageHandler::send(Value)) { \
    emit failed(msg::CouplingResponse::None); \
    return; \
} \
m_timer.start();

ClientCouplingHandler::ClientCouplingHandler(
    Sender& sender,
    ClientCouplingHandlerCallbacks& callbacks,
    QObject* parent
)
    : MessageHandler(sender, parent)
    , m_callbacks(callbacks)
{
    connect(&m_timer, &QTimer::timeout, this, [=]()
    {
        if (m_coupled)
        {
            sendHeartbeatEvent();
            return;
        }
        emit failed(msg::CouplingResponse::Timeout);
    });
    m_timer.setInterval(m_callbacks.getTimeout());
}

bool ClientCouplingHandler::processMessage(const msg::PeerMessage& msg)
{
    bool ok = false;
    HANDLE_MESSAGE(msg, ok, HandshakeResponse)
    HANDLE_MESSAGE(msg, ok, LoginResponse)
    HANDLE_MESSAGE(msg, ok, ConnectResponse)
    return ok;
}

void ClientCouplingHandler::startCoupling()
{
    msg::HandshakeRequest request;
    SEND_REQUEST(request);
}

void ClientCouplingHandler::onHandshakeResponse(const msg::HandshakeResponse& msg)
{
    if (msg.result != msg::CouplingResponse::Accepted)
    {
        emit failed(msg.result);
        return;
    }

    msg::LoginRequest request;
    m_callbacks.getCredentials(request.credentials);

    SEND_REQUEST(request)
}

void ClientCouplingHandler::onLoginResponse(const msg::LoginResponse& msg)
{
    CHECK_RESPONSE(msg)

    msg::ConnectRequest request;
    request.dataPort = m_callbacks.getDataPort();

    SEND_REQUEST(request)
}

void ClientCouplingHandler::onConnectResponse(const msg::ConnectResponse& msg)
{
    CHECK_RESPONSE(msg)

    m_peerDataPort = msg.dataPort;

    m_timer.setInterval(msg.heartbeatInterval/2);
    m_timer.start();

    sendHeartbeatEvent();

    m_coupled = true;
    emit coupled();
}

void ClientCouplingHandler::sendHeartbeatEvent()
{
    msg::HeartbeatEvent e;
    SEND_REQUEST(e);
}
