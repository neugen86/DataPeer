#pragma once

#include <memory>
#include <vector>

#include <QDataStream>

namespace lib::impl
{
struct DataSerializer
{
    virtual ~DataSerializer() = default;
    virtual bool serialize(QDataStream& out) const = 0;
    virtual bool deserialize(QDataStream& in) = 0;
};

template <typename T>
struct RefSerializer : DataSerializer
{
    explicit RefSerializer(T& value)
        : m_ref(value)
    {}

    bool serialize(QDataStream& out) const
    {
        out << m_ref.get();
        return out.status() == QDataStream::Ok;
    }

    bool deserialize(QDataStream& in)
    {
        in >> m_ref.get();
        return in.status() == QDataStream::Ok;
    }

private:
    std::reference_wrapper<T> m_ref;
};

struct Serializer : DataSerializer
{
    void clear()
    {
        m_serializers.clear();
    }

    template <typename T>
    Serializer& add(T& ref)
    {
        using Type = RefSerializer<T>;
        m_serializers.emplace_back(std::make_unique<Type>(ref));
        return *this;
    }

    bool serialize(QDataStream& out) const
    {
        for (const auto& item : m_serializers)
        {
            if (!item->serialize(out))
                return false;
        }
        return true;
    }

    bool deserialize(QDataStream& in)
    {
        for (const auto& item : m_serializers)
        {
            if (!item->deserialize(in))
                return false;
        }
        return true;
    }

private:
    std::vector<std::unique_ptr<DataSerializer>> m_serializers;
};
} // namespace lib::impl
