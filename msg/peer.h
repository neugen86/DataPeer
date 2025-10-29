#pragma once

#include "lib/message.h"

namespace msg
{
DECLARE_ROOT_MESSAGE(PeerMessage)
{
    struct Header : lib::Serializable
    {
        using IdType = qint64;

        IdType id;
        qint64 timestamp = 0;
        SERIALIZABLE_FIELDS(id, timestamp)
    };

    Header header;
    SERIALIZABLE_FIELDS(header);
};

#define DECLARE_PEER_MESSAGE(ClassName) \
DECLARE_MESSAGE_OF(ClassName, PeerMessage)

} // namespace msg
