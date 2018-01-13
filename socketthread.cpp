#include "socketthread.h"

SocketThread::SocketThread(qintptr descriptor, QObject *parent)
    : QThread(parent),
      socketDescriptor(descriptor),
      blockSize(0),
      operation(OPERATIONS::NONE),
      filepath(QDir::currentPath() + "daily-assistant-data")
{}

SocketThread::~SocketThread()
{
    delete socket;
}

void SocketThread::run()
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, this, &SocketThread::onReadyRead, Qt::DirectConnection);
    connect(socket, &QTcpSocket::disconnected, this, &SocketThread::onDisconnected, Qt::DirectConnection);
    exec();
}

void SocketThread::onReadyRead()
{   
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_10);

    if(socket->bytesAvailable() < sizeof(operation))
        return;

    if(operation == OPERATIONS::NONE)
        in >> operation;

    bool success;
    if(operation == OPERATIONS::BACKUP)
        success = backupData(in);
    else if(operation == OPERATIONS::RESTORE)
        success = restoreData();

    if(success)
        socket->disconnectFromHost();
}

bool SocketThread::backupData(QDataStream& in)
{
    /*
     * Beginning of data will include an 8 byte integer which informs us the size of the actual block in bytes.
     * When this data has been received, it will written into quint64 blockSize.
    */

    if(socket->bytesAvailable() < sizeof(blockSize))
        return false;

    if(blockSize == 0) {
        in >> blockSize;
        qDebug() << "Block size:" << blockSize;
    }

    /*
     * Use blockSize to determine when all of the data has been received, then read all to QByteArray.
    */

    QByteArray line;
    if(socket->bytesAvailable() < blockSize)
        return false;
    else {
        line = socket->readAll();
    }

    if(line.size() == blockSize)
        qDebug() << "All incoming data written to QByteArray," << line.size() << "bytes. Next, saving data to file...";
    else
        return false;

    if(QFile::exists(filepath)) {
        QFile::copy(filepath, filepath+".backup");
        QFile::remove(filepath);
    }
    QFile file(filepath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Can't open file for writing.";
        return false;
    }

    file.write(line);
    file.close();

    if(QFile::exists(filepath) && file.size() == line.size()) {
        qDebug() << "Succesfully saved data to file," << file.size() << "bytes.";
        QFile::remove(filepath+".backup");
    } else {
        qDebug("Saving data failed. Restoring previous data...");
        QFile::copy(filepath+".backup", filepath);
        QFile::remove(filepath+".backup");
    }
    return true;
}

bool SocketThread::restoreData()
{
    QFileInfo check_file(filepath);
    if (check_file.exists() && check_file.isFile()) {
        QFile file(filepath);

        if(!file.open(QFile::ReadOnly)) {
            qDebug() << "Can't open file.";
            return true;
        }

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_10);

        QByteArray q = file.readAll();
        file.close();

        qint64 blockSize = q.size();
        out << blockSize;
        block.append(q);
        out.device()->seek(0);
        qDebug() << "Block size:" << blockSize;
        qint64 x = 0;
        while(x < block.size()) {
            qint64 y = socket->write(block);
            if(y >= 0) {
            x += y;
            qDebug() << x << "bytes written.";
            } else {
                qDebug() << "Error occured while writing data.";
            }
        }
        socket->waitForBytesWritten();
    } else {
        qDebug() << "No data to send.";
    }
    return true;
}

void SocketThread::onDisconnected()
{
    socket->close();
    quit();
}
