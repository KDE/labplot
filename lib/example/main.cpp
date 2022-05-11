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

	//add a curve defined via a mathematical equation
	auto* eqCurve = new XYEquationCurve("eq");
	plot->addChild(eqCurve);
	auto data = XYEquationCurve::EquationData();
	data.expression1 = "50*sin(x)";
	data.min = "0.0";
	data.max = "10.0";
	eqCurve->setEquationData(data);
	eqCurve->recalculate();

	//add legend
	plot->addLegend();

	//create another plot
	auto* plot2 = new CartesianPlot("plot 2");
	plot2->setType(CartesianPlot::Type::FourAxes);
	worksheet->addChild(plot2);

	//create some random data and plot a histogram for them
	auto* hist = new Histogram("histogram");
	auto* random_data = new Column("x");

	for (int i = 0; i < 1000; ++i)
		random_data->setValueAt(i, double(100*qrand())/double(RAND_MAX));

	hist->setDataColumn(random_data);
	plot2->addChild(hist);

	//create one more plot
	auto* plot3 = new CartesianPlot("plot 3");
	plot3->setType(CartesianPlot::Type::FourAxes);
	worksheet->addChild(plot3);

	//add box plot
	auto* boxPlot = new BoxPlot("boxplot");
	QVector<const AbstractColumn*> columns{random_data};
	boxPlot->setDataColumns(columns);
	plot3->addChild(boxPlot);

	layout->addWidget(worksheet->view());
	mainWidget->show();

	return app.exec();
}
