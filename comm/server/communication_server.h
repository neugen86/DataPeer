#pragma once

#include <memory>

#include <QObject>
#include <QPointer>

#include "lib/transceiver.h"
#include "comm/actions/action_handler.h"
#include "comm/actions/action_callbacks.h"
#include "server_coupling_handler_callbacks.h"

class QTcpServer;
class QTcpSocket;
class ActionHandler;
class ServerCouplingHandler;

class CommunicationServer : public QObject
{
    Q_OBJECT

public:
    explicit CommunicationServer(ServerCouplingHandlerCallbacks couplingCallbacks,
                                 ActionHandlerCallback actionsCallback,
                                 QObject* parent = nullptr);
    ~CommunicationServer() override;

    bool sendAction(const msg::ActionRequest& msg, ActionResultCallback callback);

    bool start(quint16 port);
    void stop();

    void dropClient();

private:
    void onNewConnection();

    void startCoupling();
    void onCoupled();

    bool processMessage(const lib::Message& msg);

signals:
    void peerConnected(const QString& peerAddress,
                       int peerDataPort,
                       QPrivateSignal = {});
    void peerDisconnected(QPrivateSignal = {});

private:
    lib::Transceiver m_transceiver;

    QPointer<QTcpSocket> m_socket;
    std::unique_ptr<QTcpServer> m_server;

    ActionHandlerCallback m_actionsCallback;
    std::unique_ptr<ActionHandler> m_actionHandler;

    ServerCouplingHandlerCallbacks m_couplingCallbacks;
    std::unique_ptr<ServerCouplingHandler> m_couplingHandler;
};
