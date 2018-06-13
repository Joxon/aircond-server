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
    initNetwork();
}


MainWindow::~MainWindow()
{
    delete ui;
    delete aniOpacityChange;
    delete aniSizeChange;
    delete effOpacity;

    delete server;
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
    QSqlQuery query;
    QString   built_sql = ("CREATE TABLE IF NOT EXISTS Info_list ("
                            "id     INT      PRIMARY KEY,"
                            "roomid CHAR     NOT NULL,"
                            "time   DATETIME NOT NULL,"
                            "wind   INT      NOT NULL,"
                            "currt  DOUBLE   NOT NULL,"
                            "targt  DOUBLE   NOT NULL,"
                            "option INT      NOT NULL,"
                            "cost_p DOUBLE   NOT NULL,"
                            "cost_e DOUBLE   NOT NULL );");

    qDebug() << "built sql : " << built_sql;

    if (!query.exec(built_sql))
    {
        qDebug() << DATETIME << "bulit the detail list failed:" << query.lastError();
    }
    else
    {
        qDebug() << DATETIME << "bulit the detail list success";
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

    turn[0] = 1;
    for(int i = 1; i < 4; i++)
    {
        turn[i] = 0;
        SpeedList[i].clear();
    }

    rrTimer = new QTimer(this);
    connect(rrTimer, SIGNAL(timeout()), this, SLOT(rrIncrease()));
    rrTimer->start(999);
}


void MainWindow::on_toolButtonPower_toggled(bool checked)
{
    if (checked)
    {
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
    while (server->hasPendingConnections())
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
                qDebug() << DATETIME << "receive message : ";
                qDebug() << "\t\t\t Type : " << type << "Room : " << room << " Sitch : " << switchh;
                qDebug() << "\t\t\t Temp : " << temp << "Wind : " << wind;
            }
            //  JSON解析错误
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
                client->setLastSpeed(Client::SpeedNone);
                client->setId(room);
                client->setSpeed(0);
                client->setStartTime();
                client->setCost(0);
                client->setCurrentTemp(28);
                client->setSocket(socket);
                client->writeDetailedList(3);
                clients.append(client);
            }
            //通告报文
            if (type == 1)
            {
                if (client->isServing())
                {
                    client->calCost();
                }
                client->setCurrentTemp(temp);
                if(client->isTarget())
                {
                    client->setCurrentTemp(client->getTargetTemp());
                    client->writeDetailedList(1);   // reach target
                }

//                qDebug() << DATETIME << " readFromSockets: " << type << " " << room << " " << temp;
                if(client->isTarget())
                {
                    // room reach the target
                    // set wind = 0 and lastwind update
                    qDebug() << "speedlevel = " << (int)client->getSpeed();
                    client->setLastSpeed(client->getSpeed());
                    client->setSpeed(0);
                    sendCommonMessage(socket, 1, 1, client->getTargetTemp(), 0, client->getCost());
                    // remove the speedlist;
                    int lsize = -1;
                    for(int i = 1; i < 4; i++)
                    {
                        if (SpeedList[i].size())
                        {
                            lsize = SpeedList[i].indexOf(room);
                        }
                        if (lsize != -1)
                        {
                            SpeedList[i].removeAt(lsize);
                        }
                    }
                }
                else if(client->isServing())
                {
                    qDebug() << "give a resource to room " << room;
                    sendCommonMessage(socket, 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
                    qDebug() << DATETIME << "send message when receive common: ";
                    qDebug() << "\t\t\t Type : " << type << "Room : " << room << " Sitch : " << switchh;
                    qDebug() << "\t\t\t Temp : " << temp << "Wind : " << wind;
                }
                else if(client->isBackTemp())
                {
                    // take it to waiting list
                    int speedLevel = client->getLastSpeed();
//                    qDebug() << "speedlevel = " << speedLevel;
                    if(speedLevel)
                    {
                        client->setSpeed(speedLevel);
                        SpeedList[speedLevel].append(room);
                    }
                }
            }
            //请求报文
            else if (type == 0)
            {
                // write detail list
                int i = 1, si = -1;
                QDateTime temp_t;
                switch (switchh)
                {
                case 0:
                    // remove from speedlist
                    for(i = 1; i < 4; i++)
                    {
                        if (SpeedList[i].size())
                        {
                            si = SpeedList[i].indexOf(room);
                        }
                        if (si != -1)
                        {
                            SpeedList[i].removeAt(si);
                        }
                    }
                    client->setWorking(Client::WorkingNo);
                    client->writeDetailedList(4);
                    break;

                case 1:
                    client->setTargetTemp(temp);
                    client->setTempState();
                    client->setSpeed(wind);
                    client->writeDetailedList(2);
                    if (wind == 0)
                    {
                        // remove from speedlist
                        for(i = 1; i < 4; i++)
                        {
                            if (SpeedList[i].size())
                            {
                                si = SpeedList[i].indexOf(room);
                            }
                            if (si != -1)
                            {
                                SpeedList[i].removeAt(si);
                            }
                        }
//                        client->writeDetailedList(room);
                    }
                    else if(wind < 4)
                    {
                        // update the speed list
                        for(i = 1; i < 4; i++)
                        {
                            si = -1;
                            if(i == wind)
                            {
                                if (SpeedList[i].size())
                                {
                                    si = SpeedList[i].indexOf(room);
                                }
                                if (si == -1)
                                {
                                    SpeedList[i].append(room);
                                }
                            }
                            else
                            {
                                if (SpeedList[i].size())
                                {
                                    si = SpeedList[i].indexOf(room);
                                }
                                if (si != -1)
                                {
                                    SpeedList[i].removeAt(si);
                                }
                            }
//                            qDebug() << "SpeedListSize : " << SpeedList[i].size() << "when level = " << i;
                        }
                    }

//                    temp_t = QDateTime::currentDateTime();
//                    if (temp_t < client->getTime())
//                    {
//                        client->setTime(temp_t);
//                    }

//                    qDebug() << DATETIME << "client.start_t : " << client->getTime().toString("yyyy-MM-dd hh:mm:ss");

                    client->setWorking(Client::WorkingYes);
                    break;

                default:
                    break;
                }

                if(client->isTarget())
                {
                    // room reach the target
                    // set wind = 0 and lastwind update
                    qDebug() << "speedlevel = " << (int)client->getSpeed();
                    client->setLastSpeed(client->getSpeed());
                    client->setSpeed(0);
                    sendCommonMessage(socket, 1, 1, client->getTargetTemp(), 0, client->getCost());
                    // remove the speedlist;
                    int lsize = -1;
                    for(int i = 1; i < 4; i++)
                    {
                        if (SpeedList[i].size())
                        {
                            lsize = SpeedList[i].indexOf(room);
                        }
                        if (lsize != -1)
                        {
                            SpeedList[i].removeAt(lsize);
                        }
                    }
                }
                else if(client->isServing())
                {
                    qDebug() << "give a resource to room " << room;
                    sendCommonMessage(socket, 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
                    qDebug() << DATETIME << "send message when receive request: ";
                    qDebug() << "\t\t\t Type : " << type << "Room : " << room << " Sitch : " << switchh;
                    qDebug() << "\t\t\t Temp : " << temp << "Wind : " << wind;
                }
                else if(client->isBackTemp())
                {
                    // take it to waiting list
                    int speedLevel = client->getLastSpeed();
                    if(speedLevel)
                    {
                        client->setSpeed(speedLevel);
                        SpeedList[speedLevel].append(room);
                    }
                }

//                if (client->isServing())     // 分配成功
//                {
//                    qDebug() << "give one resource for room--" << room;
//                    sendCommonMessage(socket, 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
//                }
//                else
//                {
//                    if (client->isTarget())
//                    {       // 达到目标温度
//                        qDebug() << "Room reach the target room--" << room;
//                        sendCommonMessage(socket, 1, 0, 0, 0, 0);
//                    }
//                    else
//                    {       // 未达到目标温度
//                        qDebug() << "kill one resource for room--" << room;
//                        sendCommonMessage(socket, 1, 0, 0, -1, 0);
//                    }
//                }
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
            client->setSpeed(0);
            client->writeDetailedList(5);
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
        Client *client = qobject_cast<Client *>(clients.at(i));
        last_serving[i] = false;
        if (client->isServing())
        {
            last_serving[i] = true;
        }
        client->setServing(Client::ServingNo);
    }

    int listSize = 0, resSize = RES_NUM;
    for(int i = 3; i > 0; i--)
    {
        if(resSize <= 0)    break;          // there is no resource.
        listSize = SpeedList[i].size();
//        qDebug() << "listSize : " << listSize << "when level = " << i;
        if(listSize <= resSize)
        {   // 不需要轮转 直接分配
            for(int j = 0; j < listSize; j++)
            {
                QString clientID = SpeedList[i].at(j);
                Client  *client;
                for (int ii = 0; ii < clients.size(); ++ii)
                {
                    client = qobject_cast<Client *>(clients[ii]);
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
            roundRobin(i, resSize);
        }
        resSize -= listSize;
    }
}


void MainWindow::rrIncrease()
{
    turn[0]++;
    //qDebug() << turn[0];
    if (turn[0] % 120 == 0)                     // RR
    {
        for(int i = 1; i < 4; i++)
        {
            if (SpeedList[i].size() != 0)
            {
                turn[i] = (turn[i] + 1) % SpeedList[i].size();
            }
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
                // room reach the target
                // set wind = 0 and lastwind update
                client->setLastSpeed(client->getSpeed());
                client->setSpeed(0);
                sendCommonMessage(clientSockets[client->getId()], 1, 1, client->getTargetTemp(), 0, client->getCost());
                // remove the speedlist;
                int lsize = -1;
                for(int i = 1; i < 4; i++)
                {
                    if (SpeedList[i].size())
                    {
                        lsize = SpeedList[i].indexOf(client->getId());
                    }
                    if (lsize != -1)
                    {
                        SpeedList[i].removeAt(lsize);
                    }
                }
            }
            else
            {   // 未达到目标温度
                sendCommonMessage(clientSockets[client->getId()], 1, 1, client->getTargetTemp(), client->getSpeed(), client->getCost());
            }
        }
        if (client->isServing())
        {
            sendCommonMessage(clientSockets[client->getId()], 1, 1, client->getCurrentTemp(), (int)client->getSpeed(), client->getCost());
        }
    }
}


void MainWindow::roundRobin(int speed, int resNum)            // 轮转
{
    int listSize = SpeedList[speed].size();
    for(int i = turn[speed], j = 0; j < resNum; j++, i = (i+1)%listSize)
    {
        QString clientID = SpeedList[speed].at(i);
        Client  *client;
        for (int ii = 0; ii < clients.size(); ++ii)
        {
            client = qobject_cast<Client *>(clients[ii]);
            if (client->getId() == clientID)
            {
                break;
            }
        }
        client->setServing(Client::ServingYes);
    }
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
//    qDebug() << DATETIME << "sendCommonMessage: Type:" << type
//             << " Switch:" << switchh
//             << " Temp:" << temp
//             << " Wind:" << wind
//             << " cost:" << cost;

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
