#ifndef DETAILLIST_H
#define DETAILLIST_H

#include <QDialog>
#include <QSqlQuery>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#define OP_TGT_REACHED      1
#define OP_TASK_MODIFIED    2
#define OP_START_UP         3
#define OP_SHUT_DOWN        4
#define OP_DISCONNECTED     5
#define OP_RES_REMOVED      6
#define OP_RES_ASSIGNED     7

namespace Ui {
class DetailList;
}

class DetailList : public QDialog
{
    Q_OBJECT

public:
    DetailList(QWidget *parent, QSqlQuery q);
    ~DetailList();

private:
    Ui::DetailList *ui;
    QSqlQuery query;
};

#endif // DETAILLIST_H
