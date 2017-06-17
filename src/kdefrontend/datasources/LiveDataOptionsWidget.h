#ifndef LIVEDATAOPTIONSWIDGET_H
#define LIVEDATAOPTIONSWIDGET_H

#include <QWidget>

namespace Ui {
class LiveDataOptionsWidget;
}

class LiveDataOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LiveDataOptionsWidget(QWidget *parent = 0);
    ~LiveDataOptionsWidget();

private:
    Ui::LiveDataOptionsWidget *ui;
};

#endif // LIVEDATAOPTIONSWIDGET_H
