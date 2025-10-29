#pragma once

#include <functional>

#include "msg/coupling.h"

struct ServerCouplingHandlerCallbacks
{
    std::function<int()> getTimeout;
    std::function<quint16()> getDataPort;
    std::function<int()> getHeartbeatInterval;
    std::function<bool(const msg::LoginRequest::Credentials&)> authorize;
};
