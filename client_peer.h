#pragma once

#include <memory>

#include <QObject>

#include "data_peer.h"
#include "msg/actions.h"
#include "comm/actions/action_callbacks.h"
#include "comm/client/client_coupling_handler_callbacks.h"

class CommunicationClient;

class ClientPeer : public DataPeer
{
    Q_OBJECT

public:
    explicit ClientPeer(quint16 dataPort, QObject* parent = nullptr);
    ~ClientPeer() override;

    bool connectToHost(const QString& host, quint16 port);
    void disconnectFromHost();

    void setActionsCallback(ActionHandlerCallback callback);
    bool sendAction(const msg::ActionRequest& msg, ActionResultCallback callback);

private: // DataPeer
    void onFailed() override { disconnectFromHost(); }

private:
    ActionHandlerCallback m_actionsCallback;
    ClientCouplingHandlerCallbacks m_callbacks;
    std::unique_ptr<CommunicationClient> m_commClient;
};
