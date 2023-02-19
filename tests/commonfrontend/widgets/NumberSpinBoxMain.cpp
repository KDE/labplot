#include "NumberSpinBox.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr)
		: QMainWindow(parent) {
		QWidget* w = new QWidget(this);
		QVBoxLayout* l = new QVBoxLayout();
		NumberSpinBox* nb = new NumberSpinBox(5, w);

		{
			QHBoxLayout* h = new QHBoxLayout();
			auto* lb = new QLabel(QStringLiteral("Prefix:"), this);
			auto* le = new QLineEdit(this);
			h->addWidget(lb);
			h->addWidget(le);
			l->addLayout(h);

			connect(le, &QLineEdit::textChanged, nb, &NumberSpinBox::setPrefix);
		}

		{
			QHBoxLayout* h = new QHBoxLayout();
			auto* lb = new QLabel(QStringLiteral("Suffix:"), this);
			auto* le = new QLineEdit(this);
			h->addWidget(lb);
			h->addWidget(le);
			l->addLayout(h);

			connect(le, &QLineEdit::textChanged, nb, &NumberSpinBox::setSuffix);
		}

		{
			QHBoxLayout* h = new QHBoxLayout();
			auto* lb = new QLabel(QStringLiteral("Min:"), this);
			auto* sb = new QSpinBox(this);
			sb->setMinimum(-1e6);
			sb->setMaximum(1e6);
			sb->setValue(-10);
			nb->setMinimum(-10);
			h->addWidget(lb);
			h->addWidget(sb);
			l->addLayout(h);

			connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), [nb](int min) {
				nb->setMinimum(min);
			});
		}

		{
			QHBoxLayout* h = new QHBoxLayout();
			auto* lb = new QLabel(QStringLiteral("Max:"), this);
			auto* sb = new QSpinBox(this);
			sb->setMinimum(-1e6);
			sb->setMaximum(1e6);
			sb->setValue(10);
			nb->setMaximum(10);
			h->addWidget(lb);
			h->addWidget(sb);
			l->addLayout(h);

			connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), [nb](int max) {
				nb->setMaximum(max);
			});
		}

		QLineEdit* le = new QLineEdit(w);

		connect(nb, QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged), [le](Common::ExpressionValue value) {
			le->setText(QString::number(value.value<double>()));
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
