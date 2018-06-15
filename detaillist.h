#ifndef DETAILLIST_H
#define DETAILLIST_H

#include <QDialog>
#include <QSqlQuery>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

namespace Ui {
class detailList;
}

class detailList : public QDialog
{
    Q_OBJECT

public:
    detailList(QSqlQuery tquery, QWidget *parent = nullptr);
    ~detailList();

private slots:
    void on_exit_clicked();

private:
    Ui::detailList *ui;
    QSqlQuery sql_query;
};

#endif // DETAILLIST_H
