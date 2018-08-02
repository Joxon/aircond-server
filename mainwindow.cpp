#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quiwidget.h"
#include "IconsFontAwesome5.h"
#include "detaillist.h"

#define SERVER_PORT    6666

bool scmp(Client *a, Client *b)
{
    if ((int)a->getSpeed() == (int)b->getSpeed())
    {
        return a->getTimer() > b->getTimer();
    }
    return (int)a->getSpeed() < (int)b->getSpeed();
}


bool wcmp(Client *a, Client *b)
{
    if ((int)a->getSpeed() == (int)b->getSpeed())
    {
        return a->getTimer() < b->getTimer();
    }
    return (int)a->getSpeed() > (int)b->getSpeed();
}


MainWindow::MainWindow(QWidget *parent) :
    parent(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QUIWidget::setFormInCenter(this);

    initDatabase();
    initFont();
    initUpdater();
    initAnimation();
    initClientPanel();
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
    delete updateTimer;
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
    //"房间号" << "操作时间" << "风速" << "目标温度" << "当前温度" << "操作" << "当前价格消费" << "当前能量消费";
    QString built_sql = ("CREATE TABLE IF NOT EXISTS Info_list ("
                         "id     INT      PRIMARY KEY,"
                         "roomid CHAR     NOT NULL,"
                         "time   DATETIME NOT NULL,"
                         "wind   INT      NOT NULL,"
                         "currt  DOUBLE   NOT NULL,"
                         "targt  DOUBLE   NOT NULL,"
                         "option INT      NOT NULL,"
                         "cost_p DOUBLE   NOT NULL,"
                         "cost_e DOUBLE   NOT NULL );");

    //qDebug() << "built sql : " << built_sql;

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
    fontAwesomeSolid.setPixelSize(20);

    ui->toolButtonPower->setFont(fontAwesomeSolid);
    ui->toolButtonPower->setText(QChar(ICON_FA_POWER_OFF));

    ui->labelArrowDown->setFont(fontAwesomeSolid);
    ui->labelArrowDown->setText(QChar(ICON_FA_ANGLE_DOUBLE_DOWN));
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


void MainWindow::initUpdater()
{
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, [this]() {
        ui->labelSysTime->setText(QString("系统时间：") + DATETIME);
        if (clients.isEmpty())
        {
            ui->labelConnState->setText(QString("等待传入连接..."));
        }
        else
        {
            ui->labelConnState->setText(QString("已连接客户端：%1个").arg(clients.size()));
        }
    });
    updateTimer->start(1000);
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
    if (!server->listen(QHostAddress::Any, SERVER_PORT))
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
//                qDebug() << DATETIME << "receive message : ";
//                qDebug() << "\t\t\t Type : " << type << "Room : " << room << " Sitch : " << switchh;
//                qDebug() << "\t\t\t Temp : " << temp << "Wind : " << wind;
            }
            //  JSON解析错误
            else
            {
                qDebug() << DATETIME << jsonErr.errorString();
                return;
            }

            //搜索客户端列表
            int clientIdx  = 0;
            Client *client = nullptr;
            // 旧的ID，更新client
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
                client->setMaximumWidth(300);

                client->setConn(Client::ConnOnline);
                client->setLastSpeed(Client::SpeedNone);
                client->setId(room);
                client->setSpeed(0);
                client->setStartTime();
                client->setCost(0);
                client->setEnergy(0);
                client->setCurrentTemp(28);
                client->setSocket(socket);
                client->setTargetTemp(temp);

                client->setTimer(1);                        // 设置等待时间60
                client->setWarmingUp(false);
                addIntoWaitingQueue(client);                // 加入等待队列
                client->writeDetailedList(3);               // 开机的一个详单写入

                clients.append(client);
            }
            //通告报文
            if (type == 1)
            {
                sort(waitingQueue, waitingQueue + wSize, wcmp);
                sort(servingQueue, servingQueue + sSize, scmp);
                if (!client->isWarmingUp() && isInServingQueue(room))
                {
                    client->calCost(temp);
                }
                client->setCurrentTemp(temp);
                if (client->getSpeed() != Client::SpeedNone)
                {
                    client->setLastSpeed(client->getSpeed());
                }
                if (isInServingQueue(room))      // 在服务队列
                {
                    if (client->isWarmingUp())
                    {     // 处于回温状态
                        if (client->warmingUpCheck())
                        { // 仍在回温
                            client->changeTimer(0);
                        }
                        else
                        {   // 开启空调
                            client->setWarmingUp(false);
                            client->changeTimer(0);
                        }
                    }
                    else if (client->isTarget())
                    {   // 到达目标温度 移出服务队列 加入等待队列 进入回温状态
//                        removeFromServingQueue(client);
//                        addIntoWaitingQueue(client);
//                        client->setWarmingUp(true);
//                        client->setTimer(1);    // 设置计时器
//                        // 从等待队列中加入一个
//                        // 排序服务队列
//                        Client * tempC = waitingQueue[0];
//                        addIntoServingQueue(tempC);
//                        removeFromWaitingQueue(tempC);

                        // 进入回温状态
                        client->setWarmingUp(true);
                        client->changeTimer(0);
                    }
                    else
                    {
                        client->changeTimer(0); // 服务计时器 + 1
                    }
                }
                else                                                       // 在等待队列
                {
                    client->changeTimer(1);                                // 等待计时器 - 1
                    if (client->isWarmingUp() && client->warmingUpCheck()) // 回温
                    {
                        if (client->getTimer() <= 0)
                        {
                            client->setTimer(1);    // 未达到回温标准，重置计时器
                        }
                    }
                    else if ((client->getSpeed() == Client::SpeedNone) && client->isWarmingUp())
                    {
                        client->setSpeed(client->getLastSpeed());
                        if (client->getSpeed() == Client::SpeedHigh) // 大风
                        {
                            if (canSeize())
                            {   // 可以抢占
                                if (sSize < RES_NUM)
                                {
                                    removeFromWaitingQueue(client);
                                    addIntoServingQueue(client);
                                }
                                else
                                {
                                    waitingIntoServing(client);
                                    qDebug() << "here is first";
                                }
                            }
                            else
                            {
                                // do nothing
                            }
                        }
                        else
                        {
                            // do nothing
                        }
                    }
                    else if (client->getTimer() <= 0) // 计时器到了
                    {
                                                      // 替换服务队列中优先级最低的
                                                      // 服务队列排序
                        sort(servingQueue, servingQueue + sSize, scmp);
                        if (client->isWarmingUp())    // 回温的空调进入服务队列 删除回温标记
                        {
                            client->setWarmingUp(false);
                        }
                        waitingIntoServing(client);
                        qDebug() << "here is second";
                    }
                }

                if (isInServingQueue(room))
                {
                    client->setServing(Client::ServingYes);
                    if (client->isWarmingUp())
                    {
                        sendCommonMessage(socket, 1, 1, client->getTargetTemp(), 0, client->getCost());
                    }
                    else
                    {
                        sendCommonMessage(socket, 1, 1, client->getTargetTemp(), (int)client->getSpeed(), client->getCost());
                    }
                }
                else
                {
                    client->setServing(Client::ServingNo);
                    sendCommonMessage(socket, 1, 1, client->getTargetTemp(), 0, client->getCost());
                }

                qDebug() << DATETIME << "Now is room--" << room;
                qDebug() << "servingQueue:";
                for (int i = 0; i < sSize; i++)
                {
                    qDebug() << "i = " << i << "room -- " << servingQueue[i]->getId();
                }
                qDebug() << "waitingQueue:";
                for (int i = 0; i < wSize; i++)
                {
                    qDebug() << "i = " << i << "room -- " << waitingQueue[i]->getId();
                }
            }
            //请求报文
            else if (type == 0)
            {
                // write detail list

                int lastWind = 0;
                switch (switchh)
                {
                case 0:
                    // remove from queue
                    if (isInServingQueue(room))
                    {   // 在服务队列里
                        removeFromServingQueue(client);
                    }
                    else
                    {   // 在等待队列里
                        removeFromWaitingQueue(client);
                    }
                    client->setWorking(Client::WorkingNo);
                    client->writeDetailedList(4);               // 写关机详单
                    break;

                case 1:

                    client->setTargetTemp(temp);

                    if (!client->isWorking())
                    {
                        client->writeDetailedList(3); // 写开机详单
                        if (wind == 3)
                        {                             // 判断是否可以抢占
                            if (canSeize())
                            {
                                if (sSize < RES_NUM)
                                {
                                    removeFromWaitingQueue(client);
                                    addIntoServingQueue(client);
                                }
                                else
                                {
                                    bootIntoServing(client);
                                    qDebug() << "here is boot";
                                }
                            }
                            else
                            {
                                // do nothing
                            }
                        }
                        else
                        {
                            if (sSize < RES_NUM)
                            {   // 资源足够
                                removeFromWaitingQueue(client);
                                addIntoServingQueue(client);
                                qDebug() << "here is third";
                            }
                            else
                            {
                                // 加入等待队列
                                addIntoWaitingQueue(client);
                                client->setTimer(1);
                            }
                        }
                    }
                    else
                    {
                        if (client->isWarmingUp())   // 中断回温
                        {
                            client->setWarmingUp(false);
                        }
                        else
                        {
                            // do nothing
                        }
                        lastWind = client->getLastSpeed();
                        if (lastWind == 0)
                        {     // 初次启动
                            if (sSize < RES_NUM)
                            { // 资源足够
                                removeFromWaitingQueue(client);
                                addIntoServingQueue(client);
                                qDebug() << "here is third";
                            }
                        }
                        else if (lastWind < wind)
                        {   // 调高风
                            if (isInServingQueue(room))
                            {
                                // do nothing
                            }
                            else    // 在等待队列里
                            {
                                if (wind == 3)
                                {
                                    // 判断是否可以抢占
                                    if (canSeize())
                                    {
                                        if (sSize < RES_NUM)
                                        {
                                            removeFromWaitingQueue(client);
                                            addIntoServingQueue(client);
                                        }
                                        else
                                        {
                                            waitingIntoServing(client);
                                            qDebug() << "here is fourth";
                                        }
                                    }
                                    else
                                    {
                                        // do nothing
                                    }
                                }
                                else
                                {
                                    if (sSize < RES_NUM)
                                    {   // 资源足够
                                        removeFromWaitingQueue(client);
                                        addIntoServingQueue(client);
                                        qDebug() << "here is fifth";
                                    }
                                }
                            }
                        }
                        else if (lastWind > wind)
                        {   // 调低风
                            if (isInServingQueue(room))
                            {
                                if (lastWind == 3)
                                {
                                    // 查看等待队列里有没有可以抢占的
                                    if (mayBeSeize())
                                    {
                                        servingIntoWaiting(client);
                                        qDebug() << "here is sixth";
                                    }
                                }
                                else
                                {
                                    // do nothing
                                }
                            }
                            else    // 在等待队列
                            {
                                if (sSize < RES_NUM)
                                {   // 资源足够
                                    removeFromWaitingQueue(client);
                                    addIntoServingQueue(client);
                                    qDebug() << "here is seventh";
                                }
                            }
                        }
                    }
                    client->setSpeed(wind);
//                    client->setLastSpeed(client->getSpeed());
                    client->setWorking(Client::WorkingYes);
                    client->writeDetailedList(2);               // 写请求详单
                    break;

                default:
                    break;
                }
            }

            ui->clientPanel->setWidget(clients, 4);
        });

        connect(socket, &QTcpSocket::disconnected, [socket, this]() {
            int clientIdx  = 0;
            Client *client = nullptr;
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
            client->setEnergy(0);
            client->setCost(0);

            if (isInServingQueue(client->getId()))
            {   // 在服务队列里
                removeFromServingQueue(client);
            }
            else
            {   // 在等待队列里
                removeFromWaitingQueue(client);
            }
        });
    }
}


void MainWindow::addIntoWaitingQueue(Client *tempR)
{
    if (isInWaitingQueue(tempR))
    {
        return;
    }
    waitingQueue[wSize++] = tempR;
    qDebug() << "here is addIntoWaitingQueue";
    qDebug() << "Now room is " << tempR->getId();
    qDebug() << "servingQueue:";
    for (int i = 0; i < sSize; i++)
    {
        qDebug() << "i = " << i << "room -- " << servingQueue[i]->getId();
    }
    qDebug() << "waitingQueue:";
    for (int i = 0; i < wSize; i++)
    {
        qDebug() << "i = " << i << "room -- " << waitingQueue[i]->getId();
    }
}


void MainWindow::removeFromWaitingQueue(Client *tempR)
{
    int i = 0;

    for ( ; i < wSize; i++)
    {
        if (waitingQueue[i]->getId() == tempR->getId())
        {
            break;
        }
    }
    for ( ; i < wSize; i++)
    {
        waitingQueue[i] = waitingQueue[i + 1];
    }
    wSize--;
}


void MainWindow::addIntoServingQueue(Client *tempR)
{
    servingQueue[sSize++] = tempR;
    tempR->writeDetailedList(7);
}


void MainWindow::removeFromServingQueue(Client *tempR)
{
    int i = 0;

    for ( ; i < sSize; i++)
    {
        if (servingQueue[i]->getId() == tempR->getId())
        {
            break;
        }
    }
    for ( ; i < sSize; i++)
    {
        servingQueue[i] = servingQueue[i + 1];
    }
    sSize--;
}


bool MainWindow::isInServingQueue(QString roomId)
{
    for (int i = 0; i < sSize; i++)
    {
        if (servingQueue[i]->getId() == roomId)
        {
            return true;
        }
    }
    return false;
}


bool MainWindow::isInWaitingQueue(Client *tempR)
{
    for (int i = 0; i < wSize; i++)
    {
        if (waitingQueue[i]->getId() == tempR->getId())
        {
            return true;
        }
    }
    return false;
}


bool MainWindow::canSeize()     // 只为高风准备
{
    if (sSize < RES_NUM)        // 留有资源
    {
        return true;
    }
    else                        // 资源已满 判断抢占
    {
        // 排序服务队列   优先级 低->高
        sort(servingQueue, servingQueue + sSize, scmp);
        if (servingQueue[0]->getSpeed() != Client::SpeedHigh)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}


bool MainWindow::mayBeSeize()
{
    if (wSize > 0)
    {
        // 排序等待队列 优先级 高->低
        sort(waitingQueue, waitingQueue + wSize, wcmp);
        if (waitingQueue[0]->getSpeed() != Client::SpeedHigh)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}


void MainWindow::servingIntoWaiting(Client *tempR)
{
    for (int i = 0; i < sSize; i++)
    {
        if (servingQueue[i]->getId() == tempR->getId())
        {
            servingQueue[i] = waitingQueue[0];
            break;
        }
    }
    waitingQueue[0]->writeDetailedList(7);          // 赋予资源
    waitingQueue[0]->setTimer(0);
    waitingQueue[0] = tempR;
    waitingQueue[0]->setTimer(1);
    waitingQueue[0]->writeDetailedList(6);          // 剥夺资源
}


void MainWindow::waitingIntoServing(Client *tempR)
{
    for (int i = 0; i < wSize; i++)
    {
        if (waitingQueue[i]->getId() == tempR->getId())
        {
            waitingQueue[i] = servingQueue[0];
            break;
        }
    }
    servingQueue[0]->writeDetailedList(6);          // 剥夺资源
    servingQueue[0]->setTimer(1);
    servingQueue[0] = tempR;
    servingQueue[0]->setTimer(0);
    servingQueue[0]->writeDetailedList(7);          // 赋予资源

//    qDebug() << "here is waitingIntoServing";
//    qDebug() << "Now room is " << tempR->getId();
//    qDebug() << "servingQueue:" ;
//    for(int i = 0; i < sSize; i++)
//    {
//        qDebug() << "i = " << i << "room -- " << servingQueue[i]->getId();
//    }
//    qDebug() << "waitingQueue:";
//    for(int i = 0; i < wSize; i++)
//    {
//        qDebug() << "i = " << i << "room -- " << waitingQueue[i]->getId();
//    }
}


void MainWindow::bootIntoServing(Client *tempR)
{
    addIntoWaitingQueue(servingQueue[0]);
    servingQueue[0]->writeDetailedList(6);          // 剥夺资源
    servingQueue[0]->setTimer(1);
    servingQueue[0] = tempR;
    servingQueue[0]->setTimer(0);
    servingQueue[0]->writeDetailedList(7);          // 赋予资源

//    qDebug() << "here is bootIntoServing";
//    qDebug() << "Now room is " << tempR->getId();
//    qDebug() << "servingQueue:" ;
//    for(int i = 0; i < sSize; i++)
//    {
//        qDebug() << "i = " << i << "room -- " << servingQueue[i]->getId();
//    }
//    qDebug() << "waitingQueue:";
//    for(int i = 0; i < wSize; i++)
//    {
//        qDebug() << "i = " << i << "room -- " << waitingQueue[i]->getId();
//    }
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

    Client *client = nullptr;
    for (int i = 0; i < clients.size(); ++i)
    {
        client = qobject_cast<Client *>(clients[i]);
        if (client->getSocket() == tsock)
        {
            break;
        }
    }

//    qDebug() << DATETIME << "sendCommonMessage: Type:" << type
//             << "Room : " << client->getId() << " Switch:" << switchh
//             << "\n\t\t Temp:" << temp << " Wind:" << wind
//             << " cost:" << cost;

//    if(wind != client->getSpeed())
//    {
//        if(wind > 0)
//            client->writeDetailedList(6);
//        else
//            client->writeDetailedList(7);
//    }

    QJsonDocument document;
    document.setObject(json);

    QByteArray bytes = document.toJson(QJsonDocument::Compact);
    tsock->flush();
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
