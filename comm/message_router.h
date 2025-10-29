#pragma once

#define ROUTE_MESSAGE(Value, MessageName, Handler) \
if (Value.is<msg::MessageName>()) \
{ \
    bool ok = false; \
    Handler->processMessage(Value.as<msg::MessageName>(&ok)); \
    Q_ASSERT(ok); \
    return ok; \
}
