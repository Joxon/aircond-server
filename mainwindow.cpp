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
    initAllocation();
}


MainWindow::~MainWindow()
{

    delete ui;
    delete rrTimer;
}


void MainWindow::initDatabase()
{

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");


    database.setDatabaseName("air_conditioning.db");
    if (!database.open())
    {
        qDebug() << DATETIME << "DB is NOT open:" << database.lastError();
        exit(EXIT_FAILURE);
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


void MainWindow::initAllocation()
{
    lowSpeedList.clear();
    highSpeedList.clear();
    turn[0] = 1;
    turn[1] = turn[2] = 0;

    rrTimer = new QTimer(this);
    connect(rrTimer, SIGNAL(timeout()), this, SLOT(rrIncrease()));
    rrTimer->start(999);
}


void MainWindow::initNetwork()
{
    server = new QTcpServer(this);
    // 使用了IPv4的本地主机地址，等价于QHostAddress("127.0.0.1")
    if (!server->listen(QHostAddress::Any, 6666))
    {
        qDebug() << DATETIME << "initNetwork:" << server->errorString();
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
        aniOpacityChange->setEndValue(0.99);        // cant use 1.0??
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
        int             type;
        int             switchh;
        int             wind;
        QString         room;
        double          temp;
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
                        type = type_value.toInt();
                    }
                }
                if (jsonObj.contains("room"))
                {
                    QJsonValue room_value = jsonObj.take("room");
                    if (room_value.isString())
                    {
                        room = room_value.toString();
                    }
                }
                if (jsonObj.contains("switch"))
                {
                    QJsonValue switch_value = jsonObj.take("switch");
                    if (switch_value.isDouble())
                    {
                        switchh = switch_value.toInt();
                    }
                }
                if (jsonObj.contains("temperature"))
                {
                    QJsonValue temperature_value = jsonObj.take("temperature");
                    if (temperature_value.isDouble())
                    {
                        temp = temperature_value.toDouble();
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
//      qDebug() << DATETIME << "receive message : Type : " <<  type << " Temp : " << dTemp;

        if(room == "") continue;
        //新的ID，新增client
        Client *client;
        int    clientIdx = clientIDs.indexOf(room);
        if (clientIdx == -1)
        {
            clientIDs.append(room);

            client = new Client;
            client->setFixedHeight(150);
            client->setId(room);
            client->setStartTime();
            client->setCost(0);
            client->setCurrentTemp(28);
            clients.append(client);
//         qDebug() << DATETIME << "clent.start_t : " << client->getTime().toString("yyyy-MM-dd hh:mm:ss");
      }
      //旧的ID，更新client
      else
      {
         qDebug() << "client at 更新 (起点): ___ ";
         client = qobject_cast<Client *>(clients.at(clientIdx));
         qDebug() << "client at 更新 (终点): ___";
      }

      if (msgType == 1)
      {
         if (client->CheckServing())
         {
            client->Cost_Cal(dTemp);
         }
         client->setCurrentTemp(dTemp);
//         sendCommonMessage(socket, 1, 1,
//                           client->getCurrentTemp(),
//                           (int)client->getSpeed(),
//                           client->getCost());

         qDebug() << DATETIME << " readFromSockets: " << msgType << " " << roomID << " " << dTemp;
         if(client->CheckServing())  // 分配成功
         {
             qDebug() << "give one resource for room--" << roomID;
             sendCommonMessage(socket, 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
             qDebug() << "Send success!" ;
         }
         else
         {
             qDebug() << "kill one resource for room--" << roomID;
             sendCommonMessage(socket, 1, 0, 0, 0, 0);
         }
      }
      //请求报文
      else if (msgType == 0)
      {
         int hi = -1, li = -1;
         QDateTime temp_t;
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
               qDebug() << "无风测试(起点) : " << roomID;
               if (HighSpeedList.size())
               {
                  hi = HighSpeedList.indexOf(roomID);
               }
               if (hi != -1)
               {
                  HighSpeedList.removeAt(hi);
               }

               if (LowSpeedList.size())
               {
                  li = LowSpeedList.indexOf(roomID);
               }
               if (li != -1)
               {
                  LowSpeedList.removeAt(li);
                  qDebug() << "删除成功！ --------------------------------------";
               }
               client->write_detail_list(roomID);
               qDebug() << "无风测试(终点) : " << roomID;
            }
            else if (wind == 1)
            {
               client->setSpeed(Client::SpeedLow);
               // 判断是否更新低风队列
               qDebug() << "低风测试(起点) : " << roomID;
               if (HighSpeedList.size())
               {
                  hi = HighSpeedList.indexOf(roomID);
               }
               if (hi != -1)
               {
                  HighSpeedList.removeAt(hi);
               }

               if (LowSpeedList.size())
               {
                  li = LowSpeedList.indexOf(roomID);
               }
               if (li == -1)
               {
                  LowSpeedList.append(roomID);
               }
               qDebug() << "低风测试(终点) : " << roomID;
            }
            else if (wind == 2)
            {
               client->setSpeed(Client::SpeedHigh);
               // 判断是否更新高风队列
               qDebug() << "高风测试(起点) : " << roomID;
               if (HighSpeedList.size())
               {
                  hi = HighSpeedList.indexOf(roomID);
               }
               if (hi == -1)
               {
                  HighSpeedList.append(roomID);
               }
               if (LowSpeedList.size())
               {
                  li = LowSpeedList.indexOf(roomID);
               }
               if (li != -1)
               {
                  LowSpeedList.removeAt(li);
               }
               qDebug() << "高风测试(终点) : " << roomID;
            }

            case 1:
                client->setTargetTemp(temp);
                if (wind == 0)
                {
                    client->setSpeed(Client::SpeedNone);
                    // 从队列中清除
                    if (highSpeedList.size())
                    {
                        hi = highSpeedList.indexOf(room);
                    }
                    if (hi != -1)
                    {
                        highSpeedList.removeAt(hi);
                    }

                    if (lowSpeedList.size())
                    {
                        li = lowSpeedList.indexOf(room);
                    }
                    if (li == -1)
                    {
                        lowSpeedList.removeAt(li);
                    }
                    client->writeDetailedList(room);
                }
                else if (wind == 1)
                {
                    client->setSpeed(Client::SpeedLow);
                    // 判断是否更新低风队列
                    if (highSpeedList.size())
                    {
                        hi = highSpeedList.indexOf(room);
                    }
                    if (hi != -1)
                    {
                        highSpeedList.removeAt(hi);
                    }

                    if (lowSpeedList.size())
                    {
                        li = lowSpeedList.indexOf(room);
                    }
                    if (li == -1)
                    {
                        lowSpeedList.append(room);
                    }
                }
                else if (wind == 2)
                {
                    client->setSpeed(Client::SpeedHigh);
                    // 判断是否更新高风队列
                    if (highSpeedList.size())
                    {
                        hi = highSpeedList.indexOf(room);
                    }
                    if (hi == -1)
                    {
                        highSpeedList.append(room);
                    }
                    if (lowSpeedList.size())
                    {
                        li = lowSpeedList.indexOf(room);
                    }
                    if (li != -1)
                    {
                        lowSpeedList.removeAt(li);
                    }
                }

                temp_t = QDateTime::currentDateTime();
                if (temp_t < client->getTime())
                {
                    client->setTime(temp_t);
                }
                qDebug() << DATETIME << "client.start_t : " << client->getTime().toString("yyyy-MM-dd hh:mm:ss");
                client->setWorking(Client::WorkingYes);
                break;

            default:
                break;
            }

//         sendRequestMessage(socket, type, 1);
//         client->setServing(Client::ServingYes);
//         ResourceAllocation();
        if(client->CheckServing())  // 分配成功
        {
            qDebug() << "give one resource for room--" << roomID;
            sendCommonMessage(socket, 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
        }
        else
        {
            qDebug() << "kill one resource for room--" << roomID;
            sendCommonMessage(socket, 1, 0, 0, 0, 0);
        }
//        else
//        {
//            // 不分配
//        }
//         qDebug() << DATETIME
//                  << " readFromSockets: "
//                  << type << " "
//                  << roomID << " "
//                  << dTemp << " "
//                  << usSwitch << " "
//                  << wind;
        }
        break;
    }

    ui->clientPanel->setWidget(clients, 4);
}


void MainWindow::resourceAllocation()
{
   // 先将全部房间服务置零
   for (int i = 0; i < clients.size(); i++)
   {
      qDebug() << "client at 置零 (起点): ___";
      Client *client = qobject_cast<Client *>(clients.at(i));
      qDebug() << "client at 置零 (终点): ___";
      client->setServing(Client::ServingNo);
   }

    int hlistSize = highSpeedList.size();
    int llistSize = lowSpeedList.size();
    int surplus   = RES_NUM - hlistSize;
    if (hlistSize <= RES_NUM)
    {
        // 高速风客户不需要轮转
        qDebug() << "调度高风（起点）: " << "hlistSize = " << hlistSize
        for (int i = 0; i < hlistSize; i++)
        {
            QString clientID = highSpeedList.at(i);
            Client  *client  = qobject_cast<Client *>(clients.at(clientIDs.indexOf(clientID)));
            client->setServing(Client::ServingYes);
        }
        qDebug() << "调度高风（终点）: " << "hlistSize = " << hlistSize;
    }
    else
    {
        // 高速风客户需要轮转
        roundRobin(Client::SpeedHigh, RES_NUM);
    }

    if (surplus <= 0)
    {
        // 高速风客户占用了全部资源
        return;
    }

    if (llistSize <= surplus)
    {
        // 低速风客户不需要轮转
        for (int i = 0; i < llistSize; i++)
        {
            QString clientID = lowSpeedList.at(i);
            Client  *client  = qobject_cast<Client *>(clients.at(clientIDs.indexOf(clientID)));
            client->setServing(Client::ServingYes);
        }
    }
    else
    {
        // 低速风客户需要轮转
        roundRobin(Client::SpeedLow, surplus);
    }
}


void MainWindow::rrIncrease()
{
    turn[0]++;
    //qDebug() << turn[0];
    if (turn[0] % RES_NUM == 0)
    {
        if (lowSpeedList.size() != 0)
        {
            turn[1] = (turn[1] + 1) % lowSpeedList.size();
        }
        if (highSpeedList.size() != 0)
        {
            turn[2] = (turn[2] + 1) % highSpeedList.size();
        }
    }
    // 清空关机的序列
    for (int i = 0; i < highSpeedList.size(); i++)
    {
        QString temp    = highSpeedList.at(i);
        Client  *client = qobject_cast<Client *>(clients.at(clientIDs.indexOf(temp)));
        if (!client->isWorking())
        {
            highSpeedList.removeAt(i);
        }
    }
    qDebug() << "关机高风（终点）-------------- ";
    qDebug() << "关机低风（起点）-------------- ";
    for (int i = 0; i < lowSpeedList.size(); i++)
    {
        QString temp    = lowSpeedList.at(i);
        Client  *client = qobject_cast<Client *>(clients.at(clientIDs.indexOf(temp)));
        if (!client->isWorking())
        {
            lowSpeedList.removeAt(i);
        }
    }
    resourceAllocation();
}


void MainWindow::roundRobin(Client::Speed speed, int resNum)            // 轮转
{
    int listSize;

    if (speed == Client::SpeedLow)
    {
        listSize = lowSpeedList.size();
    }
    else
    {
        listSize = highSpeedList.size();
    }
    // turn[0] 每5s增加一次
   if(level == 1)
   {
       for (int i = turn[level], j = 0; j < maxx; j++, i = (i + 1) % M)
       {
          QString temp = LowSpeedList.at(i);
    //        qDebug() << "资源分配：" << temp;
          qDebug() << "client at 分配 L起点): " << clientIDs.indexOf(temp);
          Client *client = qobject_cast<Client *>(clients.at(clientIDs.indexOf(temp)));
          client->setServing(Client::ServingYes);
          qDebug() << "client at 分配 L(终点): " << clientIDs.indexOf(temp);
       }
   }
   else
   {
       for (int i = turn[level], j = 0; j < maxx; j++, i = (i + 1) % M)
       {
          QString temp = HighSpeedList.at(i);
    //        qDebug() << "资源分配：" << temp;
          qDebug() << "client at 分配 H(起点): " << clientIDs.indexOf(temp);
          Client *client = qobject_cast<Client *>(clients.at(clientIDs.indexOf(temp)));
          client->setServing(Client::ServingYes);
          qDebug() << "client at 分配 H(终点): " << clientIDs.indexOf(temp);
       }
   }

}


// void MainWindow::sendRequestMessage(QTcpSocket *tsock, int type, int isServed)
// {
//    QJsonObject json;

//    json.insert("type", type);
//    json.insert("isServed", isServed);
// //   json.insert("cost",  cost);

//    QJsonDocument document;
//    document.setObject(json);
//    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

//    tsock->write(byte_array);

//    qDebug() << DATETIME << " sendRequestMessage: ";

// //   QByteArray  block;
// //   QDataStream out(&block, QIODevice::WriteOnly);

// //   out.setVersion(QDataStream::Qt_5_5);

// //   out << (quint16)0;
// //   out << type;
// //   out << isServed;
// //   out.device()->seek(0);
// //   out << (quint16)(block.size() - sizeof(quint16));

// //   tsock->write(block);

// //   qDebug() << DATETIME << " sendRequestMessage: ";
// }


void MainWindow::sendCommonMessage(QTcpSocket *tsock, int type, int usSwitch, double dTemp, int usWind, double cost)
{
    QJsonObject json;

    json.insert("type", type);
    json.insert("switch", usSwitch);
    json.insert("temperature", dTemp);
    json.insert("wind", usWind);
    json.insert("cost", cost);
    qDebug() << DATETIME << "sendCommonMessage: Type:" << type << " Switch:" << usSwitch << " Temp:" << dTemp << " Wind:" << usWind << " cost:" << cost;
    QJsonDocument document;
    document.setObject(json);

    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    tsock->write(byte_array);


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
