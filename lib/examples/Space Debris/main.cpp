#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    AsciiFilter filter1;

    auto p1 = filter1.properties();
	p1.headerEnabled = false;
	filter1.setProperties(p1);

	Spreadsheet spreadsheet1("data");
    project.addChild(&spreadsheet1);

    filter1.readDataFromFile(QStringLiteral("Space Debris/Breakups.txt"), &spreadsheet1, AbstractFileFilter::ImportMode::Replace);

    if (!filter1.lastError().isEmpty()) {
		std::cout << "Import error: " << filter1.lastError().toStdString() << std::endl;
		return -1;
	}

    AsciiFilter filter2;

    auto p2 = filter2.properties();
	p2.headerEnabled = false;
	filter2.setProperties(p2);

	Spreadsheet spreadsheet2("data");
    project.addChild(&spreadsheet2);

    filter2.readDataFromFile(QStringLiteral("Space Debris/LEO.txt"), &spreadsheet2, AbstractFileFilter::ImportMode::Replace);

    if (!filter2.lastError().isEmpty()) {
		std::cout << "Import error: " << filter2.lastError().toStdString() << std::endl;
		return -1;
	}

    AsciiFilter filter3;

    auto p3 = filter3.properties();
	p3.headerEnabled = false;
	filter3.setProperties(p3);

	Spreadsheet spreadsheet3("data");
    project.addChild(&spreadsheet3);

    filter3.readDataFromFile(QStringLiteral("Space Debris/GEO.txt"), &spreadsheet3, AbstractFileFilter::ImportMode::Replace);

    if (!filter3.lastError().isEmpty()) {
		std::cout << "Import error: " << filter3.lastError().toStdString() << std::endl;
		return -1;
	}

    QTextEdit te;
    QFont fo6;
    fo6.setPointSizeF(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    Worksheet worksheet("Plot");
    project.addChild(&worksheet);

    worksheet.setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(25, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(23, Worksheet::Unit::Centimeter);
	worksheet.setPageRect(QRectF(0, 0, w, h));

    worksheet.setLayout(Worksheet::Layout::NoLayout);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea1("Plot data from Spreadsheet");
	plotArea1.setType(CartesianPlot::Type::FourAxes);

    plotArea1.setRect({Worksheet::convertToSceneUnits(0.3, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0.3, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(24.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(7, Worksheet::Unit::Centimeter)});

    plotArea1.setSymmetricPadding(false);
    plotArea1.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea1.setVerticalPadding(Worksheet::convertToSceneUnits(1.1, Worksheet::Unit::Centimeter));
    plotArea1.setRightPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea1.setBottomPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border1 = plotArea1.plotArea()->borderType();
    border1.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea1.plotArea()->setBorderType(border1);
    plotArea1.plotArea()->borderLine()->setWidth(0);

    te.clear();
    te.setFontPointSize(10);
    te.append(QStringLiteral("Breakups per Year"));
    plotArea1.title()->setText(te.toHtml());

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);

    Range<double> rangeY1 = plotArea1.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY1.setRange(0, 10);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY1);
    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);

    for (Axis* axis : plotArea1.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            te.clear();
            te.setFontPointSize(8);
            te.append(QStringLiteral("Year"));
            axis->title()->setText(te.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo6);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            te.clear();
            te.setFontPointSize(8);
            te.append(QStringLiteral("breakups"));
            axis->title()->setText(te.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo6);
        }
    }

    worksheet.addChild(&plotArea1);

    XYCurve config11("2");
    plotArea1.addChild(&config11);
    config11.setPlotType(Plot::PlotType::Scatter);
    config11.dropLine()->setDropLineType(XYCurve::DropLineType::X);
    config11.dropLine()->setStyle(Qt::PenStyle::SolidLine);
    config11.setXColumn(spreadsheet1.column(0));
    config11.setYColumn(spreadsheet1.column(1));
    config11.setLineType(XYCurve::LineType::NoLine);
    config11.symbol()->setStyle(Symbol::Style::Circle);
    config11.symbol()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    config11.setValuesType(XYCurve::ValuesType::NoValues);
    config11.background()->setPosition(Background::Position::No);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea2("xy-plot");
	plotArea2.setType(CartesianPlot::Type::FourAxes);

    plotArea2.setRect({Worksheet::convertToSceneUnits(0.3, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(7.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(17, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(7, Worksheet::Unit::Centimeter)});

    plotArea2.setSymmetricPadding(false);
    plotArea2.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea2.setVerticalPadding(Worksheet::convertToSceneUnits(1.3, Worksheet::Unit::Centimeter));
    plotArea2.setRightPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea2.setBottomPadding(Worksheet::convertToSceneUnits(1.3, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border2 = plotArea2.plotArea()->borderType();
    border2.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea2.plotArea()->setBorderType(border2);
    plotArea2.plotArea()->borderLine()->setWidth(0);

    te.clear();
    te.setFontPointSize(10);
    te.append(QStringLiteral("Near Earth Altitude Population"));
    plotArea2.title()->setText(te.toHtml());

    Range<double> rangeX2 = plotArea2.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX2.setRange(0, 2000);
    plotArea2.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX2);
    plotArea2.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);

    Range<double> rangeY2 = plotArea2.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY2.setRange(0, 7);
    plotArea2.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY2);
    plotArea2.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);

    for (Axis* axis : plotArea2.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            te.clear();
            te.setFontPointSize(8);
            te.append(QStringLiteral("Altitude [km]"));
            axis->title()->setText(te.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo6);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            te.clear();
            te.setFontPointSize(8);
            te.append(QStringLiteral("density [10-8km-3]"));
            axis->title()->setText(te.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo6);
        }
    }

    worksheet.addChild(&plotArea2);

    XYCurve config21("xy-curve");
    plotArea2.addChild(&config21);
    config21.setPlotType(Plot::PlotType::Line);
    config21.setXColumn(spreadsheet2.column(0));
    config21.setYColumn(spreadsheet2.column(1));
    config21.symbol()->setStyle(Symbol::Style::NoSymbols);
    config21.setValuesType(XYCurve::ValuesType::NoValues);
    config21.background()->setType(Background::Type::Color);
    config21.background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config21.background()->setPosition(Background::Position::Below);
    config21.background()->setFirstColor(QColor(70, 70, 70));
    config21.background()->setSecondColor(QColor(255, 255, 255));
    config21.background()->setOpacity(0.4);

    ReferenceLine rl21(&plotArea2, QStringLiteral("reference line"));
    plotArea2.addChild(&rl21);
    rl21.setOrientation(ReferenceLine::Orientation::Vertical);
    rl21.setPositionLogical({775, 0});
    rl21.line()->setWidth(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point));
    rl21.line()->setOpacity(1);
    rl21.retransform();

    ReferenceLine rl22(&plotArea2, QStringLiteral("reference line 1"));
    plotArea2.addChild(&rl22);
    rl22.setOrientation(ReferenceLine::Orientation::Vertical);
    rl22.setPositionLogical({840, 0});
    rl22.line()->setWidth(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point));
    rl22.line()->setOpacity(1);
    rl22.retransform();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea3("xy-plot 1");
	plotArea3.setType(CartesianPlot::Type::FourAxes);

    plotArea3.setRect({Worksheet::convertToSceneUnits(0.3, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(14.7, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(17, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(7, Worksheet::Unit::Centimeter)});

    plotArea3.setSymmetricPadding(false);
    plotArea3.setHorizontalPadding(Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter));
    plotArea3.setVerticalPadding(Worksheet::convertToSceneUnits(1.3, Worksheet::Unit::Centimeter));
    plotArea3.setRightPadding(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    plotArea3.setBottomPadding(Worksheet::convertToSceneUnits(1.3, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border3 = plotArea3.plotArea()->borderType();
    border3.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea3.plotArea()->setBorderType(border3);
    plotArea3.plotArea()->borderLine()->setWidth(0);

    te.clear();
    te.setFontPointSize(10);
    te.append(QStringLiteral("Geosynchronous Altitude Population"));
    plotArea3.title()->setText(te.toHtml());

    Range<double> rangeX3 = plotArea3.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX3.setRange(34, 37);
    plotArea3.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX3);
    plotArea3.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);

    Range<double> rangeY3 = plotArea3.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY3.setScale(RangeT::Scale::Log10);
    rangeY3.setRange(1, 10000);
    plotArea3.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY3);
    plotArea3.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);

    for (Axis* axis : plotArea3.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            te.clear();
            te.setFontPointSize(8);
            te.append(QStringLiteral("Altitude [km]"));
            axis->title()->setText(te.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo6);
            axis->setScalingFactor(1000);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            te.clear();
            te.setFontPointSize(8);
            te.append(QStringLiteral("density [10-12km-3]"));
            axis->title()->setText(te.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo6);
        }
    }

    worksheet.addChild(&plotArea3);

    XYCurve config31("xy-curve");
    plotArea3.addChild(&config31);
    config31.setPlotType(Plot::PlotType::Line);
    config31.setXColumn(spreadsheet3.column(0));
    config31.setYColumn(spreadsheet3.column(1));
    config31.symbol()->setStyle(Symbol::Style::NoSymbols);
    config31.setValuesType(XYCurve::ValuesType::NoValues);
    config31.background()->setType(Background::Type::Color);
    config31.background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config31.background()->setPosition(Background::Position::Below);
    config31.background()->setFirstColor(QColor(70, 70, 70));
    config31.background()->setSecondColor(QColor(255, 255, 255));
    config31.background()->setOpacity(0.4);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    TextLabel textLabel1("Text Label");
    worksheet.addChild(&textLabel1);
    te.clear();
    te.setFontPointSize(5);
    te.append("Sources:\n[1] https://orbitaldebris.jsc.nasa.gov/library/20180008451.pdf - History of On-Orbit Satellite Fragmentations\n[2] https://orbitaldebris.jsc.nasa.gov/photo-gallery/");
    textLabel1.setText(te.toHtml());
    textLabel1.setPosition({Worksheet::convertToSceneUnits(-5.9, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-10.8, Worksheet::Unit::Centimeter)});

    Image image1("LEO");
    worksheet.addChild(&image1);
    image1.setFileName("Space Debris/LEO.jpg");
    image1.setEmbedded(false);
    image1.setOpacity(1);
    image1.setWidth(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Centimeter));
    image1.setHeight(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Centimeter));
    image1.setKeepRatio(true);
    image1.setPosition({Worksheet::convertToSceneUnits(8.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter)});

    Image image2("GEO");
    worksheet.addChild(&image2);
    image2.setFileName("Space Debris/GEO.jpg");
    image2.setEmbedded(false);
    image2.setOpacity(1);
    image2.setWidth(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Centimeter));
    image2.setHeight(Worksheet::convertToSceneUnits(5.6, Worksheet::Unit::Centimeter));
    image2.setKeepRatio(true);
    image2.setPosition({Worksheet::convertToSceneUnits(8.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-6.5, Worksheet::Unit::Centimeter)});

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    worksheet.setTheme("Dark");

    worksheet.view()->show();

    app.exec();
}
