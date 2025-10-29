#pragma once

#include "abstract_serializer.h"
#include "serialization_context_p.h"

namespace lib
{
struct SerializationContext : AbstractSerializer
{
    struct Initializer
    {
        friend class SerializationContext;

        template<typename... Args>
        void add(Args&&... args) const
        {
            addRecursive(std::forward<Args>(args)...);
        }

    private:
        Initializer(SerializationContext* ctx)
            : m_ctx(ctx)
        {}

        template<typename T>
        void addRecursive(T& ref) const
        {
            m_ctx->m_serializer->add(ref);
        }

        template<typename T, typename... Args>
        void addRecursive(T& first, Args&&... rest) const
        {
            m_ctx->m_serializer->add(first);
            addRecursive(std::forward<Args>(rest)...);
        }

    private:
        SerializationContext* m_ctx = nullptr;
    };

    explicit SerializationContext() = default;
    ~SerializationContext();

    SerializationContext(const SerializationContext& other);
    SerializationContext& operator=(const SerializationContext& other);

    SerializationContext(SerializationContext&& other);
    SerializationContext& operator=(SerializationContext&& other);

protected:
    void initBaseContext(const Initializer&) {}

private: // SerializaionContext
    virtual void initContextChain(const Initializer& initializer)
    {
        initBaseContext(initializer);
    };

private: // BaseSerializer
    bool serialize(QDataStream& out) const override;
    bool deserialize(QDataStream& in) override;

    friend QDataStream& operator<<(QDataStream &out, const SerializationContext& rhs);
    friend QDataStream& operator>>(QDataStream &in, SerializationContext& rhs);

private:
    mutable std::shared_ptr<impl::Serializer> m_serializer;
};
} // namespace namespace lib
