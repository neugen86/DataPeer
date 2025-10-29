#pragma once

#include "peer.h"

namespace msg
{
DECLARE_PEER_MESSAGE(DataMessage)
{
    enum Type
    {
        Unknown,
        ClientState,
        ServerState
    };

    Type type() const { return m_type; }

    explicit DataMessage(Type type = DataMessage::Unknown)
        : m_type(type)
    {}

private:
    Type m_type;
    SERIALIZABLE_FIELDS(m_type);
};

template <typename DataMessage::Type data_type>
struct DataTypeMessage : DataMessage
{
    explicit DataTypeMessage()
        : DataMessage(data_type)
    {}
};

//-------------------------------------------------------------------

DECLARE_MESSAGE_OF(ClientStateMessage, DataTypeMessage<DataMessage::ClientState>)
{
    QByteArray data;
    SERIALIZABLE_FIELDS(data)
};

DECLARE_MESSAGE_OF(ServerStateMessage, DataTypeMessage<DataMessage::ServerState>)
{
    QByteArray data;
    SERIALIZABLE_FIELDS(data)
};
} // namespace msg
