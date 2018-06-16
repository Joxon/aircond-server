#include "detaillist.h"
#include "ui_detaillist.h"
#include <QDebug>

DetailList::DetailList(QWidget *parent, QSqlQuery q) :
    QDialog(parent),
    ui(new Ui::DetailList),
    query(q)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowCloseButtonHint);

    query.last();
    int rowCount = query.at() + 1;
    ui->detail_list->setRowCount(rowCount);
    const int colCount = 8;
    ui->detail_list->setColumnCount(colCount);

    QStringList header;
    header << "房间号" << "操作时间" << "风速" << "当前温度" << "目标温度" << "操作" << "当前价格消费" << "当前能量消费";
    ui->detail_list->setHorizontalHeaderLabels(header);

    query.first();
    int row = 0;
    do
    {
        qDebug() << "detailist : room id " << query.value(1).toString() << " start time " << query.value(2).toString();
        for (int col = 0; col < colCount; col++)
        {
            if (col != 5)
            {
                ui->detail_list->setItem(row, col, new QTableWidgetItem(query.value(col + 1).toString()));
            }
            else
            {
                QString op = "";
                switch (query.value(col + 1).toInt())
                {
                case OP_TGT_REACHED:
                    op = "到达目标温度";
                    break;

                case OP_TASK_MODIFIED:
                    op = "修改任务";
                    break;

                case OP_START_UP:
                    op = "开机";
                    break;

                case OP_SHUT_DOWN:
                    op = "关机";
                    break;

                case OP_DISCONNECTED:
                    op = "断开连接";
                    break;

                case OP_RES_ASSIGNED:
                    op = "给予资源";
                    break;

                case OP_RES_REMOVED:
                    op = "剥夺资源";
                    break;
                }
                ui->detail_list->setItem(row, col, new QTableWidgetItem(op));
            }
        }
        row++;
    } while (query.next());
}


DetailList::~DetailList()
{
    delete ui;
}
