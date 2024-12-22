#include <labplot/labplot.h>

int main(int argc, char **argv) {
	// create a spreadsheet and import the data into it
	auto* spreadsheet = new Spreadsheet("data");
	AsciiFilter filter;
	filter.readDataFromFile("data.txt", spreadsheet);

	// create a worksheet
	auto* worksheet = new Worksheet("worksheet");

	//create a plot area and add it to the worksheet
	auto* plotArea = new CartesianPlot("plot area");
	plotArea->setType(CartesianPlot::Type::FourAxes);
	plotArea->addLegend();
	worksheet->addChild(plotArea);

	// create a histogram for the imported data and add it to the plot area
	auto* histogram = new Histogram("histogram");
	histogram->setNormalization(Histogram::Normalization::ProbabilityDensity);
	histogram->setDataColumn(spreadsheet->column(0));
	plotArea->addChild(histogram);

	//perform a fit to the raw data and show it
	auto* fitCurve = new XYFitCurve("fit");
	fitCurve->setDataSourceType(XYAnalysisCurve::DataSourceType::Histogram);
	fitCurve->setDataSourceHistogram(histogram);
	plotArea->addChild(histogram);

	// initialize the fit
	auto fitData = fitCurve->fitData();
	fitData.modelCategory = nsl_fit_model_distribution;
	fitData.modelType = nsl_sf_stats_gaussian;
	fitData.algorithm = nsl_fit_algorithm_ml; // ML distribution fit
	XYFitCurve::initFitData(fitData);
	fitCurve->setFitData(fitData);

	// perform the actual fit
	fitCurve->recalculate();

	// apply the theme "Dracula"
	worksheet->loadTheme("Dracula");

	// export the worksheet to PDF
	// TODO:
}
