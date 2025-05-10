#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    AsciiFilter filter1;

    auto p1 = filter1.properties();
	p1.headerEnabled = false;
	filter1.setProperties(p1);

	Spreadsheet spreadsheet1("IPCE");
    project.addChild(&spreadsheet1);

    filter1.readDataFromFile(QStringLiteral("Multi-Ranges/IPCE.txt"), &spreadsheet1, AbstractFileFilter::ImportMode::Replace);

    if (!filter1.lastError().isEmpty()) {
		std::cout << "Import error: " << filter1.lastError().toStdString() << std::endl;
		return -1;
	}

    AsciiFilter filter2;

    auto p2 = filter2.properties();
	p2.headerEnabled = false;
	filter2.setProperties(p2);

	Spreadsheet spreadsheet2("Current");
    project.addChild(&spreadsheet2);

    filter2.readDataFromFile(QStringLiteral("Multi-Ranges/Current.txt"), &spreadsheet2, AbstractFileFilter::ImportMode::Replace);

    if (!filter2.lastError().isEmpty()) {
		std::cout << "Import error: " << filter2.lastError().toStdString() << std::endl;
		return -1;
	}

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    Worksheet worksheet1("single-range demo");
    project.addChild(&worksheet1);

    worksheet1.setUseViewSize(false);
    double w1 = Worksheet::convertToSceneUnits(15, Worksheet::Unit::Centimeter);
	double h1 = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);
	worksheet1.setPageRect(QRectF(0, 0, w1, h1));

    worksheet1.setLayout(Worksheet::Layout::VerticalLayout);
    worksheet1.setLayoutTopMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet1.setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet1.setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet1.setLayoutRightMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    worksheet1.setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));
    worksheet1.setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));

    CartesianPlot plotArea1("IPCE1");
	plotArea1.setType(CartesianPlot::Type::FourAxes);

    plotArea1.setSymmetricPadding(false);
    plotArea1.setHorizontalPadding(Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter));
    plotArea1.setVerticalPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea1.setRightPadding(Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter));
    plotArea1.setBottomPadding(Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border1 = plotArea1.plotArea()->borderType();
    border1.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea1.plotArea()->setBorderType(border1);

    for (Axis* axis : plotArea1.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            axis->title()->setText(QStringLiteral("Wavelength (nm)"));
        } else {
            axis->title()->setText(QStringLiteral(""));
            axis->title()->setVisible(false);
        }
    }

    worksheet1.addChild(&plotArea1);

    XYCurve config11("IPCE");
    plotArea1.addChild(&config11);
    config11.setPlotType(Plot::PlotType::Line);
    config11.setXColumn(spreadsheet1.column(0));
    config11.setYColumn(spreadsheet1.column(1));
    config11.setLineType(XYCurve::LineType::Line);
    config11.symbol()->setStyle(Symbol::Style::NoSymbols);
    config11.setValuesType(XYCurve::ValuesType::NoValues);
    config11.background()->setType(Background::Type::Color);
    config11.background()->setPosition(Background::Position::Below);
    config11.background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config11.background()->setFirstColor(config11.color());
    config11.background()->setSecondColor("white");
    config11.background()->setOpacity(0.4);

    XYCurve config12("Current Density");
    plotArea1.addChild(&config12);
    config12.setPlotType(Plot::PlotType::Line);
    config12.setXColumn(spreadsheet2.column(0));
    config12.setYColumn(spreadsheet2.column(1));
    config12.setLineType(XYCurve::LineType::Line);
    config12.symbol()->setStyle(Symbol::Style::NoSymbols);
    config12.setValuesType(XYCurve::ValuesType::NoValues);
    config12.background()->setType(Background::Type::Color);
    config12.background()->setPosition(Background::Position::Below);
    config12.background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config12.background()->setFirstColor(config12.color());
    config12.background()->setSecondColor("white");
    config12.background()->setOpacity(0.4);

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX1 = plotArea1.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX1.setRange(350, 800);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX1);

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY1 = plotArea1.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY1.setRange(0, 80);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY1);

    worksheet1.view()->show();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    Worksheet worksheet2("multi-range demo");
    project.addChild(&worksheet2);

    worksheet2.setUseViewSize(false);
    double w2 = Worksheet::convertToSceneUnits(15, Worksheet::Unit::Centimeter);
	double h2 = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);
	worksheet2.setPageRect(QRectF(0, 0, w2, h2));

    worksheet2.setLayout(Worksheet::Layout::VerticalLayout);
    worksheet2.setLayoutTopMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet2.setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet2.setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet2.setLayoutRightMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    worksheet2.setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));
    worksheet2.setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));

    CartesianPlot plotArea2("IPCE2");
	plotArea2.setType(CartesianPlot::Type::FourAxes);

    plotArea2.setSymmetricPadding(false);
    plotArea2.setHorizontalPadding(Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter));
    plotArea2.setVerticalPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea2.setRightPadding(Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter));
    plotArea2.setBottomPadding(Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border2 = plotArea2.plotArea()->borderType();
    border2.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea2.plotArea()->setBorderType(border2);

    Range<double> rangeX2 = plotArea2.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX2.setRange(350, 800);
    plotArea2.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX2);

    Range<double> rangeY21 = plotArea2.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY21.setRange(0, 80);
    plotArea2.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY21);

    plotArea2.addYRange();

    Range<double> rangeY22 = plotArea2.range(CartesianCoordinateSystem::Dimension::Y, 1);
    rangeY22.setRange(0, 12);
    plotArea2.setRange(CartesianCoordinateSystem::Dimension::Y, 1, rangeY22);

    plotArea2.enableAutoScale(CartesianCoordinateSystem::Dimension::X, -1, false);
    plotArea2.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, -1, false);

    plotArea2.addCoordinateSystem();
    plotArea2.setCoordinateSystemRangeIndex(1, CartesianCoordinateSystem::Dimension::Y, 1);

    worksheet2.addChild(&plotArea2);

    for (Axis* axis : plotArea2.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            axis->title()->setText(QStringLiteral("Wavelength (nm)"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("IPCE (%)"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Right) {
            axis->title()->setText(QStringLiteral("Current density (mA cm−2)"));
            axis->setTitleOffsetX(Worksheet::convertToSceneUnits(42, Worksheet::Unit::Point));
            axis->setMajorTicksDirection(Axis::TicksDirection(Axis::TicksFlags::ticksIn));
            axis->setLabelsPosition(Axis::LabelsPosition::In);
			axis->setCoordinateSystemIndex(1);
        } else {
            axis->title()->setText(QStringLiteral(""));
            axis->title()->setVisible(false);
        }
    }

    XYCurve config21("IPCE");
    plotArea2.addChild(&config21);
    config21.setPlotType(Plot::PlotType::Line);
    config21.setXColumn(spreadsheet1.column(0));
    config21.setYColumn(spreadsheet1.column(1));
    config21.setLineType(XYCurve::LineType::Line);
    config21.symbol()->setStyle(Symbol::Style::NoSymbols);
    config21.setValuesType(XYCurve::ValuesType::NoValues);
    config21.background()->setType(Background::Type::Color);
    config21.background()->setPosition(Background::Position::Below);
    config21.background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config21.background()->setFirstColor(config21.color());
    config21.background()->setSecondColor("white");
    config21.background()->setOpacity(0.4);
    for (Axis* axis : plotArea2.children<Axis>()) {
        if (axis->coordinateSystemIndex() == config21.coordinateSystemIndex()) {
            axis->setLabelsColor(config21.color());
            axis->majorTicksLine()->setColor(config21.color());
            axis->minorTicksLine()->setColor(config21.color());
            axis->line()->setColor(config21.color());
        }
    }

    XYCurve config22("Current Density");
    plotArea2.addChild(&config22);
    config22.setPlotType(Plot::PlotType::Line);
    config22.setXColumn(spreadsheet2.column(0));
    config22.setYColumn(spreadsheet2.column(1));
    config22.setLineType(XYCurve::LineType::Line);
    config22.symbol()->setStyle(Symbol::Style::NoSymbols);
    config22.setValuesType(XYCurve::ValuesType::NoValues);
    config22.background()->setType(Background::Type::Color);
    config22.background()->setPosition(Background::Position::Below);
    config22.background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config22.background()->setFirstColor(config22.color());
    config22.background()->setSecondColor("white");
    config22.background()->setOpacity(0.4);
    config22.setCoordinateSystemIndex(1);
    for (Axis* axis : plotArea2.children<Axis>()) {
        if (axis->coordinateSystemIndex() == config22.coordinateSystemIndex()) {
            axis->setLabelsColor(config22.color());
            axis->majorTicksLine()->setColor(config22.color());
            axis->minorTicksLine()->setColor(config22.color());
            axis->line()->setColor(config22.color());
        }
    }

    worksheet2.view()->show();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    Worksheet worksheet3("multi-range demo, alternative");
    project.addChild(&worksheet3);

    worksheet3.setUseViewSize(false);
    double w3 = Worksheet::convertToSceneUnits(15, Worksheet::Unit::Centimeter);
	double h3 = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);
	worksheet3.setPageRect(QRectF(0, 0, w3, h3));

    worksheet3.setLayout(Worksheet::Layout::VerticalLayout);
    worksheet3.setLayoutTopMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet3.setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet3.setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet3.setLayoutRightMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    worksheet3.setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));
    worksheet3.setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));

    CartesianPlot plotArea3("IPCE3");
	plotArea3.setType(CartesianPlot::Type::FourAxes);

    plotArea3.setSymmetricPadding(false);
    plotArea3.setHorizontalPadding(Worksheet::convertToSceneUnits(3.5, Worksheet::Unit::Centimeter));
    plotArea3.setVerticalPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea3.setRightPadding(Worksheet::convertToSceneUnits(0.6, Worksheet::Unit::Centimeter));
    plotArea3.setBottomPadding(Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border3 = plotArea3.plotArea()->borderType();
    border3.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea3.plotArea()->setBorderType(border3);

    Range<double> rangeX3 = plotArea3.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX3.setRange(350, 800);
    plotArea3.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX3);

    Range<double> rangeY31 = plotArea3.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY31.setRange(0, 80);
    plotArea3.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY31);

    plotArea3.addYRange();

    Range<double> rangeY32 = plotArea3.range(CartesianCoordinateSystem::Dimension::Y, 1);
    rangeY32.setRange(0, 12);
    plotArea3.setRange(CartesianCoordinateSystem::Dimension::Y, 1, rangeY32);

    plotArea3.enableAutoScale(CartesianCoordinateSystem::Dimension::X, -1, false);
    plotArea3.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, -1, false);

    plotArea3.addCoordinateSystem();
    plotArea3.setCoordinateSystemRangeIndex(1, CartesianCoordinateSystem::Dimension::Y, 1);

    worksheet3.addChild(&plotArea3);

    for (Axis* axis : plotArea3.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            axis->title()->setText(QStringLiteral("Wavelength (nm)"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("IPCE (%)"));
            axis->setOffset(Worksheet::convertToSceneUnits(-0.5, Worksheet::Unit::Centimeter));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Right) {
            axis->title()->setText(QStringLiteral("Current density (mA cm−2)"));
            axis->setPosition(Axis::Position::Left);
            axis->setOffset(Worksheet::convertToSceneUnits(-2.2, Worksheet::Unit::Centimeter));
            axis->setTitleOffsetX(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Point));
            axis->setMajorTicksDirection(Axis::TicksDirection(Axis::TicksFlags::ticksIn));
            axis->setLabelsPosition(Axis::LabelsPosition::Out);
            axis->setLabelsOffset(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
			axis->setCoordinateSystemIndex(1);
        } else {
            axis->title()->setText(QStringLiteral(""));
            axis->title()->setVisible(false);
            axis->setVisible(false);
        }
    }

    XYCurve config31("IPCE");
    plotArea3.addChild(&config31);
    config31.setPlotType(Plot::PlotType::Line);
    config31.setXColumn(spreadsheet1.column(0));
    config31.setYColumn(spreadsheet1.column(1));
    config31.setLineType(XYCurve::LineType::Line);
    config31.symbol()->setStyle(Symbol::Style::NoSymbols);
    config31.setValuesType(XYCurve::ValuesType::NoValues);
    config31.background()->setType(Background::Type::Color);
    config31.background()->setPosition(Background::Position::Below);
    config31.background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config31.background()->setFirstColor(config31.color());
    config31.background()->setSecondColor("white");
    config31.background()->setOpacity(0.4);
    for (Axis* axis : plotArea3.children<Axis>()) {
        if (axis->coordinateSystemIndex() == config31.coordinateSystemIndex()) {
            axis->setLabelsColor(config31.color());
            axis->majorTicksLine()->setColor(config31.color());
            axis->minorTicksLine()->setColor(config31.color());
            axis->line()->setColor(config31.color());
        }
    }

    XYCurve config32("Current Density");
    plotArea3.addChild(&config32);
    config32.setPlotType(Plot::PlotType::Line);
    config32.setXColumn(spreadsheet2.column(0));
    config32.setYColumn(spreadsheet2.column(1));
    config32.setLineType(XYCurve::LineType::Line);
    config32.symbol()->setStyle(Symbol::Style::NoSymbols);
    config32.setValuesType(XYCurve::ValuesType::NoValues);
    config32.background()->setType(Background::Type::Color);
    config32.background()->setPosition(Background::Position::Below);
    config32.background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config32.background()->setFirstColor(config32.color());
    config32.background()->setSecondColor("white");
    config32.background()->setOpacity(0.4);
    config32.setCoordinateSystemIndex(1);
    for (Axis* axis : plotArea3.children<Axis>()) {
        if (axis->coordinateSystemIndex() == config32.coordinateSystemIndex()) {
            axis->setLabelsColor(config32.color());
            axis->majorTicksLine()->setColor(config32.color());
            axis->minorTicksLine()->setColor(config32.color());
            axis->line()->setColor(config32.color());
        }
    }

    worksheet3.view()->show();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    app.exec();
}
