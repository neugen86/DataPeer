#include "action_handler.h"

ActionHandler::ActionHandler(Sender& sender,
                             ActionHandlerCallback callback,
                             QObject* parent)
    : MessageHandler(sender, parent)
    , m_callback(std::move(callback))
{}

bool ActionHandler::sendAction(const msg::ActionRequest& msg,
                               ActionResultCallback callback)
{
    m_pendingActions.emplace(msg.requestId, std::move(callback));
    return send(msg);
}

bool ActionHandler::processMessage(const msg::PeerMessage& msg)
{
    bool ok = false;
    HANDLE_MESSAGE(msg, ok, ActionRequest)
    HANDLE_MESSAGE(msg, ok, ActionResponse)
    return ok;
}

bool ActionHandler::onActionRequest(const msg::ActionRequest& request)
{
    msg::ActionResponse response(request);
    response.result = msg::ActionResponse::Rejected;

    const bool valid = (response.type() != msg::ActionResponse::Unknown);

    if (valid && m_callback(request))
    {
        response.result = msg::ActionResponse::Accepted;
    }

    return send(response);
}

bool ActionHandler::onActionResponse(const msg::ActionResponse& response)
{
    auto it = m_pendingActions.find(response.requestId);

    if (it != m_pendingActions.cend())
    {
        it->second(msg::ActionResponse::Accepted == response.result);
        m_pendingActions.erase(it);
    }

    return true;
}
