#include "server.h"

Server::Server(quint16 port, QWidget *parent)
    : QTcpServer(parent),
      port(port)
{
    QHostAddress ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();

    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost && ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i);
            break;
        }
    }

    if (ipAddress.isNull())
        ipAddress = QHostAddress(QHostAddress::LocalHost);
    host = ipAddress;
}

void Server::start()
{
    if (this->listen(host, port)) {
        qDebug() << "The server is running on IP: " << host.toString() << ":" << this->serverPort();
    } else {
        qDebug() << "Unable to start the server: " << this->errorString();
    }
}

void Server::incomingConnection(qintptr handle)
{
    qDebug() << "New connection received with handle" << handle;
    SocketThread *thread = new SocketThread(handle);
    connect(thread, &SocketThread::finished, thread, &SocketThread::deleteLater);

    thread->start();
}

Server::~Server()
{

}
