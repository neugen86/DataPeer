#pragma once

#include <QObject>

#include "msg/data.h"
#include "comm/data_channel.h"

class DataPeer : public QObject
{
    Q_OBJECT

public:
    enum State
    {
        Connected,
        Disconnected,
        Failed
    };

    explicit DataPeer(quint16 dataPort, QObject* parent = nullptr);

    void setDataCallback(DataChannel::Callback callback);
    bool sendData(const msg::DataMessage& msg);

    State state() const { return m_state; }

    bool isConnected() const
    {
        return (DataPeer::Connected == m_state);
    }

protected:
    bool createDataChannel();
    bool connectDataPeer(const QString& host, quint16 port);
    void disconnectDataPeer();

private:
    bool setState(State value);

private:
    virtual void onFailed() = 0;

signals:
    void stateChanged(State state, QPrivateSignal = {});
    void connected(QPrivateSignal = {});
    void disconnected(QPrivateSignal = {});
    void failed(QPrivateSignal = {});

private:
    quint16 m_dataPort = 0;
    DataChannel m_dataChannel;
    State m_state = DataPeer::Disconnected;
};
