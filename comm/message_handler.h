#pragma once

#include <QObject>

#include "msg/peer.h"
#include "lib/message_sender.h"

class MessageHandler : public QObject
{
    Q_OBJECT

public:
    using Sender = lib::MessageSender;

    explicit MessageHandler(Sender& sender, QObject* parent = nullptr)
        : QObject(parent)
        , m_sender(sender)
    {}

    virtual bool processMessage(const msg::PeerMessage& msg) = 0;

protected:
    bool send(const msg::PeerMessage& msg)
    {
        return m_sender.send(msg);
    }

private:
    Sender& m_sender;
};

#define HANDLE_MESSAGE(Value, Ok, MessageName) \
if (Value.is<msg::MessageName>()) { \
    on##MessageName(Value.as<msg::MessageName>(&Ok)); \
}
