#pragma once

#include <functional>

#include "msg/coupling.h"

struct ClientCouplingHandlerCallbacks
{
    std::function<int()> getTimeout;
    std::function<quint16()> getDataPort;
    std::function<void(msg::LoginRequest::Credentials&)> getCredentials;
};
