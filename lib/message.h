#pragma once

#include <memory>
#include <typeinfo>

#include <QSet>
#include <QByteArray>

#include "serializable.h"

namespace lib
{
struct MessageBounds
{
    qint64 pos = -1;
    qint64 size = -1;

    qint64 endPos() const
    {
        return valid() ? (pos + size) : -1;
    }

    bool valid() const
    {
        return !(pos < 0 || size < 0);
    }
};

struct Sentinel
{
    using Type = int;
    static const int Size = sizeof(Type);
    static const Type header_start  = 0xbeafbeaf;
    static const Type header_end    = 0xcafecafe;
    static const Type message_start = header_start;
    static const Type message_end   = 0xdeaddead;
};

struct Message : protected Serializable
{
    explicit Message(const QByteArray& data = {});

    QByteArray toBytes(bool& ok) const;
    bool fromBytes(QByteArray data);

    static MessageBounds getBounds(
        Sentinel::Type sentinelStart, Sentinel::Type sentinelEnd,
        const QByteArray& data, qint64 from = 0);

    template <typename T>
    bool is() const
    {
        (void)static_cast<const T&>(*this);
        bool ok = dynamic_cast<const T*>(this);
        ok = ok || m_typeIds.contains(T::MessageId);
        return ok;
    }

    template<typename T>
    T& as(bool* success = nullptr)
    {
        if (dynamic_cast<T*>(this))
        {
            if (success) *success = true;
            return static_cast<T&>(*this);
        }
        return fromData<T>(success);
    }

    template<typename T>
    const T& as(bool* success = nullptr) const
    {
        if (dynamic_cast<const T*>(this))
        {
            if (success) *success = true;
            return static_cast<const T&>(*this);
        }
        return fromData<T>(success);
    }

protected:
    template <typename T>
    static QString getId()
    {
        return typeid(T).name();
    }

    void addId(QSet<QString>&) const {}

private:
    template<typename T>
    T& fromData(bool* success = nullptr) const
    {
        bool ok = false;
        auto ptr = std::make_shared<T>();
        m_tmpList.push_back(ptr);

        if (is<T>())
        {
            ok = ptr->loadContent(m_content);
            if (ok) { ptr->m_typeIds = m_typeIds; }
        }

        if (success) *success = ok;
        return *ptr;
    }

    virtual void collectIds(QSet<QString>& result) const
    {
        addId(result);
    }

    bool setHeader(QByteArray& content) const;
    bool loadContent(const QByteArray& content);

private:
    QByteArray m_content;
    mutable QSet<QString> m_typeIds;
    mutable std::vector<std::shared_ptr<Message>> m_tmpList;
};

// ----------------------------------------------------------------------------

template <typename ClassName, typename Base>
struct MessageOf : SerializableHeir<Base>
{
    static const QString MessageId;

    using ParentClassType = MessageOf<ClassName, Base>;

    template<typename... Args>
    explicit MessageOf(Args&&... args)
        : SerializableHeir<Base>(std::forward<Args>(args)...)
    {}

protected:
    explicit MessageOf() = default;

    void addId(QSet<QString>& result) const
    {
        Base::addId(result);
        result.insert(MessageId);
    }

private:
    void collectIds(QSet<QString>& result) const override
    {
        MessageOf::addId(result);
    }
};

template <typename ClassName>
using RootMessage = MessageOf<ClassName, Message>;

template <typename ClassName, typename Base>
const QString MessageOf<ClassName, Base>::MessageId = Message::getId<ClassName>();

#define DECLARE_ROOT_MESSAGE(ClassName) \
struct ClassName : lib::RootMessage<ClassName>

#define DECLARE_MESSAGE_OF(ClassName, Base) \
struct ClassName : lib::MessageOf<ClassName, Base>

} // namespace lib
