#include <atomic>
#include <memory>
#include <thread>

#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

#include "client_peer.h"
#include "server_peer.h"
#include "msg/actions.h"
#include "msg/data.h"

std::thread g_clientThread;
std::thread g_serverThread;
std::atomic_bool g_running = true;
int g_stateDelayMs = 300;

void ClientRoutine(DataPeer* peer)
{
    int num = 0;
    msg::ClientStateMessage state;
    msg::ServerStateMessage unexpected;

    qDebug() << "[" << Q_FUNC_INFO << "]" << "STARTED";

    while (g_running && peer->isConnected())
    {
        if (++num % 5) {
            state.data = QString("Client state #%1").arg(num).toUtf8();
            peer->sendData(state);
        }
        else {
            peer->sendData(unexpected);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(g_stateDelayMs));
    }
    qDebug() << "[" << Q_FUNC_INFO << "]" << "FINISHED";
}

void ServerRoutine(DataPeer* peer)
{
    int num = 0;
    msg::ServerStateMessage state;
    msg::ClientStateMessage unexpected;

    qDebug() << "[" << Q_FUNC_INFO << "]" << "STARTED";

    while (g_running && peer->isConnected())
    {
        if (++num % 7) {
            state.data = QString("Server state #%1").arg(num).toUtf8();
            peer->sendData(state);
        } else {
            peer->sendData(unexpected);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(g_stateDelayMs));
    }
    qDebug() << "[" << Q_FUNC_INFO << "]" << "FINISHED";
}

void EnablePrintableState(DataPeer* peer, const QString& id)
{
    QObject::connect(peer, &DataPeer::stateChanged,
                     peer, [=](DataPeer::State state)
    {
        switch (state)
        {
        case DataPeer::Connected:
            qDebug() << "======= " << id << ": connected =======";
            break;
        case DataPeer::Disconnected:
            qDebug() << "======= " << id << ": disconnected =======";
            break;
        case DataPeer::Failed:
            qDebug() << "======= " << id << ": failed =======";
            break;
        default:
            qCritical() << "======= " << id << ": unknown state=======";
        }
    });
}

std::shared_ptr<ClientPeer> CreateClient(quint16 port)
{
    auto client = std::make_shared<ClientPeer>(port);
    EnablePrintableState(client.get(), "CLIENT_STATE");

    client->setDataCallback([](const msg::DataMessage& msg)
    {
        if (msg.is<msg::ServerStateMessage>()) {
            auto state = msg.as<msg::ServerStateMessage>();
            qDebug() << "CLIENT_ON_DATA:" << state.data;
            return;
        }
        qDebug() << "CLIENT_ON_DATA: unexpected type";
    });

    QObject::connect(client.get(), &DataPeer::connected, [=]()
    {
        auto send = [=](const auto& action, auto actionName)
        {
            qDebug() << "= CLIENT_ACTIONS: requesting" << actionName;
            client->sendAction(action, [=](bool success) {
                if (!success) {
                    qDebug() << "===== CLIENT_ACTIONS:" << actionName << "REJECTED";
                    return;
                }
                qDebug() << "===== CLIENT_ACTIONS:" << actionName << "APPLIED";
                if (!g_clientThread.joinable()) {
                    g_clientThread = std::thread(&ClientRoutine, client.get());
                }
            });
        };

        QMetaObject::invokeMethod(client.get(), [=](){
            msg::FooAction foo;
            send(foo, "(FooAction)");
        }, Qt::QueuedConnection);

        msg::BarAction bar;
        send(bar, "(BarAction)");
    });

    return client;
}

std::shared_ptr<ServerPeer> CreateServer(quint16 port)
{
    auto server = std::make_shared<ServerPeer>(port);
    EnablePrintableState(server.get(), "SERVER_STATE");

    server->setActionsCallback([=](const msg::ActionRequest& req)
    {
        if (req.is<msg::FooAction>()) {
            auto action = req.as<msg::FooAction>();
            qDebug() << "=== SERVER_ACTIONS: APPLYING (FooAction)";

            if (!g_serverThread.joinable()) {
                g_serverThread = std::thread(&ServerRoutine, server.get());
            }

            return true;
        }
        qDebug() << "=== SERVER_ACTIONS: REJECTING unexpected action";
        return false;
   });

    server->setDataCallback([=](const msg::DataMessage& msg)
    {
        if (msg.is<msg::ClientStateMessage>()) {
            auto state = msg.as<msg::ClientStateMessage>();
            qDebug() << "SERVER_ON_DATA:" << state.data;
            return;
        }
        qDebug() << "SERVER_ON_DATA: unexpected type";
    });

    return server;
}

int main(int argc, char* argv[])
{
    qDebug() << "=========================";
    qDebug() << "=== Staring =============";
    QCoreApplication app(argc, argv);

    auto client = CreateClient(4567);
    auto server = CreateServer(4568);
    const bool stopClientToExit = false;

    QObject::connect(client.get(), &DataPeer::disconnected,
                     &app, &QCoreApplication::quit);

    QTimer::singleShot(std::chrono::seconds(3), &app, [=]()
    {
        if (stopClientToExit) {
            client->disconnectFromHost();
        } else {
            server->stop();
        }
    });

    bool ok = false;
    const quint16 commPort = 12345;
    ok = server->start(commPort);
    ok = ok && client->connectToHost("localhost", commPort);

    int exitCode = ok ? app.exec() : 1;
    g_running = false;

    server->stop();
    app.processEvents();

    if (g_clientThread.joinable()) {
        g_clientThread.join();
    }

    if (g_serverThread.joinable()) {
        g_serverThread.join();
    }

    qDebug() << "==========================================";
    return exitCode;
}
