#ifndef DETAILLIST_H
#define DETAILLIST_H

#include <QDialog>
#include <QSqlQuery>

namespace Ui {
class detailList;
}

class detailList : public QDialog
{
   Q_OBJECT

public:
   detailList(QSqlQuery tquery, QWidget *parent = 0);
   ~detailList();

private slots:
   void on_exit_clicked();

private:
   Ui::detailList *ui;
   QSqlQuery sql_query;
};

#endif // DETAILLIST_H
