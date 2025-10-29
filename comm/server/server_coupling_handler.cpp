#include "server_coupling_handler.h"

#define SEND_RESPONSE(Value) \
if (!MessageHandler::send(Value)) { \
    emit failed(); \
    return; \
} \
if (!m_coupled) { \
    m_timer.start(); \
}

namespace
{
bool accepted(msg::CouplingResponse::Result type)
{
    return (msg::CouplingResponse::Accepted == type);
}
} // anonymous namespace

ServerCouplingHandler::ServerCouplingHandler(
    Sender& sender,
    ServerCouplingHandlerCallbacks& callbacks,
    QObject* parent
)
    : MessageHandler(sender, parent)
    , m_callbacks(callbacks)
{
    connect(&m_timer, &QTimer::timeout,
            this, &ServerCouplingHandler::failed);

    m_timer.setInterval(m_callbacks.getTimeout());
}

bool ServerCouplingHandler::processMessage(const msg::PeerMessage& msg)
{
    bool ok = false;
    HANDLE_MESSAGE(msg, ok, HandshakeRequest)
    HANDLE_MESSAGE(msg, ok, LoginRequest)
    HANDLE_MESSAGE(msg, ok, ConnectRequest)
    HANDLE_MESSAGE(msg, ok, HeartbeatEvent)
    return ok;
}

void ServerCouplingHandler::onHandshakeRequest(const msg::HandshakeRequest& msg)
{
    msg::HandshakeResponse response;
    response.result = msg::CouplingResponse::Rejected;

    if (m_handshaked || m_authorized || m_coupled)
    {
        response.result = msg::CouplingResponse::BadRequest;
    }
    else
    {
        response.result = msg::CouplingResponse::Accepted;
    }

    SEND_RESPONSE(response)

    if (!m_handshaked && accepted(response.result))
    {
        m_handshaked = true;
    }
}

void ServerCouplingHandler::onLoginRequest(const msg::LoginRequest& msg)
{
    msg::LoginResponse response;
    response.result = msg::CouplingResponse::Rejected;

    if (m_authorized || m_coupled)
    {
        response.result = msg::CouplingResponse::BadRequest;
    }
    else if (!m_handshaked)
    {
        response.result = msg::CouplingResponse::NoHandshake;
    }
    else
    {
        if (m_callbacks.authorize(msg.credentials))
        {
            response.result = msg::CouplingResponse::Accepted;
        }
    }

    SEND_RESPONSE(response)

    if (!m_authorized && accepted(response.result))
    {
        m_authorized = true;
    }
}

void ServerCouplingHandler::onConnectRequest(const msg::ConnectRequest& msg)
{
    msg::ConnectResponse response;
    response.result = msg::CouplingResponse::Rejected;

    if (m_coupled)
    {
        response.result = msg::CouplingResponse::BadRequest;
    }
    else if (!m_handshaked)
    {
        response.result = msg::CouplingResponse::NoHandshake;
    }
    else if (!m_authorized)
    {
        response.result = msg::CouplingResponse::NotAuthorized;
    }
    else
    {
        m_peerDataPort = msg.dataPort;

        const auto interval = m_callbacks.getHeartbeatInterval();
        m_timer.setInterval(interval);

        response.heartbeatInterval = interval;
        response.dataPort = m_callbacks.getDataPort();
        response.result = msg::CouplingResponse::Accepted;
    }

    SEND_RESPONSE(response)

    if (!m_coupled && accepted(response.result))
    {
        m_coupled = true;
        emit coupled();
    }
}

void ServerCouplingHandler::onHeartbeatEvent(const msg::HeartbeatEvent& msg)
{
    m_timer.start();
}
