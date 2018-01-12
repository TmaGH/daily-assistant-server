#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QtNetwork>
#include <QtCore>
#include <QLabel>
#include <QMessageBox>
#include <QFile>

#include "serverdialog.h"
#include "socketthread.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
   explicit Server(quint16 port = 41815, QWidget *parent = nullptr);
    void start();
    ~Server();


private:
    QHostAddress host;
    quint16 port;
    QVector<QFile> files;

protected:
    void incomingConnection(qintptr handle) override;

};

#endif // SERVER_H
