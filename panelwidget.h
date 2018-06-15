#ifndef PANELWIDGET_H
#define PANELWIDGET_H

#include "client.h"

#include <QtWidgets>

class PanelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanelWidget(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *);

private:
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QFrame *frame;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;

public:
    QSize sizeHint()                const;
    QSize minimumSizeHint()         const;

public slots:
    void setWidget(QList<QWidget *> widgets, int columnCount);
    void setMargin(int left, int top, int right, int bottom);
    void setMargin(int margin);
    void setSpacing(int space);
};

#endif // PANELWIDGET_H
