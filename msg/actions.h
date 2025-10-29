#pragma once

#include "peer.h"

namespace msg
{
static qint64 NextRequestId = 0;

DECLARE_PEER_MESSAGE(ActionMessage)
{
    enum Type
    {
        Unknown,
        Foo,
        Bar
    };

    Type type() const { return m_type; }

    explicit ActionMessage(Type type = ActionMessage::Unknown)
        : m_type(type)
    {}

private:
    Type m_type;
    SERIALIZABLE_FIELDS(m_type)
};

//-------------------------------------------------------------------

DECLARE_MESSAGE_OF(ActionRequest, ActionMessage)
{
    explicit ActionRequest(ActionMessage::Type type = ActionMessage::Unknown)
        : ParentClassType(type) {}

    qint64 requestId = -1;
    SERIALIZABLE_FIELDS(requestId)
};

DECLARE_MESSAGE_OF(ActionResponse, ActionMessage)
{
    enum Result
    {
        Accepted,
        Rejected
    };

    explicit ActionResponse()
        : ParentClassType(ActionMessage::Unknown)
    {}

    explicit ActionResponse(const ActionRequest& request)
        : ParentClassType(request.type())
        , requestId(request.requestId)
    {}

    qint64 requestId = -1;
    Result result = ActionResponse::Rejected;
    SERIALIZABLE_FIELDS(requestId, result)
};

//-------------------------------------------------------------------

template <ActionMessage::Type action_type>
struct ActionOf : ActionRequest
{
    explicit ActionOf()
        : ActionRequest(action_type)
    {
        requestId = ++NextRequestId;
    }
};

DECLARE_MESSAGE_OF(FooAction, ActionOf<ActionMessage::Foo>)
{
    bool foo;
    SERIALIZABLE_FIELDS(foo)
};

DECLARE_MESSAGE_OF(BarAction, ActionOf<ActionMessage::Bar>)
{
    bool bar;
    SERIALIZABLE_FIELDS(bar)
};
} // namespace msg
