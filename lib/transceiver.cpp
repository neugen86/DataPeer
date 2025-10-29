#include "transceiver.h"

#include <QDateTime>
#include <QUdpSocket>
#include <QNetworkDatagram>

#include "message.h"

#define TEST_GARBAGE_DATA

namespace
{
const int cMaxBufferSize = 1024;

} // anonymous namespace

namespace lib
{
Transceiver::Transceiver(QObject* parent)
    : QObject(parent)
{}

bool Transceiver::listen(QIODevice* device, Callback callback)
{
    m_buffer.close();

    if (!callback || !device ||
        !m_buffer.open(QIODevice::ReadWrite))
    {
        stop();
        return false;
    }

    m_device = device;
    m_callback = std::move(callback);

    m_udpDevice = qobject_cast<QUdpSocket*>(m_device);

    connect(m_device, &QObject::destroyed,
            this, &Transceiver::stop);

    connect(m_device, &QIODevice::aboutToClose,
            this, &Transceiver::stop);

    connect(m_device, &QIODevice::readyRead,
            this, &Transceiver::onReadyRead);

    if (m_device->isOpen())
    {
        onReadyRead();
    }

    return true;
}

bool Transceiver::write(const QByteArray& data) const
{
#ifdef TEST_GARBAGE_DATA
    m_device->write("qwe");
#endif // TEST_GARBAGE_DATA

    const int count = m_device->write(data);

#ifdef TEST_GARBAGE_DATA
    m_device->write("asd");
#endif // TEST_GARBAGE_DATA

    return count == data.size();
}

bool Transceiver::send(const lib::Message& msg) const
{
    bool ok = false;
    const auto data = msg.toBytes(ok);
    return ok && write(data);
}

void Transceiver::stop()
{
    if (m_device)
    {
        m_device->disconnect(this);
    }

    m_buffer.close();
    m_buffer.setData({});

    m_device.clear();
    m_udpDevice.clear();
    m_callback = nullptr;

    emit stopped();
}

void Transceiver::onReadyRead()
{
    QByteArray data;
    const int pos = m_buffer.pos();

    if (m_udpDevice)
    {
        while (m_udpDevice->hasPendingDatagrams())
        {
            const auto datagram = m_udpDevice->receiveDatagram();
            data.append(datagram.data());
        }
    }
    else
    {
        data = m_device->readAll();
    }

    m_buffer.write(data);
    m_buffer.seek(pos);

    if (readData() || m_buffer.size() > cMaxBufferSize)
    {
        flush();
    }
}

bool Transceiver::readData()
{
    bool ok = false;

    for (lib::MessageBounds bounds; !m_buffer.atEnd(); ok = true)
    {
        const auto foo = m_buffer.data();

        bounds = lib::Message::getBounds(
            lib::Sentinel::message_start,
            lib::Sentinel::message_end,
            m_buffer.data(), m_buffer.pos());

        if (!bounds.valid())
            break;

        const auto data = m_buffer.data().mid(bounds.pos, bounds.size);
        m_buffer.seek(bounds.endPos());

        m_callback(lib::Message(data));
    }

    return ok;
}

void Transceiver::flush()
{
    const int pos = m_buffer.pos();
    const auto data = m_buffer.readAll().mid(pos);

    m_buffer.close();
    m_buffer.setData(data);

    if (!m_buffer.open(QIODevice::ReadWrite))
    {
        emit stopped();
        return;
    }

    m_buffer.seek(0);
}
} // namespace lib
