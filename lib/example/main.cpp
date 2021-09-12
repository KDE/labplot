#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>

#include <labplot/labplot.h>

int main(int argc, char **argv) {
	QApplication app (argc, argv);

	auto* mainWidget = new QWidget();
	auto*  layout = new QHBoxLayout;
	mainWidget->setLayout(layout);

	//create a worksheet and a plot
	auto* worksheet = new Worksheet("test");

	//create a plot
	auto* plot = new CartesianPlot("plot");
	plot->setType(CartesianPlot::Type::FourAxes);
	worksheet->addChild(plot);

	//Generate some data for f(x) = a*x^2 + b*x + c with additional noise
	int count = 10;
	double a = 1.27;
	double b = 4.375;
	double c = -6.692;

	auto* x = new Column("x");
	auto* y = new Column("y");

	for (int i = 0; i < count; ++i) {
		x->setValueAt(i, i);
		double rand_value = 10*double(qrand())/double(RAND_MAX);
		double value = a*pow(i, 2) + b*i + c + rand_value;
		y->setValueAt(i, value);
	}

	auto* curve = new XYCurve("raw data");
	curve->setXColumn(x);
	curve->setYColumn(y);
// 	curve->setLineStyle(XYCurve::LineStyle::NoLine);
// 	curve->symbol()->setStyle(Symbol::Circle);
	plot->addChild(curve);
	plot->autoScale();

	//perform a fit to the raw data and show it
	auto* fitCurve = new XYFitCurve("fit ");
	//TODO:
	fitCurve->recalculate();
	plot->addChild(fitCurve);

	//add legend
	plot->addLegend();

	layout->addWidget(worksheet->view());

	mainWidget->show();

	return app.exec();
}
