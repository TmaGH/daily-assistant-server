#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <QThread>
#include <QtNetwork>

class SocketThread : public QThread
{
    Q_OBJECT
public:
    SocketThread(qintptr descriptor, QObject *parent = 0);
    ~SocketThread();

protected:
    void run() override;

signals:
    void onFinishReceived();

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    qintptr     socketDescriptor;
    QTcpSocket *socket;
    qint32      blockSize;
};

#endif // SOCKETTHREAD_H
