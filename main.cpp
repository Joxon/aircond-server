#include "mainwindow.h"
#include "quiwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   QUIWidget    qui;
   MainWindow   mwnd(&qui);

   app.setWindowIcon(QIcon(":/main.ico"));
   qui.setMainWidget(&mwnd);
   qui.setTitle("A Simple Server");
   qui.setStyle(QUIWidget::StyleDarkGray);
   //qui.setSizeGripEnabled(true);
   qui.setVisible(QUIWidget::BtnMenuMax, false);
   qui.setVisible(QUIWidget::BtnMenuThemes, true);
   qui.show();

   return app.exec();
}
