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
#include "algorithm"

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

//    bool isInList(QString room);

    void sendCommonMessage(QTcpSocket *socket, int type, int switchh, double temp, int wind, double cost);
    void sendRequestMessage(QTcpSocket *socket, int type, int isServed);

//    void rrIncrease();

private:
    QWidget *parent;
    Ui::MainWindow *ui;
    QFont fontAwesomeSolid;

    QPropertyAnimation *aniSizeChange;
    QPropertyAnimation *aniOpacityChange;
    QGraphicsOpacityEffect *effOpacity;

    QTcpServer *server;
    QList<QWidget *> clients;
    QMap<QString, QTcpSocket *> clientSockets;
    //QVector<QTcpSocket *> sockets;
    //QStringList clientIDs;

    QTimer *rrTimer;
    QStringList SpeedList[4];

    Client * waitingQueue[10];
    Client * servingQueue[10];

    int RES_NUM = 5;
    int wSize = 0;                          // 等待队列长度
    int sSize = 0;                          // 服务队列长度
//    int turn[4];
    bool last_serving[10];
    void initDatabase();
    void initNetwork();
    void initFont();
    void initAnimation();
    void initClientPanel();
//    void initAllocation();
    void resourceAllocation();
    void roundRobin(int speed, int resNum);

    bool wcmp(Client a,Client b);
    bool scmp(Client a, Client b);
    void addIntoWaitingQueue(Client *tempR);
    void removeFromWaitingQueue(Client *tempR);
    void addIntoServingQueue(Client *tempR);
    void removeFromServingQueue(Client *tempR);
    bool isInServingQueue(QString roomId);
    bool canSeize();
    bool mayBeSeize();
    void waitingIntoServing(Client * tempR);
    void bootIntoServing(Client * tempR);
};

#endif // MAINWINDOW_H
