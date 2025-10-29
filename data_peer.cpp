#include "data_peer.h"

#include <QDebug>

#define BIND_SIGNALS(SignalType, StateType) \
connect(&m_dataChannel, &DataChannel::SignalType, this, [=]() { \
    if (setState(StateType)) emit SignalType(); \
}, Qt::QueuedConnection);

DataPeer::DataPeer(quint16 dataPort, QObject* parent)
    : QObject(parent)
    , m_dataPort(dataPort)
{
    BIND_SIGNALS(failed, DataPeer::Failed);
    BIND_SIGNALS(connected, DataPeer::Connected)
    BIND_SIGNALS(disconnected, DataPeer::Disconnected);
}

void DataPeer::setDataCallback(DataChannel::Callback callback)
{
    m_dataChannel.setCallback(std::move(callback));
}

bool DataPeer::sendData(const msg::DataMessage& msg)
{
    return m_dataChannel.send(msg);
}

bool DataPeer::createDataChannel()
{
    qDebug() << "-- Peer:" << "creating data channel ...";
    bool ok = m_dataChannel.start(m_dataPort);
    qDebug() << "-- Peer:" << "data channel created:" << ok;
    return ok;
}

bool DataPeer::connectDataPeer(const QString& host, quint16 port)
{
    qDebug() << "-- Peer:" << "connecting to data peer ...";
    bool ok = m_dataChannel.connectTo(host, port);
    qDebug() << "-- Peer:" << "data peer connected:" << ok;
    return true;
}

void DataPeer::disconnectDataPeer()
{
    qDebug() << "-- Peer:" << "disconnecting from data peer ...";
    m_dataChannel.releaseConnection();
    qDebug() << "-- Peer:" << "data peer disconnected";
}

bool DataPeer::setState(const State value)
{
    if (value == m_state)
        return false;

    m_state = value;

    if (DataPeer::Failed == m_state)
    {
        onFailed();
    }

    emit stateChanged(m_state);
    return true;
}
