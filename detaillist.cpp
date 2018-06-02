#include "detaillist.h"
#include "ui_detaillist.h"

detailList::detailList(QSqlQuery tquery, QWidget *parent) :
   QDialog(parent), sql_query(tquery),
   ui(new Ui::detailList)
{
   ui->setupUi(this);
   sql_query.last();
   int row = sql_query.at() + 1;
   ui->detail_list->setRowCount(row);
   int col = 5;
   ui->detail_list->setColumnCount(col);
   ui->detail_list->setSelectionBehavior ( QAbstractItemView::SelectRows); //设置选择行为，以行为单位
   ui->detail_list->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行

   sql_query.first();
   int j = 0;
   do
   {
      for (int i = 0; i < col; i++)
      {
         ui->detail_list->setItem(j, i, new QTableWidgetItem(sql_query.value(i + 1).toString()));
      }
      // 考虑一下 转换是否是这样。
      j++;
   }while (sql_query.next());
}


detailList::~detailList()
{
   delete ui;
}


void detailList::on_exit_clicked()
{
   this->close();
}
