#pragma once

#include "peer.h"

namespace msg
{
DECLARE_PEER_MESSAGE(CouplingMessage)
{
    enum Step
    {
        Unknown,
        Handshake,
        Login,
        Connect,
        Heartbeat
    };

    Step step() const { return m_step; }

    explicit CouplingMessage(Step step = CouplingMessage::Unknown)
        : m_step(step)
    {}

private:
    Step m_step;
    SERIALIZABLE_FIELDS(m_step)
};

//-------------------------------------------------------------------

template <typename CouplingMessage::Step step_type>
struct CouplingRequestOf : CouplingMessage
{
    explicit CouplingRequestOf()
        : CouplingMessage(step_type)
    {}
};

DECLARE_MESSAGE_OF(HandshakeRequest, CouplingRequestOf<CouplingMessage::Handshake>)
{};

DECLARE_MESSAGE_OF(LoginRequest, CouplingRequestOf<CouplingMessage::Login>)
{
    struct Credentials : lib::Serializable
    {
        QString name;
        QString pass;
        SERIALIZABLE_FIELDS(name, pass)
    };

    Credentials credentials;
    SERIALIZABLE_FIELDS(credentials)
};

DECLARE_MESSAGE_OF(ConnectRequest, CouplingRequestOf<CouplingMessage::Connect>)
{
    quint16 dataPort = 0;
    SERIALIZABLE_FIELDS(dataPort)
};

//-------------------------------------------------------------------

struct CouplingResponse : CouplingMessage
{
    enum Result
    {
        None,
        Accepted,
        Rejected,
        NoHandshake,
        NotAuthorized,
        BadRequest,
        Timeout
    };

    explicit CouplingResponse(CouplingMessage::Step step)
        : CouplingMessage(step)
    {}

    Result result;
    SERIALIZABLE_FIELDS(result)
};

template <typename CouplingMessage::Step step_type>
struct CouplingResponseOf : CouplingResponse
{
    explicit CouplingResponseOf()
        : CouplingResponse(step_type)
    {}
};

DECLARE_MESSAGE_OF(HandshakeResponse, CouplingResponseOf<CouplingMessage::Handshake>)
{};

DECLARE_MESSAGE_OF(LoginResponse, CouplingResponseOf<CouplingMessage::Login>)
{};

DECLARE_MESSAGE_OF(ConnectResponse, CouplingResponseOf<CouplingMessage::Connect>)
{
    quint16 dataPort = 0;
    int heartbeatInterval = 0;
    SERIALIZABLE_FIELDS(dataPort, heartbeatInterval)
};

//-------------------------------------------------------------------

DECLARE_MESSAGE_OF(HeartbeatEvent, CouplingMessage)
{
    explicit HeartbeatEvent()
        : ParentClassType(CouplingMessage::Heartbeat)
    {}
};
} // namespace msg
