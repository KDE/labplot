#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    AsciiFilter filter;

    auto p = filter.properties();
	p.headerEnabled = false;
	filter.setProperties(p);

	auto* spreadsheet = new Spreadsheet("Spreadsheet");
    project.addChild(spreadsheet);

    filter.readDataFromFile(QStringLiteral("Data = Smooth + Rough/data.txt"), spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* worksheet = new Worksheet("Plot data from Spreadsheet");
    project.addChild(worksheet);

    worksheet->setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	worksheet->setPageRect(QRectF(0, 0, w, h));

    worksheet->setLayout(Worksheet::Layout::VerticalLayout);
    worksheet->setLayoutTopMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.4, Worksheet::Unit::Centimeter));
	worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.4, Worksheet::Unit::Centimeter));
	worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(0.4, Worksheet::Unit::Centimeter));

    worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0.4, Worksheet::Unit::Centimeter));

    QFont fo7;
    fo7.setPointSizeF(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Point));

    QFont fo10;
    fo10.setPointSizeF(10);
    QTextEdit te10;
    te10.setFont(fo10);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* plotArea1 = new CartesianPlot("Plot data from Spreadsheet");
	plotArea1->setType(CartesianPlot::Type::FourAxes);

    plotArea1->setSymmetricPadding(false);
    plotArea1->setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea1->setVerticalPadding(Worksheet::convertToSceneUnits(0.8, Worksheet::Unit::Centimeter));
    plotArea1->setRightPadding(Worksheet::convertToSceneUnits(0.8, Worksheet::Unit::Centimeter));
    plotArea1->setBottomPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border1 = plotArea1->plotArea()->borderType();
    border1.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea1->plotArea()->setBorderType(border1);

    plotArea1->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);

    Range<double> rangeY1 = plotArea1->range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY1.setRange(300, 650);
    plotArea1->setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY1);
    plotArea1->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);

    for (Axis* axis : plotArea1->children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            te10.setText(QStringLiteral("index"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DashLine);
            axis->setLabelsFont(fo7);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            te10.setText(QStringLiteral("data"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DashLine);
            axis->setLabelsFont(fo7);
        }
    }

    worksheet->addChild(plotArea1);

    auto* config11 = new XYCurve("data");
    plotArea1->addChild(config11);
    config11->setPlotType(Plot::PlotType::Scatter);
    config11->setXColumn(spreadsheet->column(0));
    config11->setYColumn(spreadsheet->column(1));
    config11->setLineType(XYCurve::LineType::NoLine);
    config11->symbol()->setStyle(Symbol::Style::Circle);
    config11->symbol()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    config11->setValuesType(XYCurve::ValuesType::NoValues);
    config11->background()->setPosition(Background::Position::No);

    auto* config12 = new XYSmoothCurve("smooth 1st iteration");
    plotArea1->addChild(config12);
    config12->setDataSourceType(XYFitCurve::DataSourceType::Curve);
    config12->setDataSourceCurve(config11);
    XYSmoothCurve::SmoothData sData11 = config12->smoothData();
    sData11.type = nsl_smooth_type_moving_average;
    sData11.points = 5;
    sData11.weight = nsl_smooth_weight_uniform;
    sData11.mode = nsl_smooth_pad_none;
    sData11.autoRange = true;
    config12->setSmoothData(sData11);
    config12->setLineInterpolationPointsCount(1);
    config12->symbol()->setStyle(Symbol::Style::NoSymbols);
    config12->setValuesType(XYCurve::ValuesType::NoValues);
    config12->background()->setPosition(Background::Position::No);

    auto* config13 = new XYSmoothCurve("smooth 2nd iteration");
    plotArea1->addChild(config13);
    config13->setDataSourceType(XYFitCurve::DataSourceType::Curve);
    config13->setDataSourceCurve(config12);
    XYSmoothCurve::SmoothData sData12 = config13->smoothData();
    sData12.type = nsl_smooth_type_moving_average;
    sData12.points = 5;
    sData12.weight = nsl_smooth_weight_uniform;
    sData12.mode = nsl_smooth_pad_none;
    sData12.autoRange = true;
    config13->setSmoothData(sData12);
    config13->setLineInterpolationPointsCount(1);
    config13->symbol()->setStyle(Symbol::Style::NoSymbols);
    config13->setValuesType(XYCurve::ValuesType::NoValues);
    config13->background()->setPosition(Background::Position::No);

    auto* legend1 = new CartesianPlotLegend("Legend1");
    plotArea1->addChild(legend1);
    legend1->setLabelFont(fo7);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* plotArea2 = new CartesianPlot("xy-plot");
	plotArea2->setType(CartesianPlot::Type::FourAxes);

    plotArea2->setSymmetricPadding(false);
    plotArea2->setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea2->setVerticalPadding(Worksheet::convertToSceneUnits(0.8, Worksheet::Unit::Centimeter));
    plotArea2->setRightPadding(Worksheet::convertToSceneUnits(0.8, Worksheet::Unit::Centimeter));
    plotArea2->setBottomPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border2 = plotArea2->plotArea()->borderType();
    border2.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea2->plotArea()->setBorderType(border2);

    plotArea2->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);

    Range<double> rangeY2 = plotArea2->range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY2.setRange(-120, 100);
    plotArea2->setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY2);
    plotArea2->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);

    for (Axis* axis : plotArea2->children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            te10.setText(QStringLiteral("index"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DashLine);
            axis->setLabelsFont(fo7);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            te10.setText(QStringLiteral("rough"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DashLine);
            axis->setLabelsFont(fo7);
        }
    }

    worksheet->addChild(plotArea2);

    auto* config21 = new XYCurve("rough 1st iteration");
    plotArea2->addChild(config21);
    config21->setPlotType(Plot::PlotType::Scatter);
    config21->setXColumn(spreadsheet->column(0));
    config21->setYColumn(config12->roughsColumn());
    config21->setLineType(XYCurve::LineType::Line);
    config21->symbol()->setStyle(Symbol::Style::NoSymbols);
    config21->setValuesType(XYCurve::ValuesType::NoValues);
    config21->background()->setPosition(Background::Position::No);

    auto* config22 = new XYCurve("rough 2nd iteration");
    plotArea2->addChild(config22);
    config22->setPlotType(Plot::PlotType::Scatter);
    config22->setXColumn(spreadsheet->column(0));
    config22->setYColumn(config13->roughsColumn());
    config22->setLineType(XYCurve::LineType::Line);
    config22->symbol()->setStyle(Symbol::Style::NoSymbols);
    config22->setValuesType(XYCurve::ValuesType::NoValues);
    config22->background()->setPosition(Background::Position::No);

    auto* legend2 = new CartesianPlotLegend("Legend2");
    plotArea2->addChild(legend2);
    legend2->setLabelFont(fo7);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    worksheet->view()->show();

    app.exec();
}
