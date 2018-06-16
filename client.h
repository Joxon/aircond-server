#ifndef CLIENT_H
#define CLIENT_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "quiwidget.h"

#include <QtCore>
#include <QtWidgets>
#include <QtGui>
#include <QtSql/QtSql>

namespace Ui {
class Client;
}

class Client : public QWidget
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = nullptr);
    ~Client();

    enum Conn
    {
        ConnOffline,
        ConnOnline
    };

    enum Working
    {
        WorkingNo,
        WorkingYes,
        WorkingDone
    };

    enum Serving
    {
        ServingNo,
        ServingYes
    };

    enum Speed
    {
        SpeedNone,
        SpeedLow,
        SpeedMid,
        SpeedHigh
    };

    void setId(const QString& i);
    void setConn(Client::Conn c);
    void setWorking(Client::Working work);
    void setServing(Client::Serving s);
    void setCurrentTemp(double t);
    void setTargetTemp(double t);
    void setSpeed(int s);
    void setLastSpeed(Client::Speed s);
    void setEnergy(double e);
    void setCost(double c);
    void setStartTime();
    void setSocket(QTcpSocket *s);
    void setTempState();

    QString getId();
    int getLastSpeed();
    double getTargetTemp();
    double getCurrentTemp() const;
    Speed getSpeed() const;
    double getCost() const;

//    QDateTime getTime();                               // 获得start_t;
//    void setTime(QDateTime time);                      // 设置start_t;
    QTcpSocket *getSocket() const;

    void calCost();    // 计算价格
    bool isServing();  // 判断服务
    bool isWorking();  // 判断工作
    bool isTarget();   // 达到目标
    bool isBackTemp(); // 正在回温
    bool hasWind();    // 判断风速

public slots:
    void writeDetailedList(int option);            // 写入数据库
    void readDetailedList(QString roomid);         // 打印详单

private slots:
    void on_comboBox_activated(const QString& arg1);

private:
    Ui::Client *ui;

    int fontId;
    QString fontName;
    QFont font;

    QString id;
    Conn conn;               // 连接状态
    Working working;         // 工作状态
    Serving serving;         // 服务状态
    double currentTemp;      // 当前温度
    double targetTemp;       // 目标温度
    Speed speed;             // 风速
    Speed lastSpeed;         // 上次风速
    double energy;           // 消耗的能量
    double cost;             // 费用
    bool tempState;          // 趋近关系 true 下 false 上
    QDateTime costStartTime; // 计费开启时间
    QDateTime connStartTime; // 连接开启时间

    QTcpSocket *socket;
};

#endif // CLIENT_H
