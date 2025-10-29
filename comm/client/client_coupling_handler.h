#pragma once

#include <QTimer>

#include "msg/coupling.h"
#include "comm/message_handler.h"
#include "client_coupling_handler_callbacks.h"

class ClientCouplingHandler : public MessageHandler
{
    Q_OBJECT

public:
    explicit ClientCouplingHandler(
        Sender& sender,
        ClientCouplingHandlerCallbacks& callbacks,
        QObject* parent = nullptr);

    void startCoupling();

    quint16 peerDataPort() const { return m_peerDataPort; }

public: // MessageHandler
    bool processMessage(const msg::PeerMessage& msg) override;

private:
    void onHandshakeResponse(const msg::HandshakeResponse& msg);
    void onLoginResponse(const msg::LoginResponse& msg);
    void onConnectResponse(const msg::ConnectResponse& msg);
    void sendHeartbeatEvent();

signals:
    void coupled();
    void failed(msg::CouplingResponse::Result res);

private:
    bool m_coupled = false;
    quint16 m_peerDataPort = 0;

    QTimer m_timer;
    ClientCouplingHandlerCallbacks& m_callbacks;
};

Q_DECLARE_METATYPE(msg::CouplingResponse::Result)
