#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>

#include <labplot/labplot.h>

int main(int argc, char **argv) {
	QApplication app (argc, argv);

	auto* mainWidget = new QWidget();
	auto*  layout = new QHBoxLayout;
	mainWidget->setLayout(layout);


	auto* worksheet = new Worksheet("test");
	layout->addWidget(worksheet->view());

	mainWidget->show();

	return app.exec();
}
