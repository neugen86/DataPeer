#include "message.h"

#include <QBuffer>
#include <QDataStream>
#include <QStringList>

#include "abstract_serializer.h"

namespace
{
const char TypesDelimeter = '|';

struct Header : lib::Serializable
{
    int contentSize = 0;
    QString typeInfo = "nothing";
    int sentinelStart = lib::Sentinel::header_start;
    int sentinelEnd = lib::Sentinel::header_end;

    bool check() const
    {
        return lib::Sentinel::header_start == sentinelStart &&
               lib::Sentinel::header_end == sentinelEnd;

    }

    SERIALIZABLE_FIELDS( // order is important
        sentinelStart,
        contentSize,
        typeInfo,
        sentinelEnd
    )
};

QSet<QString> fromList(const QStringList& list)
{
    return QSet<QString>(list.constBegin(), list.constEnd());
}

bool addSentinel(lib::Sentinel::Type value, QByteArray& out)
{
    QBuffer buffer(&out);
    QDataStream stream(&buffer);
    lib::AbstractSerializer::setupDataStream(stream);

    if (!buffer.open(QIODevice::WriteOnly))
        return false;

    buffer.seek(buffer.bytesAvailable());
    stream << value;
    return true;
}

bool checkSentinel(const lib::Sentinel::Type value, const QByteArray& in)
{
    QDataStream stream(in);
    lib::AbstractSerializer::setupDataStream(stream);

    lib::Sentinel::Type tmp = 0;
    stream >> tmp;
    return (value == tmp);
}

bool findHeader(const QByteArray& data, Header& header, qint64* size = nullptr)
{
    const auto bounds = lib::Message::getBounds(
        lib::Sentinel::header_start,
        lib::Sentinel::header_end,
        data);

    if (!bounds.valid())
        return false;

    const auto tmp = data.right(data.size() - bounds.pos);
    const bool ok = header.fromBytes(data) && header.check();

    if (ok && size)
    {
        *size = bounds.size;
    }

    return ok;
}
} // anonymous namespace

namespace lib
{
Message::Message(const QByteArray& data)
{
    fromBytes(data);
}

QByteArray Message::toBytes(bool& ok) const
{
    if (m_typeIds.isEmpty())
    {
        collectIds(m_typeIds);
    }

    auto result = SerializationContext::toBytes(ok);

    if (ok && setHeader(result))
    {
        addSentinel(Sentinel::message_end, result);
        return result;
    }
    return {};
}

bool Message::fromBytes(QByteArray data)
{
    m_content.clear();

    Header header;
    qint64 headerSize = 0;
    if (!findHeader(data, header, &headerSize))
        return false;

    data = data.right(data.size() - headerSize);
    const auto rest = data.mid(header.contentSize);

    if (!checkSentinel(Sentinel::message_end, rest))
    {
        m_content.clear();
        return false;
    }

    const auto list = header.typeInfo.split(TypesDelimeter);
    m_typeIds = fromList(list);

    return loadContent(data);
}

MessageBounds Message::getBounds(
    Sentinel::Type sentinelFrom, Sentinel::Type sentinelTo,
    const QByteArray& data, qint64 from)
{
    MessageBounds result;

    auto getSentinelPos =
        [](const Sentinel::Type value, const QByteArray& data, qint64 from)
    {
        if (data.isEmpty())
            return -1;

        QByteArray tmp;
        addSentinel(value, tmp);

        return data.indexOf(tmp, from);
    };

    const qint64 startPos = getSentinelPos(sentinelFrom, data, from);
    if (startPos < 0) return result;

    const qint64 endPos = getSentinelPos(sentinelTo, data, startPos + Sentinel::Size);
    if (endPos < 0) return result;

    result.pos = startPos;
    result.size = endPos + Sentinel::Size - startPos;

    return result;
}

bool Message::setHeader(QByteArray& content) const
{
    Header header;
    header.contentSize = content.size();
    header.typeInfo = QStringList(
        m_typeIds.constBegin(), m_typeIds.constEnd()).join(TypesDelimeter);

    bool ok = false;
    const auto headerData = header.toBytes(ok);
    ok = ok && header.check();

    if (ok) content.prepend(headerData);

    return ok;
}

bool Message::loadContent(const QByteArray& content)
{
    m_content = content;
    return SerializationContext::fromBytes(content);
}
} // namespace lib
