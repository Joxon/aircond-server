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
        qDebug() << DATETIME << "initDatabase:" << database.lastError();
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


void MainWindow::on_toolButtonPower_toggled(bool checked)
{
    if (checked)
    {
        initNetwork();

        ui->labelArrowDown->hide();
        //aniSizeChange->setStartValue(parent->geometry());
        aniSizeChange->setEndValue(QRect(parent->x(), parent->y() - 200, 1000, 600));
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
        aniSizeChange->setEndValue(QRect(parent->x(), parent->y() + 200, 1000, 200));
        aniSizeChange->start();

        aniOpacityChange->setStartValue(1.0);
        aniOpacityChange->setEndValue(0.1);
        aniOpacityChange->start();
    }
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


void MainWindow::storeSockets()
{
    if (server->hasPendingConnections())
    {
        QTcpSocket *socket = server->nextPendingConnection();

        connect(socket, &QTcpSocket::readyRead, [socket, this]() {
            if (!socket->isValid() ||
                (socket->bytesAvailable() < (sizeof(quint16))))
            {
                return;
            }

            //JSON解析
            int type             = -1;
            int switchh          = -1;
            int wind             = -1;
            QString room         = "";
            double temp          = -1.0;
            QByteArray jsonBytes = socket->readLine();
            QJsonParseError jsonErr;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonBytes, &jsonErr);
            //JSON解析正确
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
                        if (room == "")
                        {
                            return;
                        }
                        if (!clientSockets.contains(room))
                        {
                            clientSockets[room] = socket;
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
            //JSON解析错误
            else
            {
                qDebug() << DATETIME << jsonErr.errorString();
                return;
            }

            //搜索客户端列表
            int clientIdx = 0;
            Client *client;
            //旧的ID，更新client
            while (clientIdx < clients.size())
            {
                client = qobject_cast<Client *>(clients[clientIdx]);
                if (client->getId() == room)
                {
                    client->setConn(Client::ConnOnline);
                    break;
                }
                else
                {
                    ++clientIdx;
                }
            }
            //新的ID，新增client
            if (clientIdx >= clients.size())
            {
                client = new Client;
                client->setFixedHeight(150);
                client->setMaximumWidth(250);

                client->setConn(Client::ConnOnline);
                client->setId(room);
                client->setStartTime();
                client->setCost(0);
                client->setCurrentTemp(28);
                client->setSocket(socket);

                clients.append(client);
            }

            //通告报文
            if (type == 1)
            {
                if (client->isServing())
                {
                    client->calCost(temp);
                }
                client->setCurrentTemp(temp);
                //         sendCommonMessage(socket, 1, 1,
                //                           client->getCurrentTemp(),
                //                           (int)client->getSpeed(),
                //                           client->getCost());

                qDebug() << DATETIME << " readFromSockets: " << type << " " << room << " " << temp;
                if (client->isServing())     // 分配成功
                {
                    qDebug() << "give one resource for room--" << room;
                    sendCommonMessage(socket, 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
                    qDebug() << "Send success!";
                }
                else
                {
                    if (client->isTarget())
                    {       // 达到目标温度
                        qDebug() << "Room reach the target room--" << room;
                        sendCommonMessage(socket, 1, 0, 0, 0, 0);
                    }
                    else
                    {       // 未达到目标温度
                        qDebug() << "kill one resource for room--" << room;
                        sendCommonMessage(socket, 1, 0, 0, -1, 0);
                    }
                }
            }
            //请求报文
            else if (type == 0)
            {
                int hi = -1, li = -1;
                QDateTime temp_t;
                switch (switchh)
                {
                case 0:     // 关机 此时存储一次账单
                    client->setWorking(Client::WorkingNo);
                    client->writeDetailedList(room);
                    break;

                case 1:
                    client->setTargetTemp(temp);
                    if (wind == 0)
                    {
                        client->setSpeed(Client::SpeedNone);
                        // 从队列中清除
                        //qDebug() << "无风测试(起点) : " << room;
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
                        if (li != -1)
                        {
                            lowSpeedList.removeAt(li);
                            //qDebug() << "删除成功！ --------------------------------------";
                        }
                        client->writeDetailedList(room);
                        //qDebug() << "无风测试(终点) : " << room;
                    }
                    else if (wind == 1)
                    {
                        client->setSpeed(Client::SpeedLow);
                        // 判断是否更新低风队列
                        qDebug() << "低风测试(起点) : " << room;
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
                        qDebug() << "低风测试(终点) : " << room;
                    }
                    else if (wind == 2)
                    {
                        client->setSpeed(Client::SpeedHigh);
                        // 判断是否更新高风队列
                        qDebug() << "高风测试(起点) : " << room;
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
                        qDebug() << "高风测试(终点) : " << room;
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
                if (client->isServing())     // 分配成功
                {
                    qDebug() << "give one resource for room--" << room;
                    sendCommonMessage(socket, 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
                }
                else
                {
                    if (client->isTarget())
                    {       // 达到目标温度
                        qDebug() << "Room reach the target room--" << room;
                        sendCommonMessage(socket, 1, 0, 0, 0, 0);
                    }
                    else
                    {       // 未达到目标温度
                        qDebug() << "kill one resource for room--" << room;
                        sendCommonMessage(socket, 1, 0, 0, -1, 0);
                    }
                }
                //        else
                //        {
                //            // 不分配
                //        }
                //         qDebug() << DATETIME
                //                  << " readFromSockets: "
                //                  << type << " "
                //                  << room << " "
                //                  << temp << " "
                //                  << switchh << " "
                //                  << wind;
            }

            ui->clientPanel->setWidget(clients, 4);
        });

        connect(socket, &QTcpSocket::disconnected, [socket, this]() {
            int clientIdx = 0;
            Client *client;
            while (clientIdx < clients.size())
            {
                client = qobject_cast<Client *>(clients[clientIdx]);
                if (client->getSocket() == socket)
                {
                    break;
                }
                else
                {
                    ++clientIdx;
                }
            }
            if (clientIdx >= clients.size())
            {
                return;
            }
            client->setConn(Client::ConnOffline);
            client->setWorking(Client::WorkingNo);
            client->setServing(Client::ServingNo);
            client->setCurrentTemp(0.0);
            client->setTargetTemp(0.0);
            client->setSpeed(Client::SpeedNone);
            // 再算一次钱或者不算
        });
    }
}


void MainWindow::readFromSockets()
{
}


void MainWindow::resourceAllocation()
{
    // 先将全部房间服务置零
    for (int i = 0; i < clients.size(); i++)
    {
//        qDebug() << "client at 置零 (起点): ___";
        Client *client = qobject_cast<Client *>(clients.at(i));
//        qDebug() << "client at 置零 (终点): ___";
        last_serving[i] = false;
        if (client->isServing())
        {
            last_serving[i] = true;
        }
        client->setServing(Client::ServingNo);
    }

    int hlistSize = highSpeedList.size();
    int llistSize = lowSpeedList.size();
    int surplus   = RES_NUM - hlistSize;
    if (hlistSize <= RES_NUM)
    {
        // 高速风客户不需要轮转
//        qDebug() << "调度高风（起点）: " << "hlistSize = " << hlistSize;
        for (int i = 0; i < hlistSize; i++)
        {
            QString clientID = highSpeedList.at(i);
            Client  *client;
            for (int i = 0; i < clients.size(); ++i)
            {
                client = qobject_cast<Client *>(clients[i]);
                if (client->getId() == clientID)
                {
                    break;
                }
            }
            client->setServing(Client::ServingYes);
        }
//        qDebug() << "调度高风（终点）: " << "hlistSize = " << hlistSize;
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
            Client  *client;
            for (int i = 0; i < clients.size(); ++i)
            {
                client = qobject_cast<Client *>(clients[i]);
                if (client->getId() == clientID)
                {
                    break;
                }
            }
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
        QString clientID = highSpeedList.at(i);
        Client  *client;
        for (int i = 0; i < clients.size(); ++i)
        {
            client = qobject_cast<Client *>(clients[i]);
            if (client->getId() == clientID)
            {
                break;
            }
        }
        if (!client->isWorking())
        {
            highSpeedList.removeAt(i);
        }
    }
//    qDebug() << "关机高风（终点）-------------- ";
//    qDebug() << "关机低风（起点）-------------- ";
    for (int i = 0; i < lowSpeedList.size(); i++)
    {
        QString clientID = lowSpeedList.at(i);
        Client  *client;
        for (int i = 0; i < clients.size(); ++i)
        {
            client = qobject_cast<Client *>(clients[i]);
            if (client->getId() == clientID)
            {
                break;
            }
        }
        if (!client->isWorking())
        {
            lowSpeedList.removeAt(i);
        }
    }
    resourceAllocation();
    // send
    for (int i = 0; i < clients.size(); i++)
    {
        Client *client = qobject_cast<Client *>(clients.at(i));
        if (!client->isServing() && last_serving[i])
        {     // 剥夺资源
            if (client->isTarget())
            { // 达到目标温度
                sendCommonMessage(clientSockets[client->getId()], 1, 0, 0, 0, 0);
            }
            else
            {   // 未达到目标温度
                sendCommonMessage(clientSockets[client->getId()], 1, 0, 0, -1, 0);
            }
        }
        if (client->isServing())
        {
            sendCommonMessage(clientSockets[client->getId()], 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
        }
    }
}


void MainWindow::roundRobin(Client::Speed speed, int resNum)            // 轮转
{
    if (speed == Client::SpeedLow)
    {
        int listSize = lowSpeedList.size();
        for (int i = turn[(int)speed], j = 0; j < resNum; j++, i = (i + 1) % listSize)
        {
            QString clientID = lowSpeedList.at(i);
            Client  *client;
            for (int i = 0; i < clients.size(); ++i)
            {
                client = qobject_cast<Client *>(clients[i]);
                if (client->getId() == clientID)
                {
                    break;
                }
            }
            //        qDebug() << "资源分配：" << temp;
            //qDebug() << "client at 分配 L起点): " << clientIDs.indexOf(temp);
            client->setServing(Client::ServingYes);
            //qDebug() << "client at 分配 L(终点): " << clientIDs.indexOf(temp);
        }
    }
    else if (speed == Client::SpeedHigh)
    {
        int listSize = highSpeedList.size();
        for (int i = turn[(int)speed], j = 0; j < resNum; j++, i = (i + 1) % listSize)
        {
            QString clientID = highSpeedList.at(i);
            Client  *client;
            for (int i = 0; i < clients.size(); ++i)
            {
                client = qobject_cast<Client *>(clients[i]);
                if (client->getId() == clientID)
                {
                    break;
                }
            }
            //        qDebug() << "资源分配：" << temp;
            //qDebug() << "client at 分配 H(起点): " << clientIDs.indexOf(temp);
            client->setServing(Client::ServingYes);
            //qDebug() << "client at 分配 H(终点): " << clientIDs.indexOf(temp);
        }
    }
    // turn[0] 每5s增加一次
}


void MainWindow::sendRequestMessage(QTcpSocket *socket, int type, int isServed)
{
#ifdef USE_JSON
    QJsonObject json;
    json.insert("type", type);
    json.insert("isServed", isServed);
    //json.insert("cost",  cost);

    QJsonDocument document;
    document.setObject(json);

    QByteArray bytes = document.toJson(QJsonDocument::Compact);
    socket->write(bytes);

    qDebug() << DATETIME << " sendRequestMessage: ";
#else
    QByteArray  block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out << (quint16)0;
    out << type;
    out << isServed;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tsock->write(block);
    qDebug() << DATETIME << " sendRequestMessage: ";
#endif
}


void MainWindow::sendCommonMessage(QTcpSocket *tsock, int type, int switchh, double temp, int wind, double cost)
{
#ifdef USE_JSON
    QJsonObject json;
    json.insert("type", type);
    json.insert("switch", switchh);
    json.insert("temperature", temp);
    json.insert("wind", wind);
    json.insert("cost", cost);
    qDebug() << DATETIME << "sendCommonMessage: Type:" << type
             << " Switch:" << switchh
             << " Temp:" << temp
             << " Wind:" << wind
             << " cost:" << cost;

    QJsonDocument document;
    document.setObject(json);

    QByteArray bytes = document.toJson(QJsonDocument::Compact);
    tsock->write(bytes);
#else
    QByteArray  block;
    QDataStream out(&block, QIODevice::WriteOnly);

    //设置数据流的版本，客户端和服务器端使用的版本要相同
    out.setVersion(QDataStream::Qt_5_5);
    out << (quint16)0;
    out << QString("0");
    out << roomMap[room];
    out << type;
    out << Switch;
    out << temperature;
    out << wind;
    out << cost;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tsock->write(block);
    clientConnection->disconnectFromHost();
    //发送数据成功后，显示提示
    roomMap[room]++;
#endif
}
