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

private:
    bool backupData(QDataStream &in);
    bool restoreData();

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    qintptr    socketDescriptor;
    QTcpSocket *socket;
    qint64     blockSize;
    qint8      operation;
    QString    filepath;

    enum OPERATIONS : char {NONE, BACKUP, RESTORE};

};

#endif // SOCKETTHREAD_H
