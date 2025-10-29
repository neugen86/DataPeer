#include "abstract_serializer.h"

#include <QBuffer>

namespace lib
{
QByteArray AbstractSerializer::toBytes(bool& ok) const
{
    QByteArray data;
    QBuffer buffer(&data);

    if ((ok = buffer.open(QIODevice::WriteOnly)))
    {
        QDataStream stream(&buffer);
        setupDataStream(stream);
        ok = serialize(stream);
    }

    return ok ?  data : QByteArray();
}

bool AbstractSerializer::fromBytes(const QByteArray& data)
{
    QDataStream stream(data);
    setupDataStream(stream);
    return deserialize(stream);
}

void AbstractSerializer::setupDataStream(QDataStream& stream)
{
    stream.setByteOrder(QDataStream().byteOrder());
    stream.setVersion(QDataStream::Qt_5_12);
}
} // namespace lib::util
