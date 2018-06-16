#ifndef DAILYREPORT_H
#define DAILYREPORT_H

#include <QDialog>

namespace Ui {
class DailyReport;
}

class DailyReport : public QDialog
{
    Q_OBJECT

public:
    explicit DailyReport(QWidget *parent = nullptr, QString roomID = "");
    ~DailyReport();

private slots:
    void on_radioButtonCurrentRoom_clicked(bool checked);

    void on_radioButtonAllRooms_clicked(bool checked);

private:
    Ui::DailyReport *ui;
    QString currentRoomID;

    void queryCurrentRoom();
    void queryAllRooms();
};

#endif // DAILYREPORT_H
