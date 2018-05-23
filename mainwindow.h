#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma execution_character_set("utf-8")

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

   void sendCommonMessage(QTcpSocket *tsock, int msgType, int usSwitch, double dTemp, int usWind, double cost);
   void sendRequestMessage(QTcpSocket *tsock, int msgType, int isServed);

   void RRinc();

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
   QList<QString> LowSpeedList;
   QList<QString> HighSpeedList;

   int turn[3];

   void initDatabase();
   void initNetwork();
   void initFont();
   void initAnimation();
   void initClientPanel();
   void ResourceAllocation();
   void RoundRobin(int level, int maxx);
};

#endif // MAINWINDOW_H
