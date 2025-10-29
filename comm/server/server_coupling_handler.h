#pragma once

#include <QTimer>

#include "msg/coupling.h"
#include "comm/message_handler.h"
#include "server_coupling_handler_callbacks.h"

class ServerCouplingHandler : public MessageHandler
{
    Q_OBJECT

public:
    explicit ServerCouplingHandler(
        Sender& sender,
        ServerCouplingHandlerCallbacks& callbacks,
        QObject* parent = nullptr);

    quint16 peerDataPort() const { return m_peerDataPort; }

public: // MessageHandler
    bool processMessage(const msg::PeerMessage& msg) override;

private:
    void onHandshakeRequest(const msg::HandshakeRequest& msg);
    void onLoginRequest(const msg::LoginRequest& msg);
    void onConnectRequest(const msg::ConnectRequest& msg);
    void onHeartbeatEvent(const msg::HeartbeatEvent& msg);

signals:
    void coupled();
    void failed();

private:
    bool m_handshaked = false;
    bool m_authorized = false;
    bool m_coupled = false;
    quint16 m_peerDataPort = 0;

    QTimer m_timer;
    ServerCouplingHandlerCallbacks& m_callbacks;
};
