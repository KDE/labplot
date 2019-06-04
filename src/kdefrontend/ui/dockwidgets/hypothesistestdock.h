#ifndef HYPOTHESISTESTDOCK_H
#define HYPOTHESISTESTDOCK_H

#include <QDialog>

namespace Ui {
class HypothesisTestDock;
}

class HypothesisTestDock : public QDialog
{
    Q_OBJECT

public:
    explicit HypothesisTestDock(QWidget *parent = nullptr);
    ~HypothesisTestDock();

private:
    Ui::HypothesisTestDock *ui;
};

#endif // HYPOTHESISTESTDOCK_H
