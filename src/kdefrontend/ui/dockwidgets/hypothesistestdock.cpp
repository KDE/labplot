#include "hypothesistestdock.h"
#include "ui_hypothesistestdock.h"

HypothesisTestDock::HypothesisTestDock(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HypothesisTestDock)
{
    ui->setupUi(this);
}

HypothesisTestDock::~HypothesisTestDock()
{
    delete ui;
}
