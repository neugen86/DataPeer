#pragma once

#include <memory>
#include <functional>

#include <QObject>

#include "msg/data.h"
#include "lib/transceiver.h"

class QUdpSocket;
class DataMessage;

class DataChannel : public QObject
{
    Q_OBJECT

public:
    explicit DataChannel(QObject* parent = nullptr);
    ~DataChannel() override;

    bool start(quint16 port);

    bool connectTo(const QString& host, quint16 port);
    void releaseConnection();

    using Callback = std::function<void(const msg::DataMessage&)>;
    void setCallback(Callback callback);

    bool send(const msg::DataMessage& msg);

private:
    bool processMessage(const lib::Message& msg);

signals:
    void connected(QPrivateSignal = {});
    void disconnected(QPrivateSignal = {});
    void failed(QPrivateSignal = {});

private:
    quint16 m_port = 0;
    Callback m_callback;
    lib::Transceiver m_receiver;
    std::unique_ptr<QUdpSocket> m_in;
    std::unique_ptr<QUdpSocket> m_out;
};
