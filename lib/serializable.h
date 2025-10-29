#pragma once

#include "serialization_context.h"

namespace lib
{
template <typename T>
struct SerializableHeir : T
{
    using Initializer = SerializationContext::Initializer;

    template<typename... Args>
    explicit SerializableHeir(Args&&... args)
        : T(std::forward<Args>(args)...)
    {}

protected:
    explicit SerializableHeir() = default;

    void initBaseContext(const Initializer& initializer)
    {
        T::initBaseContext(initializer);
    }

private:
    void initContextChain(const Initializer& initializer) override
    {
        SerializableHeir::initBaseContext(initializer);
    }
};

using Serializable = SerializableHeir<SerializationContext>;

#define SERIALIZABLE_FIELDS(...) \
private: \
void initContextChain(const lib::SerializationContext::Initializer& initializer) { \
    initBaseContext(initializer); \
} \
public: \
void initBaseContext(const lib::SerializationContext::Initializer& initializer) { \
    SerializableHeir::initBaseContext(initializer); \
    initializer.add(__VA_ARGS__); \
}
} // namespace lib
