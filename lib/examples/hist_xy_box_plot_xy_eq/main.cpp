#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>

#include <labplot.h>

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	auto* mainWidget = new QWidget();
	auto* layout = new QHBoxLayout;
	mainWidget->setLayout(layout);

	auto* worksheet = new Worksheet(QStringLiteral("test"));

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	plot->setType(CartesianPlot::Type::FourAxes);
	worksheet->addChild(plot);

	int count = 10;
	double a = 1.27;
	double b = 4.375;
	double c = -6.692;

	auto* x = new Column(QStringLiteral("x"));
	auto* y = new Column(QStringLiteral("y"));

	for (int i = 0; i < count; ++i) {
		x->setValueAt(i, i);
		double randValue = 10 * double(rand()) / double(RAND_MAX);
		double value = a * pow(i, 2) + b * i + c + randValue;
		y->setValueAt(i, value);
	}

	auto* curve = new XYCurve(QStringLiteral("raw data"));
	curve->setXColumn(x);
	curve->setYColumn(y);
	curve->setLineType(XYCurve::LineType::NoLine);
	curve->symbol()->setStyle(Symbol::Style::Circle);
	plot->addChild(curve);

	auto* eqCurve = new XYEquationCurve(QStringLiteral("eq"));
	plot->addChild(eqCurve);
	auto data = XYEquationCurve::EquationData();
	data.expression1 = QStringLiteral("50*sin(x)");
	data.min = QStringLiteral("0.0");
	data.max = QStringLiteral("10.0");
	eqCurve->setEquationData(data);
	eqCurve->recalculate();

	plot->addLegend();

	auto* plot2 = new CartesianPlot(QStringLiteral("plot 2"));
	plot2->setType(CartesianPlot::Type::FourAxes);
	worksheet->addChild(plot2);

	auto* hist = new Histogram(QStringLiteral("histogram"));
	auto* randomData = new Column(QStringLiteral("x"));

	for (int i = 0; i < 1000; ++i)
		randomData->setValueAt(i, double(100 * rand()) / double(RAND_MAX));

	hist->setDataColumn(randomData);
	plot2->addChild(hist);

	auto* plot3 = new CartesianPlot(QStringLiteral("plot 3"));
	plot3->setType(CartesianPlot::Type::FourAxes);
	worksheet->addChild(plot3);

	auto* boxPlot = new BoxPlot(QStringLiteral("boxplot"));
	QVector<const AbstractColumn*> columns{randomData};
	boxPlot->setDataColumns(columns);
	plot3->addChild(boxPlot);

	layout->addWidget(worksheet->view());
	mainWidget->show();

	return app.exec();
}
