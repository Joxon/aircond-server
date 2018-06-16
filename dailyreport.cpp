#include "dailyreport.h"
#include "ui_dailyreport.h"

#include <QSqlQuery>

DailyReport::DailyReport(QWidget *parent, QString roomID) :
    QDialog(parent),
    ui(new Ui::DailyReport),
    currentRoomID(roomID)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowCloseButtonHint);

    ui->radioButtonCurrentRoom->setText(QString("当前房间(%1)").arg(roomID));
    ui->radioButtonCurrentRoom->setChecked(true);
    ui->radioButtonAllRooms->setChecked(false);

    queryCurrentRoom();
}


DailyReport::~DailyReport()
{
    delete ui;
}


void DailyReport::on_radioButtonCurrentRoom_clicked(bool checked)
{
    if (checked)
    {
        ui->radioButtonAllRooms->setChecked(false);
        queryCurrentRoom();
    }
}


void DailyReport::on_radioButtonAllRooms_clicked(bool checked)
{
    if (checked)
    {
        ui->radioButtonCurrentRoom->setChecked(false);
        queryAllRooms();
    }
}


void DailyReport::queryCurrentRoom()
{
}


void DailyReport::queryAllRooms()
{
}
