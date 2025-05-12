#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    AsciiFilter filter;

    auto p = filter.properties();
	p.headerEnabled = false;
	filter.setProperties(p);

	Spreadsheet spreadsheet("ENSO-data");
    project.addChild(&spreadsheet);

    filter.readDataFromFile(QStringLiteral("ENSO/data.txt"), &spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    Worksheet worksheet("Worksheet");
    project.addChild(&worksheet);

    worksheet.setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(15, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(15, Worksheet::Unit::Centimeter);
	worksheet.setPageRect(QRectF(0, 0, w, h));

    worksheet.setTheme(QStringLiteral("Solarized Light"));

    double ms = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
    worksheet.setLayout(Worksheet::Layout::VerticalLayout);
    worksheet.setLayoutTopMargin(ms);
	worksheet.setLayoutBottomMargin(ms);
	worksheet.setLayoutLeftMargin(ms);
	worksheet.setLayoutRightMargin(ms);

    worksheet.setLayoutHorizontalSpacing(ms);
    worksheet.setLayoutVerticalSpacing(ms);

    CartesianPlot plotArea("xy-plot");
	plotArea.setType(CartesianPlot::Type::FourAxes);
    plotArea.title()->setText(QStringLiteral("El Niño-Southern Oscillation"));
    PlotArea::BorderType border = plotArea.plotArea()->borderType();
    border.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea.plotArea()->setBorderType(border);
    worksheet.addChild(&plotArea);

    for (Axis* axis : plotArea.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->title()->setText(QStringLiteral("Month"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("Atmospheric Pressure"));
        }
    }

    XYCurve data("data");
    data.setPlotType(Plot::PlotType::Scatter);
    data.setYColumn(spreadsheet.column(0));
    data.setXColumn(spreadsheet.column(1));
    plotArea.addChild(&data);

    XYFitCurve fit("fit");

    fit.initFitData(XYAnalysisCurve::AnalysisAction::FitCustom);
    XYFitCurve::FitData fitData = fit.fitData();

    fitData.modelCategory = nsl_fit_model_custom;
    fitData.modelType = 0;
    fitData.model = QStringLiteral("b1 + b2*cos( 2*pi*x/12 ) + b3*sin( 2*pi*x/12 ) + b5*cos( 2*pi*x/b4 ) + b6*sin( 2*pi*x/b4 ) + b8*cos( 2*pi*x/b7 ) + b9*sin( 2*pi*x/b7 )");
    fitData.paramNames = {"b1", "b2", "b3", "b5", "b4", "b6", "b8", "b7", "b9"};
    fitData.paramNamesUtf8 = {"b1", "b2", "b3", "b5", "b4", "b6", "b8", "b7", "b9"};
    fitData.paramStartValues = {10.6415, 3.05277, 0.479257, -0.0808176, -0.699536, -0.243715, 0.39189, -0.300106, 0.532237};
    fitData.paramFixed.fill(false, fitData.paramNames.size());
    fitData.paramLowerLimits.fill(-std::numeric_limits<double>::max(), fitData.paramNames.size());
    fitData.paramUpperLimits.fill(std::numeric_limits<double>::max(), fitData.paramNames.size());

    XYFitCurve::initFitData(fitData);

    fit.setYDataColumn(spreadsheet.column(0));
    fit.setXDataColumn(spreadsheet.column(1));

    fit.initStartValues(fitData);
    fit.setFitData(fitData);
    fit.recalculate();

    plotArea.addChild(&fit);

    fit.recalculate();

    plotArea.addLegend();

    worksheet.view()->show();

    app.exec();
}
