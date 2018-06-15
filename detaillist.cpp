#include "detaillist.h"
#include "ui_detaillist.h"
#include <QDebug>

detailList::detailList(QSqlQuery tquery, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::detailList),
    sql_query(tquery)
{
    ui->setupUi(this);

    ui->detail_list->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为，以行为单位
    ui->detail_list->setSelectionMode(QAbstractItemView::SingleSelection); //设置选择模式，选择单行

    sql_query.last();
    int row = sql_query.at() + 1;
    ui->detail_list->setRowCount(row);
    int col = 8;
    ui->detail_list->setColumnCount(col);

    QStringList header;
    header << "RoomId" << "操作时间" << "风速" << "目标温度" << "当前温度" << "操作" << "当前价格消费" << "当前能量消费";
    ui->detail_list->setHorizontalHeaderLabels(header);

    sql_query.first();
    int j = 0;
    do
    {
        qDebug() << "detailist : room id " << sql_query.value(1).toString() << " start time " << sql_query.value(2).toString();
        for (int i = 0; i < col; i++)
        {
            if (i != 5)
            {
                ui->detail_list->setItem(j, i, new QTableWidgetItem(sql_query.value(i + 1).toString()));
            }
            else
            {
                QString op = "";
                switch (sql_query.value(i + 1).toInt())
                {
                case 0:
                    op = "到达目标温度";
                    break;

                case 1:
                    op = "修改任务";
                    break;

                case 2:
                    op = "开机";
                    break;

                case 3:
                    op = "关机";
                    break;

                case 4:
                    op = "断开连接";
                    break;

                case 5:
                    op = "给予资源";
                    break;

                case 6:
                    op = "剥夺资源";
                    break;
                }
                ui->detail_list->setItem(j, i, new QTableWidgetItem(op));
            }
        }
        j++;
    } while (sql_query.next());
}


detailList::~detailList()
{
    delete ui;
}


void detailList::on_exit_clicked()
{
    this->close();
}
