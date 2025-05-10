#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    // #####################################################################################################################################################################
    // #####################################################################################################################################################################
    // #####################################################################################################################################################################

    auto* lld = new Folder("Lower Level of Difficulty");
    project.addChild(lld);

    auto* allFits = new Worksheet("All Fits");
    lld->addChild(allFits);

    allFits->setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	allFits->setPageRect(QRectF(0, 0, w, h));

    allFits->setLayout(Worksheet::Layout::VerticalLayout);
    allFits->setLayoutTopMargin(Worksheet::convertToSceneUnits(1.2, Worksheet::Unit::Centimeter));

    auto* wsTextLabel = new TextLabel("WS Text Label", TextLabel::Type::General);
    wsTextLabel->setText(QStringLiteral("Lower Level of Difficulty"));

    allFits->addChild(wsTextLabel);

    wsTextLabel->setVerticalAlignment(WorksheetElement::VerticalAlignment::Center);
    wsTextLabel->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    
    wsTextLabel->position().point.setX(Worksheet::convertToSceneUnits(-0.3, Worksheet::Unit::Centimeter));
    wsTextLabel->position().point.setY(Worksheet::convertToSceneUnits(9.5, Worksheet::Unit::Centimeter));
    wsTextLabel->setPosition(wsTextLabel->position());

    AsciiFilter filter;

    auto p = filter.properties();
	p.headerEnabled = false;
	filter.setProperties(p);

	auto* spreadsheet = new Spreadsheet("Norris Data");
    lld->addChild(spreadsheet);

    filter.readDataFromFile(QStringLiteral("NIST - Linear Regression/norris_data.txt"), spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    auto* plotArea = new CartesianPlot("Plot Area");
	plotArea->setType(CartesianPlot::Type::FourAxes);

    for (Axis* axis : plotArea->children<Axis>()) {
        axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        axis->minorGridLine()->setStyle(Qt::PenStyle::NoPen);
    }

    allFits->addChild(plotArea);

    plotArea->title()->setText(QStringLiteral("Norris"));
    plotArea->title()->setVerticalAlignment(WorksheetElement::VerticalAlignment::Top);
    plotArea->title()->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    WorksheetElement::PositionWrapper position;
    position.point = {Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-0.5, Worksheet::Unit::Centimeter)};
    position.verticalPosition = WorksheetElement::VerticalPosition::Top;
    plotArea->title()->setPosition(position);

    auto* plot = new XYCurve("Plot");
    plot->setLineType(XYCurve::LineType::NoLine);
    plot->setPlotType(Plot::PlotType::Scatter);
    plot->setXColumn(spreadsheet->column(0));
    plot->setYColumn(spreadsheet->column(1));
    plotArea->addChild(plot);

    auto* fit = new XYFitCurve("Fit to 'Plot'");

    fit->initFitData(XYAnalysisCurve::AnalysisAction::FitCustom);
    XYFitCurve::FitData fitData = fit->fitData();

    fitData.modelCategory = nsl_fit_model_custom;
    fitData.modelType = 0;
    fitData.model = QStringLiteral("B0 + B1*x");
    fitData.paramNames = {"B0", "B1"};
    fitData.paramNamesUtf8 = {"B0", "B1"};
    fitData.paramStartValues = {1, 1};
    fitData.paramFixed.fill(false, fitData.paramNames.size());
    fitData.paramLowerLimits.fill(-std::numeric_limits<double>::max(), fitData.paramNames.size());
    fitData.paramUpperLimits.fill(std::numeric_limits<double>::max(), fitData.paramNames.size());

    XYFitCurve::initFitData(fitData);

    fit->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
    fit->setDataSourceCurve(plot);

    fit->initStartValues(fitData);
    fit->setFitData(fitData);
    fit->recalculate();

    plotArea->addChild(fit);

    fit->recalculate();

    // #####################################################################################################################################################################

    AsciiFilter filter2;

    auto p2 = filter2.properties();
	p2.headerEnabled = false;
	filter2.setProperties(p2);

    auto* spreadsheet2 = new Spreadsheet("Pontius Data");

    lld->addChild(spreadsheet2);

    filter2.readDataFromFile(QStringLiteral("NIST - Linear Regression/pontius_data.txt"), spreadsheet2, AbstractFileFilter::ImportMode::Replace);

    if (!filter2.lastError().isEmpty()) {
		std::cout << "Import error: " << filter2.lastError().toStdString() << std::endl;
		return -1;
	}

    auto* plotArea2 = new CartesianPlot("Plot Area2");
	plotArea2->setType(CartesianPlot::Type::FourAxes);

    for (Axis* axis : plotArea2->children<Axis>()) {
        axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        axis->minorGridLine()->setStyle(Qt::PenStyle::NoPen);
    }

    allFits->addChild(plotArea2);

    plotArea2->title()->setText(QStringLiteral("Pontius"));
    plotArea2->title()->setVerticalAlignment(WorksheetElement::VerticalAlignment::Top);
    plotArea2->title()->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    WorksheetElement::PositionWrapper position2;
    position2.point = {Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-0.5, Worksheet::Unit::Centimeter)};
    position2.verticalPosition = WorksheetElement::VerticalPosition::Top;
    plotArea2->title()->setPosition(position2);

    auto* plot2 = new XYCurve("Plot");
    plot2->setLineType(XYCurve::LineType::NoLine);
    plot2->setPlotType(Plot::PlotType::Scatter);
    plot2->setXColumn(spreadsheet2->column(0));
    plot2->setYColumn(spreadsheet2->column(1));
    plotArea2->addChild(plot2);
    
    auto* fit2 = new XYFitCurve("Fit to 'Plot'");

    fit2->initFitData(XYAnalysisCurve::AnalysisAction::FitCustom);
    XYFitCurve::FitData fitData2 = fit2->fitData();

    fitData2.modelCategory = nsl_fit_model_custom;
    fitData2.modelType = 0;
    fitData2.model = QStringLiteral("B0 + B1*x + B2*(x**2)");
    fitData2.paramNames = {"B0", "B1", "B2"};
    fitData2.paramNamesUtf8 = {"B0", "B1", "B2"};
    fitData2.paramStartValues = {0.1, 0.01, 0.02};
    fitData2.paramFixed.fill(false, fitData2.paramNames.size());
    fitData2.paramLowerLimits.fill(-std::numeric_limits<double>::max(), fitData2.paramNames.size());
    fitData2.paramUpperLimits.fill(std::numeric_limits<double>::max(), fitData2.paramNames.size());

    XYFitCurve::initFitData(fitData2);

    fit2->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
    fit2->setDataSourceCurve(plot2);

    fit2->initStartValues(fitData2);
    fit2->setFitData(fitData2);
    fit2->recalculate();

    plotArea2->addChild(fit2);

    fit2->recalculate();

    allFits->view()->show();

    // #####################################################################################################################################################################
    // #####################################################################################################################################################################
    // #####################################################################################################################################################################

    auto* ald = new Folder("Average Level of Difficulty");
    project.addChild(ald);

    auto* allFits2 = new Worksheet("All Fits2");
    ald->addChild(allFits2);

    allFits2->setUseViewSize(false);
    double w2 = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double h2 = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	allFits2->setPageRect(QRectF(0, 0, w2, h2));

    allFits2->setLayout(Worksheet::Layout::VerticalLayout);
    allFits2->setLayoutTopMargin(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    auto* wsTextLabel2 = new TextLabel("WS Text Label2", TextLabel::Type::General);
    wsTextLabel2->setText(QStringLiteral("Average Level of Difficulty"));

    wsTextLabel2->setVerticalAlignment(WorksheetElement::VerticalAlignment::Center);
    wsTextLabel2->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    
    wsTextLabel2->position().point.setX(Worksheet::convertToSceneUnits(-0.4, Worksheet::Unit::Centimeter));
    wsTextLabel2->position().point.setY(Worksheet::convertToSceneUnits(9.5, Worksheet::Unit::Centimeter));
    wsTextLabel2->setPosition(wsTextLabel2->position());

    allFits2->addChild(wsTextLabel2);

    AsciiFilter filter3;

    auto p3 = filter3.properties();
	p3.headerEnabled = false;
	filter3.setProperties(p3);

	auto* spreadsheet3 = new Spreadsheet("NoInt1 Data");

    ald->addChild(spreadsheet3);

    filter3.readDataFromFile(QStringLiteral("NIST - Linear Regression/noint1_data.txt"), spreadsheet3, AbstractFileFilter::ImportMode::Replace);

    if (!filter3.lastError().isEmpty()) {
		std::cout << "Import error: " << filter3.lastError().toStdString() << std::endl;
		return -1;
	}

    auto* plotArea3 = new CartesianPlot("Plot Area3");
	plotArea3->setType(CartesianPlot::Type::FourAxes);

    for (Axis* axis : plotArea3->children<Axis>()) {
        axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        axis->minorGridLine()->setStyle(Qt::PenStyle::NoPen);
    }

    allFits2->addChild(plotArea3);

    plotArea3->title()->setText(QStringLiteral("NoInt1"));
    plotArea3->title()->setVerticalAlignment(WorksheetElement::VerticalAlignment::Top);
    plotArea3->title()->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    WorksheetElement::PositionWrapper position3;
    position3.point = {Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-0.5, Worksheet::Unit::Centimeter)};
    position3.verticalPosition = WorksheetElement::VerticalPosition::Top;
    plotArea3->title()->setPosition(position3);

    auto* plot3 = new XYCurve("Plot3");
    plot3->setLineType(XYCurve::LineType::NoLine);
    plot3->setPlotType(Plot::PlotType::Scatter);
    plot3->setXColumn(spreadsheet3->column(0));
    plot3->setYColumn(spreadsheet3->column(1));
    plotArea3->addChild(plot3);

    auto* fit3 = new XYFitCurve("Fit to 'Plot'");

    fit3->initFitData(XYAnalysisCurve::AnalysisAction::FitCustom);
    XYFitCurve::FitData fitData3 = fit3->fitData();

    fitData3.modelCategory = nsl_fit_model_custom;
    fitData3.modelType = 0;
    fitData3.model = QStringLiteral("B1*x");
    fitData3.paramNames = {"B1"};
    fitData3.paramNamesUtf8 = {"B1"};
    fitData3.paramStartValues = {1};
    fitData3.paramFixed.fill(false, fitData3.paramNames.size());
    fitData3.paramLowerLimits.fill(-std::numeric_limits<double>::max(), fitData3.paramNames.size());
    fitData3.paramUpperLimits.fill(std::numeric_limits<double>::max(), fitData3.paramNames.size());

    XYFitCurve::initFitData(fitData3);

    fit3->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
    fit3->setDataSourceCurve(plot3);

    fit3->initStartValues(fitData3);
    fit3->setFitData(fitData3);
    fit3->recalculate();

    plotArea3->addChild(fit3);

    fit3->recalculate();

    // #####################################################################################################################################################################

    AsciiFilter filter4;

    auto p4 = filter4.properties();
	p4.headerEnabled = false;
	filter4.setProperties(p4);

	auto* spreadsheet4 = new Spreadsheet("NoInt2 Data");

    ald->addChild(spreadsheet4);

    filter4.readDataFromFile(QStringLiteral("NIST - Linear Regression/noint2_data.txt"), spreadsheet4, AbstractFileFilter::ImportMode::Replace);

    if (!filter4.lastError().isEmpty()) {
		std::cout << "Import error: " << filter4.lastError().toStdString() << std::endl;
		return -1;
	}

    auto* plotArea4 = new CartesianPlot("Plot Area4");
	plotArea4->setType(CartesianPlot::Type::FourAxes);

    for (Axis* axis : plotArea4->children<Axis>()) {
        axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        axis->minorGridLine()->setStyle(Qt::PenStyle::NoPen);
    }

    allFits2->addChild(plotArea4);

    plotArea4->title()->setText(QStringLiteral("NoInt2"));
    plotArea4->title()->setVerticalAlignment(WorksheetElement::VerticalAlignment::Top);
    plotArea4->title()->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    WorksheetElement::PositionWrapper position4;
    position4.point = {Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-0.5, Worksheet::Unit::Centimeter)};
    position4.verticalPosition = WorksheetElement::VerticalPosition::Top;
    plotArea4->title()->setPosition(position4);

    auto* plot4 = new XYCurve("Plot4");
    plot4->setLineType(XYCurve::LineType::NoLine);
    plot4->setPlotType(Plot::PlotType::Scatter);
    plot4->setXColumn(spreadsheet4->column(0));
    plot4->setYColumn(spreadsheet4->column(1));
    plotArea4->addChild(plot4);

    auto* fit4 = new XYFitCurve("Fit to 'Plot4'");

    fit4->initFitData(XYAnalysisCurve::AnalysisAction::FitCustom);
    XYFitCurve::FitData fitData4 = fit4->fitData();

    fitData4.modelCategory = nsl_fit_model_custom;
    fitData4.modelType = 0;
    fitData4.model = QStringLiteral("B1*x");
    fitData4.paramNames = {"B1"};
    fitData4.paramNamesUtf8 = {"B1"};
    fitData4.paramStartValues = {1};
    fitData4.paramFixed.fill(false, fitData4.paramNames.size());
    fitData4.paramLowerLimits.fill(-std::numeric_limits<double>::max(), fitData4.paramNames.size());
    fitData4.paramUpperLimits.fill(std::numeric_limits<double>::max(), fitData4.paramNames.size());

    XYFitCurve::initFitData(fitData4);

    fit4->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
    fit4->setDataSourceCurve(plot4);

    fit4->initStartValues(fitData4);
    fit4->setFitData(fitData4);
    fit4->recalculate();

    plotArea4->addChild(fit4);

    fit4->recalculate();

    allFits2->view()->show();

    // #####################################################################################################################################################################
    // #####################################################################################################################################################################
    // #####################################################################################################################################################################

    app.exec();
}
