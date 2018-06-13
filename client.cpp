#include "client.h"
#include "ui_client.h"
#include "detaillist.h"
#include "IconsFontAwesome5.h"

Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);

    //引入图形字体
    fontId   = QFontDatabase::addApplicationFont(":/image/Font-Awesome-5-Free-Solid-900.otf");
    fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
    font     = QFont(fontName);
    font.setPixelSize(15);

    ui->labelRoomIcon->setFont(font);
    ui->labelRoomIcon->setText((QChar)ICON_FA_SPINNER);

//   ui->labelConnIcon->setFont(font);
//   ui->labelConnIcon->setText((QChar)ICON_FA_TOGGLE_OFF);

    ui->labelCurrentTempIcon->setFont(font);
    ui->labelCurrentTempIcon->setText((QChar)ICON_FA_THERMOMETER_FULL);

    ui->labelTargetTempIcon->setFont(font);
    ui->labelTargetTempIcon->setText((QChar)ICON_FA_THERMOMETER_EMPTY);

    ui->labelSpeedIcon->setFont(font);
    ui->labelSpeedIcon->setText((QChar)ICON_FA_SNOWFLAKE);

    ui->toolButtonDetails->setFont(font);
    ui->toolButtonDetails->setText((QChar)ICON_FA_LIST);
}


Client::~Client()
{
    delete ui;
}


void Client::setId(const QString& i)
{
    id = i;
    ui->labelRoomName->setText(QString("房间 %1").arg(i));
}


void Client::setConn(Client::Conn c)
{
    conn = c;
    switch (c)
    {
    case Client::ConnOffline:
        ui->labelConn->setText("离线");
        break;

    case Client::ConnOnline:
        ui->labelConn->setText("在线");
        break;
    }
}


//void Client::setConn(Client::Conn conn)
//{
//   switch (conn)
//   {
//   case ConnOffline:
//      ui->labelConnIcon->setFont(font);
//      ui->labelConnIcon->setText((QChar)ICON_FA_TOGGLE_OFF);
//      ui->labelConn->setText(tr("离线"));
//      ui->frameDetails->setVisible(false);
//      ui->frameCost->setVisible(false);
//      break;

//   case ConnOnline:
//      ui->labelConnIcon->setFont(font);
//      ui->labelConnIcon->setText((QChar)ICON_FA_TOGGLE_ON);
//      ui->labelConn->setText(tr("在线"));
//      ui->frameDetails->setVisible(true);
//      ui->frameCost->setVisible(true);
//      break;

//   default:
//      break;
//   }
//}


void Client::setWorking(Client::Working w)
{
    working = w;
    switch (w)
    {
    case WorkingNo:
        ui->labelWorking->setText(tr("工作状态：否"));
        ui->labelTargetTemp->setText(tr("目标温度：N/A"));
        ui->labelSpeed->setText(tr("风速：N/A"));
        break;

    case WorkingYes:
        ui->labelWorking->setText(tr("工作状态：是"));
        break;

    case WorkingDone:
        ui->labelWorking->setText(tr("工作状态：完成"));
        ui->labelSpeed->setText(tr("风速：N/A"));
        break;
    }
}


void Client::setServing(Client::Serving s)
{
    serving = s;
    switch (s)
    {
    case ServingNo:
        ui->labelServing->setText(tr("服务状态：否"));
        break;

    case ServingYes:
        ui->labelServing->setText(tr("服务状态：是"));
        break;
    }
}


void Client::setCurrentTemp(double t)
{
    currentTemp = t;
    if (qFuzzyIsNull(t))
    {
        ui->labelCurrentTemp->setText("当前温度：N/A");
    }
    else
    {
        ui->labelCurrentTemp->setText(QString("当前温度：%1 ℃").arg(t));
    }
}


void Client::setTargetTemp(double t)
{
    targetTemp = t;
    if (qFuzzyIsNull(t))
    {
        ui->labelCurrentTemp->setText("目标温度：N/A");
    }
    else
    {
        ui->labelTargetTemp->setText(QString("目标温度：%1 ℃").arg(t));
    }
}

void Client::setLastSpeed(Client::Speed s)
{
    lastSpeed = s;
}

void Client::setSpeed(int s)
{
    switch (s)
    {
    case 0:
        speed = SpeedNone;
        ui->labelSpeed->setText(QString("风速：无"));
        break;

    case 1:
        speed = SpeedLow;
        ui->labelSpeed->setText(QString("风速：低"));
        break;

    case 2:
        speed = SpeedMid;
        ui->labelSpeed->setText(QString("风速：中"));
        break;

    case 3:
        speed = SpeedHigh;
        ui->labelSpeed->setText(QString("风速：高"));
        break;
    }
}


void Client::setEnergy(double e)
{
    energy = e;
    ui->labelEnergy->setText(QString("能量：%1 度").arg(e));
}


void Client::setCost(double c)
{
    cost = c;
    ui->labelCost->setText(QString("费用：%1 元").arg(c));
}


void Client::setStartTime()
{
    connStartTime = QDateTime::currentDateTime();
//    costStartTime = QDateTime::fromString("2999-12-31 23:59:59", "yyyy-MM-dd hh:mm:ss");
//    qDebug() << connStartTime;
//    qDebug() << costStartTime;
}


double Client::getCurrentTemp() const
{
    return currentTemp;
}


QString Client::getId()
{
    return id;
}

int Client::getLastSpeed()
{
    return (int)lastSpeed;
}

Client::Speed Client::getSpeed() const
{
    return speed;
}


double Client::getCost() const
{
    return cost;
}

double Client::getTargetTemp()
{
    return  targetTemp;
}

//QDateTime Client::getTime()                                     // 获得start_t;
//{
//    return costStartTime;
//}


//void Client::setTime(QDateTime time)                                          // 设置start_t;
//{
//    costStartTime = time;
//}


//void Client::Init_Room()
//{
//    Link = Work = Service = false;              // 未连接，未工作，未服务
//    Now = Goal = cost = energy = 0;             // 初始化为0000
//    Wind = 0;                                    // 风速是 0 1 2 对应三个挡位
//   start_t.fromString("9999-12-31 00:00:00", "yyyy-MM-dd hh:mm:ss");
//}


void Client::calCost(double new_n)                   // 为了计算需要1个周期计算一次，不然需要不断获取上一次的温度风速等信息
{
    //cost += wind * S;             S == 1;
    double wind = 0;

    switch (SpeedLow)
    {
    case SpeedNone:
        wind = 0;
        break;

    case SpeedLow:
        wind = 1.0;
        break;

    case SpeedMid:
        wind = 2.0;
        break;

    case SpeedHigh:
        wind = 3.0;
        break;
    }
    double temp = wind * 0.02;

//    qDebug() << DATETIME << "now temp : " << new_n << " ever temp : " << currentTemp << "Wind : " << speed;
    // 还需要编一个公式计算能量 暂定为 cost / 2
    cost  += temp;
    energy = cost / 2 ;
    ui->labelEnergy->setText(QString("能量：%1 度").arg(energy));
    ui->labelCost->setText(QString("费用：%1 元").arg(cost));
//    qDebug() << DATETIME << "now cost : " << cost << " temp cost : " << temp;
}


bool Client::isServing()
{
    return this->serving == ServingYes;
}


bool Client::isWorking()
{
    return this->working == WorkingYes;
}

void Client::setTempState()
{
    tempState = (currentTemp - targetTemp) > 0;
}

bool Client::isTarget()
{
    double tempT = (currentTemp - targetTemp);
    if(tempState)
    {   // example 28->26
        if(tempT > 0)
            return false;
    }
    else
    {   // 24->26
        if(tempT < 0)
            return false;
    }
    tempT = fabs(tempT);
    double Diff;
    if(speed == SpeedHigh)
        Diff = 0.2;
    else
        Diff = (double)(0.05 * (int)speed);
//    qDebug() << "Diff = " << Diff;
    tempT += 0.001;
    return tempT <= Diff;
}

bool Client::isBackTemp()
{
    if(!isServing() && isWorking() && fabs(currentTemp - targetTemp) >= 1.0)
    {
        return true;
    }
    else
        return false;
}

bool Client::hasWind()
{
    if (this->speed == SpeedNone)
    {
        return false;
    }
    else
    {
        return true;
    }
}


void Client::writeDetailedList(int option)
{  // 当出现：①达到目标 ②修改任务 ③开机 ④关机 5 断开连接
    QDateTime now_t  = QDateTime::currentDateTime();
    QString   now_ts = now_t.toString("yyyy-MM-dd hh:mm:ss");

    QString   select_max_sql = "SELECT MAX(id) from Info_list";
    int       max_id         = 0;
    QSqlQuery sql_query;

    if (!sql_query.exec(select_max_sql))
    {
        qDebug() << sql_query.lastError();
    }
    else
    {
        while (sql_query.next())
        {
            max_id = sql_query.value(0).toInt();
            qDebug() << QString("max id:%1").arg(max_id);
        }
    }
    max_id++;

    QString mid        = QString::number(max_id, 10);
    QString roomid     = id;
    QString wd         = QString::number(speed, 10);
    QString nt         = QString::number(currentTemp, 10, 4);
    QString tt         = QString::number(targetTemp, 10, 4);
    QString op         = QString::number(option, 10);
    QString cp         = QString::number(cost, 10, 4);
    QString ep         = QString::number(energy, 10, 4);
    QString insert_sql = "insert into Info_list values(" + mid + ", \"" + roomid + "\", \"" + now_ts + "\", " + wd + ", " + nt + ", " + tt + ", " + op + ", " + cp + ", " + ep + ")";
    qDebug() << "insert sql : " << insert_sql;
    if (!sql_query.exec(insert_sql))
    {
        qDebug() << DATETIME << "write_detail_list:" << sql_query.lastError();
    }
    else
    {
        qDebug() << DATETIME << "write detail list success";
//        QString str = "2999-01-12 17:35:00";
//        costStartTime = QDateTime::fromString(str, "yyyy-MM-dd hh:mm:ss");
    }
}


void Client::readDetailedList(QString roomid)
{
    QSqlQuery query;
    QString   select = "select * from Info_list where roomid = \"" + roomid
                       + "\" and start_t >= \"" + connStartTime.toString("yyyy-MM-dd hh:mm:ss") + "\"";

    qDebug() << "select sql : " << select;

    if (!query.exec(select))
    {
        qDebug() << DATETIME << "read_detail_list:" << query.lastError();
    }
    else
    {
        // 新建一个ui 来表示详单
        detailList *list = new detailList(query, NULL);
        list->show();
    }
}


void Client::on_toolButtonDetails_clicked()
{
    readDetailedList(this->id);
}


QTcpSocket *Client::getSocket() const
{
    return socket;
}


void Client::setSocket(QTcpSocket *s)
{
    socket = s;
}
