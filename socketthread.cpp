#include "socketthread.h"

SocketThread::SocketThread(qintptr descriptor, QObject *parent)
    : QThread(parent),
      socketDescriptor(descriptor),
      blockSize(0)
{
}

SocketThread::~SocketThread()
{
    delete socket;
}

void SocketThread::run()
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead,    this, &SocketThread::onReadyRead,    Qt::DirectConnection);
    connect(socket, &QTcpSocket::disconnected, this, &SocketThread::onDisconnected, Qt::DirectConnection);

    exec();
}

void SocketThread::onReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_10);

    if (blockSize == 0) {
        if (socket->bytesAvailable() < sizeof(quint32))
        return;
        in >> blockSize;
    }
    if (socket->bytesAvailable() < blockSize)
        return;

    QByteArray line = socket->readAll();

    QString filepath = "/daily_assistant_data";
    if(QFile::exists(filepath)) {
        QFile::copy(filepath, filepath+".backup");
        QFile::remove(filepath);
    }
    QFile file(filepath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file.";
        return;
    }
    file.write(line);

    file.close();

    emit onFinishReceived();
    socket->disconnectFromHost();
    qDebug() << "Saved file succesfully.";
}

void SocketThread::onDisconnected()
{
    socket->close();
    quit();
}
