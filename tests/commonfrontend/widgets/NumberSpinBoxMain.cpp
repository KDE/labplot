#include "NumberSpinBox.h"

#include <QApplication>
#include <QLineEdit>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr)
		: QMainWindow(parent) {
		QWidget* w = new QWidget(this);
		NumberSpinBox* nb = new NumberSpinBox(w);
		QLineEdit* le = new QLineEdit(w);
		QVBoxLayout* l = new QVBoxLayout();

        connect(nb, &NumberSpinBox::valueChanged, [le] (double value) {
            le->setText(QString::number(value));
        });

		l->addWidget(nb);
		l->addWidget(le);
		w->setLayout(l);
		setCentralWidget(w);
	}

private:
};

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}

#include "NumberSpinBoxMain.moc"
