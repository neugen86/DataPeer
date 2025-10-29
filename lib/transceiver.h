#pragma once

#include <functional>

#include <QObject>
#include <QBuffer>
#include <QPointer>

#include "message_sender.h"

class QUdpSocket;

namespace lib
{
class Message;

class Transceiver : public QObject
                  , public MessageSender
{
    Q_OBJECT

public:
    explicit Transceiver(QObject* parent = nullptr);

    using Callback = std::function<bool(const lib::Message&)>;
    bool listen(QIODevice* device, Callback callback);

    bool write(const QByteArray& data) const;
    void stop();

public: // MessageSender
    bool send(const lib::Message& msg) const override;

private:
    void onReadyRead();
    bool readData();
    void flush();

signals:
    void stopped(QPrivateSignal = {});

private:
    QBuffer m_buffer;
    Callback m_callback;
    QPointer<QIODevice> m_device;
    QPointer<QUdpSocket> m_udpDevice;
};
} // namespace lib
