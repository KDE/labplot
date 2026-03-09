#include <QApplication>
#include <cassert>

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

    filter.readDataFromFile(QStringLiteral("Helium Rydberg Spectrum/data.txt"), spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    auto* worksheet = new Worksheet("Worksheet");
    project.addChild(worksheet);

    worksheet->setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(16.3, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(12.7, Worksheet::Unit::Centimeter);
	worksheet->setPageRect(QRectF(0, 0, w, h));

    double ms = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
    worksheet->setLayout(Worksheet::Layout::HorizontalLayout);
    worksheet->setLayoutTopMargin(ms);
	worksheet->setLayoutBottomMargin(ms);
	worksheet->setLayoutLeftMargin(ms);
	worksheet->setLayoutRightMargin(ms);

    worksheet->setLayoutHorizontalSpacing(ms);
    worksheet->setLayoutVerticalSpacing(ms);

    worksheet->setTheme(QStringLiteral("Monokai"));

    auto* plotArea = new CartesianPlot("Plot Area");
	plotArea->setType(CartesianPlot::Type::FourAxes);
    plotArea->title()->setText(QStringLiteral("Rydberg Spectrum of Helium"));
    CartesianPlot::BorderType border = plotArea->borderType();
    border.setFlag(CartesianPlot::BorderTypeFlags::BorderLeft, true);
	border.setFlag(CartesianPlot::BorderTypeFlags::BorderTop, true);
	border.setFlag(CartesianPlot::BorderTypeFlags::BorderRight, true);
	border.setFlag(CartesianPlot::BorderTypeFlags::BorderBottom, true);
    plotArea->setBorderType(border);
    plotArea->setSymmetricPadding(false);
    plotArea->setHorizontalPadding(Worksheet::convertToSceneUnits(2.1, Worksheet::Unit::Centimeter));
    plotArea->setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea->setRightPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea->setBottomPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    worksheet->addChild(plotArea);

    for (Axis* axis : plotArea->children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->title()->setText(QStringLiteral("wavelength (nm)"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("Count"));
        }
    }

    auto* config1 = new XYCurve("config1");
    plotArea->addChild(config1);
    config1->setPlotType(Plot::PlotType::Line);
    config1->setXColumn(spreadsheet->column(0));
    config1->setYColumn(spreadsheet->column(1));
    config1->symbol()->setStyle(Symbol::Style::NoSymbols);
    config1->setValuesType(XYCurve::ValuesType::NoValues);
    config1->background()->setType(Background::Type::Color);
    config1->background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config1->background()->setPosition(Background::Position::Below);
    config1->background()->setFirstColor(config1->color());
    config1->background()->setSecondColor(QColor(255, 255, 255));
    config1->background()->setOpacity(0.4);

    auto* config2 = new XYCurve("config2");
    plotArea->addChild(config2);
    config2->setPlotType(Plot::PlotType::Line);
    config2->setXColumn(spreadsheet->column(2));
    config2->setYColumn(spreadsheet->column(3));
    config2->symbol()->setStyle(Symbol::Style::NoSymbols);
    config2->setValuesType(XYCurve::ValuesType::NoValues);
    config2->background()->setType(Background::Type::Color);
    config2->background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config2->background()->setPosition(Background::Position::Below);
    config2->background()->setFirstColor(config2->color());
    config2->background()->setSecondColor(QColor(255, 255, 255));
    config2->background()->setOpacity(0.4);

    auto* config3 = new XYCurve("config3");
    plotArea->addChild(config3);
    config3->setPlotType(Plot::PlotType::Line);
    config3->setXColumn(spreadsheet->column(4));
    config3->setYColumn(spreadsheet->column(5));
    config3->symbol()->setStyle(Symbol::Style::NoSymbols);
    config3->setValuesType(XYCurve::ValuesType::NoValues);
    config3->background()->setType(Background::Type::Color);
    config3->background()->setColorStyle(Background::ColorStyle::VerticalLinearGradient);
    config3->background()->setPosition(Background::Position::Below);
    config3->background()->setFirstColor(config3->color());
    config3->background()->setSecondColor(QColor(255, 255, 255));
    config3->background()->setOpacity(0.4);
    config3->setVisible(true);

    plotArea->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX = plotArea->range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX.setRange(343, 343.7);
    plotArea->setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX);

    plotArea->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY = plotArea->range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY.setRange(0, 10000);
    plotArea->setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY);

    XYCurve* firstCurve = plotArea->children<XYCurve>().first();
    assert(firstCurve);

    auto* infoElement = new InfoElement("Info Element", plotArea, firstCurve, 343.292);
    plotArea->addChild(infoElement);

    for (XYCurve* curve : plotArea->children<XYCurve>()) {
        if (curve != firstCurve)
            infoElement->addCurve(curve);
    }

    infoElement->title()->setPosition({Worksheet::convertToSceneUnits(4.6, Worksheet::Unit::Centimeter),Worksheet::convertToSceneUnits(1.3, Worksheet::Unit::Centimeter)});

    infoElement->retransform();

    worksheet->view()->show();

    app.exec();
}
