#include "LiveDataOptionsWidget.h"
#include "ui_livedataoptionswidget.h"

LiveDataOptionsWidget::LiveDataOptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LiveDataOptionsWidget)
{
    ui->setupUi(this);
}

LiveDataOptionsWidget::~LiveDataOptionsWidget()
{
    delete ui;
}
