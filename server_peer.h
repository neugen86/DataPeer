#pragma once

#include <memory>

#include <QObject>

#include "data_peer.h"
#include "msg/actions.h"
#include "comm/actions/action_callbacks.h"
#include "comm/server/server_coupling_handler_callbacks.h"

class CommunicationServer;

class ServerPeer : public DataPeer
{
    Q_OBJECT

public:
    explicit ServerPeer(quint16 dataPort, QObject* parent = nullptr);
    ~ServerPeer() override;

    void setActionsCallback(ActionHandlerCallback callback);
    bool sendAction(const msg::ActionRequest& msg, ActionResultCallback callback);

    bool start(quint16 port);
    void stop();

    void dropClient();

private: // DataPeer
    void onFailed() override { stop(); }

private:
    ActionHandlerCallback m_actionsCallback;
    ServerCouplingHandlerCallbacks m_couplingCallbacks;
    std::unique_ptr<CommunicationServer> m_commServer;
};
