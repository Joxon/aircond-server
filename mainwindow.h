#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#define USE_JSON
//#define USE_DATA_STREAM

#include "client.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_toolButtonPower_toggled(bool checked);

    void storeSockets();
    void readFromSockets();

    void sendCommonMessage(QTcpSocket *socket, int type, int switchh, double temp, int wind, double cost);
    void sendRequestMessage(QTcpSocket *socket, int type, int isServed);

    void rrIncrease();

private:
    QWidget *parent;
    Ui::MainWindow *ui;
    QFont fontAwesomeSolid;

    QPropertyAnimation *aniSizeChange;
    QPropertyAnimation *aniOpacityChange;
    QGraphicsOpacityEffect *effOpacity;

    QTcpServer *server;
    QVector<QTcpSocket *> sockets;
    QList<QWidget *> clients;
    QStringList clientIDs;

    QTimer *rrTimer;
    QStringList lowSpeedList;
    QStringList highSpeedList;

    QMap <QString, QTcpSocket *> room_socket;

    const int RES_NUM = 5;
    int turn[3];
    bool last_serving[8];
    void initDatabase();
    void initNetwork();
    void initFont();
    void initAnimation();
    void initClientPanel();
    void initAllocation();
    void resourceAllocation();
    void roundRobin(Client::Speed speed, int resNum);
};

#endif // MAINWINDOW_H
