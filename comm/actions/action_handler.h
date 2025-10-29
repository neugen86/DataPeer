#pragma once

#include "action_callbacks.h"
#include "comm/message_handler.h"

class ActionHandler : public MessageHandler
{
    Q_OBJECT

public:
    explicit ActionHandler(Sender& sender,
                           ActionHandlerCallback callback,
                           QObject* parent = nullptr);

    bool sendAction(const msg::ActionRequest& msg,
                    ActionResultCallback callback);

public: // MessageHandler
    bool processMessage(const msg::PeerMessage& msg) override;

private:
    bool onActionRequest(const msg::ActionRequest& request);
    bool onActionResponse(const msg::ActionResponse& response);

private:
    ActionHandlerCallback m_callback;

    using Id = msg::PeerMessage::Header::IdType;
    std::map<Id, ActionResultCallback> m_pendingActions;
};
