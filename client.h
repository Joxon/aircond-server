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
   explicit Client(QWidget *parent = 0);
   ~Client();

//   enum Conn
//   {
//      ConnOffline,
//      ConnOnline
//   };

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
      SpeedHigh
   };

public slots:
   void setId(const QString& value);

//   void setConn(Client::Conn conn);

   void setWorking(Client::Working work);
   void setServing(Client::Serving serve);

   void setCurrentTemp(double temp);
   void setTargetTemp(double temp);
   void setSpeed(Client::Speed _speed);

   void setEnergy(double _energy);
   void setCost(double _cost);
   void setST();

   double getCurrentTemp() const;
   Speed getSpeed() const;
   double getCost() const;
   void Cost_Cal(double new_n);                              // 计算价格
   bool CheckServing();                                      // 判断服务
   void write_detail_list(QString roomid);                   // 写入数据库
   void read_detail_list(QString roomid, QString starttime); // 打印详单

private slots:

   void on_toolButtonDetails_clicked();

private:
   Ui::Client *ui;

   int fontId;
   QString fontName;
   QFont font;

   QString id;
//   Conn conn;                                              // 连接状态
   Working working;                                        // 工作状态
   Serving serving;                                        // 服务状态
   double currentTemp;                                     // 当前温度
   double targetTemp;                                      // 目标温度
   Speed speed;                                            // 风速
   double energy;                                          // 消耗的能量
   double cost;                                            // 费用

   QDateTime start_t;                                      // 开启时间

//   void Init_Room();                                       // 初始化房间
};

#endif // CLIENT_H
