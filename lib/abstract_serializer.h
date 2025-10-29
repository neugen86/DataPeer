#pragma once

#include <QDataStream>

namespace lib
{
struct AbstractSerializer
{
    virtual ~AbstractSerializer() = default;

    QByteArray toBytes(bool& ok) const;
    bool fromBytes(const QByteArray& data);

    static void setupDataStream(QDataStream& stream);

private:
    virtual bool serialize(QDataStream& out) const = 0;
    virtual bool deserialize(QDataStream& in) = 0;
};
} // namespace lib
