#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quiwidget.h"
#include "IconsFontAwesome5.h"

MainWindow::MainWindow(QWidget *parent) :
   parent(parent),
   ui(new Ui::MainWindow)
{
   ui->setupUi(this);
   QUIWidget::setFormInCenter(this);

   initDatabase();
   initFont();
   initAnimation();
   initClientPanel();

   LowSpeedList.clear();
   HighSpeedList.clear();   // 队列清零
   turn[0] = 1; turn[1] = turn[2] = 0;
   QTimer *timer = new QTimer(this);
   connect(timer, SIGNAL(timeout()), this, SLOT(RRinc()));
   timer->start(999);
}


MainWindow::~MainWindow()
{
   delete ui;
}


void MainWindow::initDatabase()
{
   QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");

   database.setDatabaseName("air_conditioning.db");
   if (!database.open())
   {
      qDebug() << "Open success!";
   }
   else
   {
      qDebug() << "Open failed!";
   }
}


void MainWindow::initFont()
{
   int     fontId   = QFontDatabase::addApplicationFont(":/image/Font-Awesome-5-Free-Solid-900.otf");
   QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);

   fontAwesomeSolid = QFont(fontName);
   fontAwesomeSolid.setPixelSize(50);

   ui->toolButtonPower->setFont(fontAwesomeSolid);
   ui->toolButtonPower->setText((QChar)ICON_FA_POWER_OFF);

   ui->labelArrowDown->setFont(fontAwesomeSolid);
   ui->labelArrowDown->setText((QChar)ICON_FA_ANGLE_DOUBLE_DOWN);
}


void MainWindow::initAnimation()
{
   aniSizeChange = new QPropertyAnimation(parent, "geometry");
   aniSizeChange->setDuration(500);
   aniSizeChange->setEasingCurve(QEasingCurve::OutCirc);

   effOpacity = new QGraphicsOpacityEffect();
   effOpacity->setOpacity(0.0);
   ui->clientPanel->setGraphicsEffect(effOpacity);

   aniOpacityChange = new QPropertyAnimation(effOpacity, "opacity");
   aniOpacityChange->setDuration(500);
   aniOpacityChange->setEasingCurve(QEasingCurve::InCirc);
}


void MainWindow::initClientPanel()
{
   //加载设备面板
   qDeleteAll(clients);
   clients.clear();

   ui->clientPanel->setMargin(5);
   ui->clientPanel->setSpacing(5);

//   for (int i = 0; i < 8; ++i)
//   {
//      Client *client = new Client;
//      client->setFixedHeight(150);
//      client->setId(QString("%1").arg(i + 1));
//      //client->setConn(Client::ConnOnline);
//      clients.append(client);
//   }
//   ui->clientPanel->setWidget(clients, 4);
}


void MainWindow::initNetwork()
{
   server = new QTcpServer(this);
   // 使用了IPv4的本地主机地址，等价于QHostAddress("127.0.0.1")
   if (!server->listen(QHostAddress::Any, 6666))
   {
      qDebug() << server->errorString();
   }
   connect(server, SIGNAL(newConnection()), this, SLOT(storeSockets()));
}


void MainWindow::on_toolButtonPower_toggled(bool checked)
{
   if (checked)
   {
      initNetwork();

      ui->labelArrowDown->hide();
      //aniSizeChange->setStartValue(parent->geometry());
      aniSizeChange->setEndValue(QRect(parent->x(), parent->y(), 800, 600));
      aniSizeChange->start();

      aniOpacityChange->setStartValue(0.1);
      aniOpacityChange->setEndValue(0.99);// cant use 1.0??
      aniOpacityChange->start();
   }
   else
   {
      delete server;

      ui->labelArrowDown->show();

      //aniSizeChange->setStartValue(parent->geometry());
      aniSizeChange->setEndValue(QRect(parent->x(), parent->y(), 800, 200));
      aniSizeChange->start();

      aniOpacityChange->setStartValue(1.0);
      aniOpacityChange->setEndValue(0.1);
      aniOpacityChange->start();
   }
}


void MainWindow::storeSockets()
{
   //将已经建立连接的套接字加入容器
   sockets.append(server->nextPendingConnection());
   connect(sockets.last(), SIGNAL(readyRead()), this, SLOT(readFromSockets()));
   //connect(sockets.last(), SIGNAL(disconnected()), sockets.last(), SLOT(deleteLater()));
}


void MainWindow::readFromSockets()
{
   for (int i = 0; i < sockets.size(); ++i)
   {
      //验证socket是否合法
      QTcpSocket *socket = sockets.at(i);
      if (!socket->isValid())
      {
         sockets.remove(i);
         continue;
      }
      if (socket->bytesAvailable() < (int)(sizeof(quint16)))
      {
         continue;
      }

//      QDataStream in(socket);
//      in.setVersion(QDataStream::Qt_5_5);
//      quint16 blockSize = 0;
//      in >> blockSize;
//      if (sockets.at(i)->bytesAvailable() < blockSize)
//      {
//         continue;
//      }


      //JSON解析
      int             msgType, usSwitch, wind;
      QString         roomID;
      double          dTemp;
      QByteArray      jsonBytes = socket->readLine();
      QJsonParseError jsonErr;
      QJsonDocument   jsonDoc = QJsonDocument::fromJson(jsonBytes, &jsonErr);
      if (jsonErr.error == QJsonParseError::NoError)
      {
         if (jsonDoc.isObject())
         {
            QJsonObject jsonObj = jsonDoc.object();
            if (jsonObj.contains("type"))
            {
               QJsonValue type_value = jsonObj.take("type");
               if (type_value.isDouble())
               {
                  msgType = type_value.toInt();
               }
            }
            if (jsonObj.contains("room"))
            {
               QJsonValue room_value = jsonObj.take("room");
               if (room_value.isString())
               {
                  roomID = room_value.toString();
               }
            }
            if (jsonObj.contains("switch"))
            {
               QJsonValue switch_value = jsonObj.take("switch");
               if (switch_value.isDouble())
               {
                  usSwitch = switch_value.toInt();
               }
            }
            if (jsonObj.contains("temperature"))
            {
               QJsonValue temperature_value = jsonObj.take("temperature");
               if (temperature_value.isDouble())
               {
                  dTemp = temperature_value.toDouble();
               }
            }
            if (jsonObj.contains("wind"))
            {
               QJsonValue wind_value = jsonObj.take("wind");
               if (wind_value.isDouble())
               {
                  wind = wind_value.toInt();
               }
            }
         }
      }

      //新的ID，新增client
      Client *client;
      int    clientIdx = clientIDs.indexOf(roomID);
      if (clientIdx == -1)
      {
         clientIDs.append(roomID);

         client = new Client;
         client->setFixedHeight(150);
         client->setId(roomID);
         client->setST();
         clients.append(client);
      }
      //旧的ID，更新client
      else
      {
         client = qobject_cast<Client *>(clients.at(clientIdx));
      }

      if (msgType == 1)
      {
         if(client->CheckServing())
            client->Cost_Cal(dTemp);
         client->setCurrentTemp(dTemp);
         sendCommonMessage(socket, msgType, 1,
                           client->getCurrentTemp(),
                           (int)client->getSpeed(),
                           client->getCost());

         qDebug() << DATETIME
                  << " readFromSockets: " << msgType << " " << roomID << " " << dTemp;
      }
      //请求报文
      else if (msgType == 0)
      {
          int hi, li;
         switch (usSwitch)
         {
         case 0:    // 关机 此时存储一次账单
            client->setWorking(Client::WorkingNo);
            client->write_detail_list(roomID);
            break;

         case 1:
            client->setTargetTemp(dTemp);

            if (wind == 0)
            {
               client->setSpeed(Client::SpeedNone);
               // 从队列中清除
               hi = HighSpeedList.indexOf(roomID);
               if(hi != -1) HighSpeedList.removeAt(hi);
               li = LowSpeedList.indexOf(roomID);
               if(li == -1) LowSpeedList.removeAt(li);
            }
            else if (wind == 1)
            {
               client->setSpeed(Client::SpeedLow);
               // 判断是否更新低风队列
               hi = HighSpeedList.indexOf(roomID);
               if(hi != -1) HighSpeedList.removeAt(hi);
               li = LowSpeedList.indexOf(roomID);
               if(li == -1) LowSpeedList.append(roomID);
            }
            else if (wind == 2)
            {
               client->setSpeed(Client::SpeedHigh);
               // 判断是否更新高风队列
               hi = HighSpeedList.indexOf(roomID);
               if(hi == -1) HighSpeedList.append(roomID);
               li = LowSpeedList.indexOf(roomID);
               if(li != -1) LowSpeedList.removeAt(li);
            }

            client->setWorking(Client::WorkingYes);
            break;

         default:
            break;
         }

//         sendRequestMessage(socket, msgType, 1);
//         client->setServing(Client::ServingYes);
//         ResourceAllocation();
        if(client->CheckServing())  // 分配成功
        {
            // 有bug
            // 分配资源
        }
        else
        {
            // 不分配
        }
         qDebug() << DATETIME
                  << " readFromSockets: "
                  << msgType << " "
                  << roomID << " "
                  << dTemp << " "
                  << usSwitch << " "
                  << wind;
      }
      break;
   }

   ui->clientPanel->setWidget(clients, 4);
}

void MainWindow::ResourceAllocation()
{
    // 先将全部房间服务置零
    for(int i = 0; i < clients.size(); i++)
    {
        Client * client = qobject_cast<Client *>(clients.at(i));
        client->setServing(Client::ServingNo);
    }

    int hn = HighSpeedList.size();
    int ln = LowSpeedList.size();
    int surplus = 5 - hn;
    if(hn <= 5)             // 可是把5改成一个define
    {   // high 不需要轮转
        for(int i = 0; i < hn; i++)
        {
            QString temp = HighSpeedList.at(i);
            Client * client = qobject_cast<Client *>(clients.at(clientIDs.indexOf(temp)));
            client->setServing(Client::ServingYes);
        }
    }
    else
    {   // high 需要轮转
        RoundRobin(2, 5);
    }

    if(surplus <= 0) return;        // high占用全部资源

    if(ln <= surplus)
    {   // low 不需要轮转
        for(int i = 0; i < ln; i++)
        {
            QString temp = HighSpeedList.at(i);
            Client * client = qobject_cast<Client *>(clients.at(clientIDs.indexOf(temp)));
            client->setServing(Client::ServingYes);
        }
    }
    else
    {   // low 需要轮转
        RoundRobin(1, surplus);
    }
}

void MainWindow::RRinc()
{
    turn[0] ++;
    if(turn[0] % 5 == 0)
    {
        turn[1] = (turn[1] + 1) % LowSpeedList.size();
        turn[2] = (turn[2] + 1) % HighSpeedList.size();
    }
    ResourceAllocation();
}

void MainWindow::RoundRobin(int level, int maxx)            // 轮转
{
    int M;
    if(level == 1) M = LowSpeedList.size();
    else    M = HighSpeedList.size();
    // turn[0] 每5s增加一次

    for(int i = turn[level], j = 0; j < maxx; j++, i = (i+1)%M)
    {
        QString temp = HighSpeedList.at(i);
//        qDebug() << "资源分配：" << temp;
        Client * client = qobject_cast<Client *>(clients.at(clientIDs.indexOf(temp)));
        client->setServing(Client::ServingYes);
    }
}

void MainWindow::sendRequestMessage(QTcpSocket *tsock, int msgType, int isServed)
{
   QJsonObject json;

   json.insert("type", msgType);
   json.insert("isServed", isServed);
   json.insert("cost",  cost);

   QJsonDocument document;
   document.setObject(json);
   QByteArray byte_array = document.toJson(QJsonDocument::Compact);

   tsock->write(byte_array);

   qDebug() << DATETIME << " sendRequestMessage: ";

//   QByteArray  block;
//   QDataStream out(&block, QIODevice::WriteOnly);

//   out.setVersion(QDataStream::Qt_5_5);

//   out << (quint16)0;
//   out << type;
//   out << isServed;
//   out.device()->seek(0);
//   out << (quint16)(block.size() - sizeof(quint16));

//   tsock->write(block);

//   qDebug() << DATETIME << " sendRequestMessage: ";
}


void MainWindow::sendCommonMessage(QTcpSocket *tsock, int msgType, int usSwitch, double dTemp, int usWind, double cost)
{
   QJsonObject json;

   json.insert("type", msgType);
   json.insert("switch", usSwitch);
   json.insert("temperature", dTemp);
   json.insert("wind", usWind);
   json.insert("cost", cost);

   QJsonDocument document;
   document.setObject(json);

   QByteArray byte_array = document.toJson(QJsonDocument::Compact);
   tsock->write(byte_array);

   qDebug() << DATETIME << " sendCommonMessage: ";

//   QByteArray  block;
//   QDataStream out(&block, QIODevice::WriteOnly);

   // 设置数据流的版本，客户端和服务器端使用的版本要相同
//   out.setVersion(QDataStream::Qt_5_5);
//   out << (quint16)0;
//    out << QString("0");
//    out << roomMap[roomID];
//   out << type;
//   out << Switch;
//   out << temperature;
//   out << wind;
//   out << cost;
//   out.device()->seek(0);
//   out << (quint16)(block.size() - sizeof(quint16));
//   tsock->write(block);
//clientConnection->disconnectFromHost();
// 发送数据成功后，显示提示
//    roomMap[roomID]++;
}


//void MainWindow::Cycle_Check()
//{
//   // 资源分配，通告信息
//   Service_Allocation();
//   // 送信
//}


//void MainWindow::Service_Allocation()
//{
//   int Working = 0;

//   for (int i = 0; i < Num; i++)
//   {
//      if (rooms[i].Link && rooms[i].Work)       // 链接且工作
//      {
//         Working++;
//      }
//   }
//   if (Working < 6)             // 直接分配
//   {
//      // 对这些房间直接允许服务
//      for (int i = 0; i < Num; i++)
//      {
//         if (rooms[i].Link && rooms[i].Work)    // 链接且工作
//         {
//            rooms[i].Service = true;
//         }
//      }
//   }
//   else                  // 超过阈值，轮转算法
//   {                     // 外部一个调用，在T的周期下调用整个Servive函数，全局有个k用于简单轮转
//      int Remainder = 5; // 剩余工作数
//      for (int i = k; (i + 1) % Num != k; i++)
//      {
//         if (rooms[i].Link && rooms[i].Work && Remainder)       // 链接且工作且有剩余
//         {
//            rooms[i].Service = true;
//            Remainder--;
//         }
//      }
//      k = (k + 1) % Num;
//   }
//}
