#include "data_channel.h"

#include <QUdpSocket>

DataChannel::DataChannel(QObject* parent)
    : QObject(parent)
{}

DataChannel::~DataChannel()
{
    m_in.reset();
    releaseConnection();
}

bool DataChannel::start(const quint16 port)
{
    if (m_in && port == m_port)
        return true;

    releaseConnection();

    if (m_in)
    {
        m_port = 0;
        m_in->disconnect(this);
    }

    m_in = std::make_unique<QUdpSocket>();

    if (!m_in->bind(QHostAddress::LocalHost, port))
    {
        m_in.reset();
        return false;
    }

    connect(m_in.get(), &QAbstractSocket::disconnected, this, [=]()
    {
        m_port = 0;
        m_in.reset();
        releaseConnection();

        emit failed();
    });

    m_port = port;

    return true;
}

bool DataChannel::connectTo(const QString& host, quint16 port)
{
    if (!m_in)
        return false;

    if (m_out && m_out->peerPort() == port)
        return true;

    releaseConnection();
    m_out = std::make_unique<QUdpSocket>();

    connect(m_out.get(), &QAbstractSocket::disconnected,
            this, &DataChannel::releaseConnection, Qt::QueuedConnection);

    connect(m_out.get(), &QAbstractSocket::connected, this, [=]()
    {
        using std::placeholders::_1;
        auto callback = std::bind(&DataChannel::processMessage, this, _1);
        m_receiver.listen(m_in.get(), callback);

        emit connected();
    });

    m_out->connectToHost(host, port);

    return true;
}

void DataChannel::releaseConnection()
{
    m_out.reset();
    m_receiver.stop();
    emit disconnected();
}

void DataChannel::setCallback(Callback callback)
{
    m_callback = std::move(callback);
}

bool DataChannel::send(const msg::DataMessage& msg)
{
    if (!m_out)
        return false;

    bool ok = false;
    const auto data = msg.toBytes(ok);

    if (!ok)
        return false;

    const int count = m_out->write(data);
    return data.size() == count;
}

bool DataChannel::processMessage(const lib::Message& msg)
{
    if (m_callback && msg.is<msg::DataMessage>())
    {
        m_callback(msg.as<msg::DataMessage>());
        return true;
    }
    return false;
}
