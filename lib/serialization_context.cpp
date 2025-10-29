#include "serialization_context.h"

namespace lib
{
SerializationContext::~SerializationContext()
{
    m_serializer.reset();
}

SerializationContext::SerializationContext(const SerializationContext& other)
    : m_serializer(nullptr)
{}

SerializationContext& SerializationContext::operator=(const SerializationContext& other)
{
    if (this != &other)
    {
        m_serializer.reset();
    }
    return *this;
}

SerializationContext::SerializationContext(SerializationContext&& other)
    : m_serializer(nullptr)
{
    other.m_serializer.reset();
}

SerializationContext& SerializationContext::operator=(SerializationContext&& other)
{
    if (this != &other)
    {
        other.m_serializer.reset();
        m_serializer.reset();
    }
    return *this;
}

bool SerializationContext::serialize(QDataStream& out) const
{
    if (!m_serializer)
    {
        m_serializer = std::make_shared<impl::Serializer>();
        auto mutableThis = const_cast<SerializationContext*>(this);
        mutableThis->initContextChain(mutableThis);
    }
    return m_serializer->serialize(out);
}

bool SerializationContext::deserialize(QDataStream& in)
{
    if (!m_serializer)
    {
        m_serializer = std::make_shared<impl::Serializer>();
        initContextChain(this);
    }
    return m_serializer->deserialize(in);
}

QDataStream& operator<<(QDataStream &out, const SerializationContext& ctx)
{
    ctx.serialize(out);
    return out;
}

QDataStream& operator>>(QDataStream &in, SerializationContext& ctx)
{
    ctx.deserialize(in);
    return in;
}
} // namespace lib
