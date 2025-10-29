#pragma once

namespace lib
{
class Message;

struct MessageSender
{
    virtual ~MessageSender() = default;
    virtual bool send(const lib::Message& msg) const = 0;
};
} // namespace lib
