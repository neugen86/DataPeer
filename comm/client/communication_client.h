#pragma once

#include <memory>

#include <QObject>

#include "msg/actions.h"
#include "lib/transceiver.h"
#include "comm/actions/action_callbacks.h"
#include "client_coupling_handler_callbacks.h"

class QTcpSocket;
class ActionHandler;
class ClientCouplingHandler;

class CommunicationClient : public QObject
{
    Q_OBJECT

public:
    explicit CommunicationClient(ClientCouplingHandlerCallbacks callbacks,
                                 ActionHandlerCallback actionsCallback,
                                 QObject* parent = nullptr);
    ~CommunicationClient() override;

    bool sendAction(const msg::ActionRequest& msg, ActionResultCallback callback);

    void connectToHost(const QString& host, quint16 port);
    void disconnectFromHost();

private:
    void startCoupling();
    void onCoupled();

    bool processMessage(const lib::Message& msg);

signals:
    void connected(quint16 peerDataPort, QPrivateSignal = {});
    void disconnected(QPrivateSignal = {});

private:
    lib::Transceiver m_transceiver;
    std::unique_ptr<QTcpSocket> m_socket;

    ActionHandlerCallback m_actionsCallback;
    std::unique_ptr<ActionHandler> m_actionHandler;

    ClientCouplingHandlerCallbacks m_couplingCallbacks;
    std::unique_ptr<ClientCouplingHandler> m_couplingHandler;
};
