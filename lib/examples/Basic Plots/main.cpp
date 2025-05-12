#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    AsciiFilter filter;

    auto p = filter.properties();
	p.headerEnabled = false;
	filter.setProperties(p);

	Spreadsheet spreadsheet("Spreadsheet");
    project.addChild(&spreadsheet);

    filter.readDataFromFile(QStringLiteral("Basic Plots/data.txt"), &spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    Worksheet worksheet("Worksheet - Spreadsheet");
    project.addChild(&worksheet);

    worksheet.setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	worksheet.setPageRect(QRectF(0, 0, w, h));

    worksheet.setLayout(Worksheet::Layout::GridLayout);
    worksheet.setLayoutRowCount(2);
    worksheet.setLayoutColumnCount(2);

    worksheet.setLayoutTopMargin(Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter));
	worksheet.setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter));
	worksheet.setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet.setLayoutRightMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    worksheet.setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter));
    worksheet.setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter));

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea1("Plot - Spreadsheet");
	plotArea1.setType(CartesianPlot::Type::FourAxes);

    plotArea1.setSymmetricPadding(false);
    plotArea1.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea1.setVerticalPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea1.setRightPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea1.setBottomPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea1.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("Frequency"));
        } else {
            axis->title()->setText(QStringLiteral(""));
            axis->title()->setVisible(false);
        }
    }

    worksheet.addChild(&plotArea1);

    Histogram config11("Lognormal(1,1)");
    plotArea1.addChild(&config11);
    config11.setDataColumn(spreadsheet.column(1));
    config11.setType(Histogram::Type::Ordinary);
    config11.setOrientation(Histogram::Orientation::Vertical);
    config11.setNormalization(Histogram::Normalization::Probability);
    config11.setBinningMethod(Histogram::BinningMethod::SquareRoot);
    config11.setAutoBinRanges(true);
    config11.line()->setHistogramLineType(Histogram::LineType::Bars);
    config11.symbol()->setStyle(Symbol::Style::NoSymbols);
    config11.value()->setType(Value::Type::NoValues);
    config11.background()->setType(Background::Type::Color);
    config11.background()->setColorStyle(Background::ColorStyle::SingleColor);
    config11.background()->setFirstColor(config11.color());
    config11.background()->setOpacity(0.5);
    config11.background()->setEnabled(true);
    config11.setRugEnabled(true);
    config11.setRugLength(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    config11.setRugWidth(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Point));
    config11.setRugOffset(Worksheet::convertToSceneUnits(-5, Worksheet::Unit::Point));

    XYFitCurve fit11("Distribution Fit to 'Lognormal(1,1)'");
    plotArea1.addChild(&fit11);

    fit11.setDataSourceType(XYFitCurve::DataSourceType::Histogram);
    fit11.setDataSourceHistogram(&config11);

    XYFitCurve::FitData fitData11 = fit11.fitData();
    fitData11.modelCategory = nsl_fit_model_category::nsl_fit_model_distribution;
    fitData11.modelType = nsl_sf_stats_distribution::nsl_sf_stats_gaussian;

    XYFitCurve::initFitData(fitData11);
    fit11.initStartValues(fitData11);
    fit11.setFitData(fitData11);
    fit11.recalculate();

    Histogram config12("Normal(0,1)");
    plotArea1.addChild(&config12);
    config12.setDataColumn(spreadsheet.column(2));
    config12.setType(Histogram::Type::Ordinary);
    config12.setOrientation(Histogram::Orientation::Vertical);
    config12.setNormalization(Histogram::Normalization::ProbabilityDensity);
    config12.setBinningMethod(Histogram::BinningMethod::SquareRoot);
    config12.setAutoBinRanges(true);
    config12.line()->setHistogramLineType(Histogram::LineType::Bars);
    config12.symbol()->setStyle(Symbol::Style::NoSymbols);
    config12.value()->setType(Value::Type::NoValues);
    config12.background()->setType(Background::Type::Color);
    config12.background()->setColorStyle(Background::ColorStyle::SingleColor);
    config12.background()->setFirstColor(config12.color());
    config12.background()->setOpacity(0.5);
    config12.background()->setEnabled(true);
    config12.setRugEnabled(true);
    config12.setRugLength(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    config12.setRugWidth(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Point));
    config12.setRugOffset(Worksheet::convertToSceneUnits(-5, Worksheet::Unit::Point));

    XYFitCurve fit12("Distribution Fit to 'Normal(0,1)'");
    plotArea1.addChild(&fit12);

    fit12.setDataSourceType(XYFitCurve::DataSourceType::Histogram);
    fit12.setDataSourceHistogram(&config12);

    XYFitCurve::FitData fitData12 = fit12.fitData();
    fitData12.modelCategory = nsl_fit_model_category::nsl_fit_model_distribution;
    fitData12.modelType = nsl_sf_stats_distribution::nsl_sf_stats_gaussian;

    XYFitCurve::initFitData(fitData12);
    fit12.initStartValues(fitData12);
    fit12.setFitData(fitData12);
    fit12.recalculate();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea2("Plot - Spreadsheet 1");
	plotArea2.setType(CartesianPlot::Type::FourAxes);

    plotArea2.setSymmetricPadding(false);
    plotArea2.setHorizontalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea2.setVerticalPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea2.setRightPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea2.setBottomPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea2.children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
    }

    worksheet.addChild(&plotArea2);

    BoxPlot config2("config2");
    plotArea2.addChild(&config2);
    config2.setDataColumns({spreadsheet.column(1), spreadsheet.column(2)});
    config2.setOrdering(BoxPlot::Ordering::None);
    config2.setWhiskersType(BoxPlot::WhiskersType::IQR);
    config2.setWhiskersRangeParameter(1.5);
    config2.setOrientation(BoxPlot::Orientation::Horizontal);
    config2.setVariableWidth(false);
    config2.setNotchesEnabled(true);
    config2.symbolMean()->setStyle(Symbol::Style::Square);
    config2.symbolOutlier()->setStyle(Symbol::Style::Circle);
    config2.setJitteringEnabled(true);
    config2.setRugEnabled(true);
    config2.setRugLength(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    config2.setRugWidth(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Point));
    config2.setRugOffset(Worksheet::convertToSceneUnits(-5, Worksheet::Unit::Point));
    config2.setWhiskersCapSize(Worksheet::convertToSceneUnits(20, Worksheet::Unit::Point));
    config2.recalc();

    // // ###################################################################################################################################################################
    // // ###################################################################################################################################################################

    CartesianPlot plotArea3("Plot - Spreadsheet 2");
	plotArea3.setType(CartesianPlot::Type::FourAxes);

    plotArea3.setSymmetricPadding(false);
    plotArea3.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea3.setVerticalPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea3.setRightPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea3.setBottomPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea3.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->title()->setText(QStringLiteral("Time"));
        } else {
            axis->title()->setText(QStringLiteral(""));
            axis->title()->setVisible(false);
        }
    }

    worksheet.addChild(&plotArea3);

    XYCurve config31("Normal(-1,2)");
    plotArea3.addChild(&config31);
    config31.setPlotType(Plot::PlotType::Scatter);
    config31.setXColumn(spreadsheet.column(0));
    config31.setYColumn(spreadsheet.column(1));
    config31.setLineType(XYCurve::LineType::NoLine);
    config31.symbol()->setStyle(Symbol::Style::Circle);
    config31.symbol()->setSize(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
    config31.symbol()->setOpacity(0.7);
    config31.setValuesType(XYCurve::ValuesType::NoValues);
    config31.background()->setPosition(Background::Position::No);
    config31.setRugEnabled(true);
    config31.setRugLength(Worksheet::convertToSceneUnits(15, Worksheet::Unit::Point));
    config31.setRugWidth(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Point));
    config31.setRugOffset(Worksheet::convertToSceneUnits(-30, Worksheet::Unit::Point));
    config31.setRugOrientation(WorksheetElement::Orientation::Vertical);
    config31.recalc();

    XYCurve config32("Normal(0,1)");
    plotArea3.addChild(&config32);
    config32.setPlotType(Plot::PlotType::Scatter);
    config32.setXColumn(spreadsheet.column(0));
    config32.setYColumn(spreadsheet.column(2));
    config32.setLineType(XYCurve::LineType::NoLine);
    config32.symbol()->setStyle(Symbol::Style::Circle);
    config32.symbol()->setSize(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
    config32.symbol()->setOpacity(0.7);
    config32.setValuesType(XYCurve::ValuesType::NoValues);
    config32.background()->setPosition(Background::Position::No);
    config32.setRugEnabled(true);
    config32.setRugLength(Worksheet::convertToSceneUnits(15, Worksheet::Unit::Point));
    config32.setRugWidth(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Point));
    config32.setRugOffset(Worksheet::convertToSceneUnits(-30, Worksheet::Unit::Point));
    config32.setRugOrientation(WorksheetElement::Orientation::Vertical);
    config32.recalc();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    worksheet.view()->show();

    app.exec();
}
