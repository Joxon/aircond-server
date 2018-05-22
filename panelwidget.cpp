#include "panelwidget.h"

#include <QScrollArea>
#include <QFrame>
#include <QBoxLayout>

PanelWidget::PanelWidget(QWidget *parent) : QWidget(parent)
{
   scrollArea = new QScrollArea(this);
   scrollArea->setObjectName("scrollAreaMain");
   scrollArea->setWidgetResizable(true);

   scrollAreaWidgetContents = new QWidget();
   scrollAreaWidgetContents->setGeometry(QRect(0, 0, 100, 100));

   verticalLayout = new QVBoxLayout(scrollAreaWidgetContents);
   verticalLayout->setSpacing(0);
   verticalLayout->setContentsMargins(0, 0, 0, 0);

   frame = new QFrame(scrollAreaWidgetContents);
   frame->setObjectName("frameMain");

   gridLayout = new QGridLayout(frame);
   gridLayout->setSpacing(0);

   verticalLayout->addWidget(frame);
   scrollArea->setWidget(scrollAreaWidgetContents);
   frame->setStyleSheet("QFrame#frameMain{border-width:0px}");
}


void PanelWidget::resizeEvent(QResizeEvent *)
{
   scrollArea->resize(this->size());
}


QSize PanelWidget::sizeHint() const
{
   return QSize(300, 200);
}


QSize PanelWidget::minimumSizeHint() const
{
   return QSize(20, 20);
}


void PanelWidget::setWidget(QList<QWidget *> widgets, int columnCount)
{
   //先清空原有所有元素
   //qDeleteAll(frame->findChildren<QWidget *>());

   int row = 0;
   int col = 0;
   int idx = 0;

   foreach(QWidget * widget, widgets)
   {
      gridLayout->addWidget(widget, row, col);
      col++;
      idx++;
      if (idx % columnCount == 0)
      {
         row++;
         col = 0;
      }
   }
   row++;

   QSpacerItem *verticalSpacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
   gridLayout->addItem(verticalSpacer, row, 0);
}


void PanelWidget::setMargin(int left, int top, int right, int bottom)
{
   gridLayout->setContentsMargins(left, top, right, bottom);
}


void PanelWidget::setMargin(int margin)
{
   setMargin(margin, margin, margin, margin);
}


void PanelWidget::setSpacing(int space)
{
   gridLayout->setSpacing(space);
}
