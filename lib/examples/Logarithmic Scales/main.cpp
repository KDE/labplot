#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    Worksheet worksheet("Worksheet");
    project.addChild(&worksheet);

    worksheet.setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(15.6, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(12.7, Worksheet::Unit::Centimeter);
	worksheet.setPageRect(QRectF(0, 0, w, h));

    worksheet.setLayout(Worksheet::Layout::GridLayout);
    worksheet.setLayoutRowCount(2);
    worksheet.setLayoutColumnCount(2);

    double ms = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
    worksheet.setLayoutTopMargin(ms);
	worksheet.setLayoutBottomMargin(ms);
	worksheet.setLayoutLeftMargin(ms);
	worksheet.setLayoutRightMargin(ms);

    worksheet.setLayoutHorizontalSpacing(ms);
    worksheet.setLayoutVerticalSpacing(ms);

    worksheet.setTheme(QStringLiteral("Solarized"));

    CartesianPlot plotArea("xy-plot");
	plotArea.setType(CartesianPlot::Type::FourAxes);
    
    PlotArea::BorderType border = plotArea.plotArea()->borderType();
    border.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea.plotArea()->setBorderType(border);

    plotArea.setSymmetricPadding(true);
    plotArea.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea.setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    plotArea.title()->setText(QStringLiteral("lin-lin"));
    plotArea.title()->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    plotArea.title()->setVerticalAlignment(WorksheetElement::VerticalAlignment::Top);
    plotArea.title()->setPosition({Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter)});

    worksheet.addChild(&plotArea);

    for (Axis* axis : plotArea.children<Axis>()) {
        axis->setMinorTicksLength(0);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->title()->setText(QStringLiteral("x"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("f(x)"));
        }
    }

    XYEquationCurve config11("f(x)=10^x");
    plotArea.addChild(&config11);
    XYEquationCurve::EquationData eqData11 = config11.equationData();
    eqData11.type = XYEquationCurve::EquationType::Cartesian;
    eqData11.count = 1000;
    eqData11.min = "0";
    eqData11.max = "10";
    eqData11.expression1 = "10^x";
    config11.setEquationData(eqData11);
    config11.recalculate();

    XYEquationCurve config12("f(x)=x");
    plotArea.addChild(&config12);
    XYEquationCurve::EquationData eqData12 = config12.equationData();
    eqData12.type = XYEquationCurve::EquationType::Cartesian;
    eqData12.count = 2;
    eqData12.min = "0";
    eqData12.max = "10";
    eqData12.expression1 = "x";
    config12.setEquationData(eqData12);
    config12.recalculate();

    XYEquationCurve config13("f(x)=log(x)");
    plotArea.addChild(&config13);
    XYEquationCurve::EquationData eqData13 = config13.equationData();
    eqData13.type = XYEquationCurve::EquationType::Cartesian;
    eqData13.count = 100;
    eqData13.min = "0";
    eqData13.max = "10";
    eqData13.expression1 = "log(x)";
    config13.setEquationData(eqData13);
    config13.recalculate();

    plotArea.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX = plotArea.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX.setRange(0, 10);
    plotArea.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX);

    plotArea.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY = plotArea.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY.setRange(0, 10);
    plotArea.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea1("xy-plot 1");
	plotArea1.setType(CartesianPlot::Type::FourAxes);
    
    PlotArea::BorderType border1 = plotArea1.plotArea()->borderType();
    border1.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea1.plotArea()->setBorderType(border1);

    plotArea1.setSymmetricPadding(true);
    plotArea1.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea1.setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    plotArea1.title()->setText(QStringLiteral("log-lin"));
    plotArea1.title()->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    plotArea1.title()->setVerticalAlignment(WorksheetElement::VerticalAlignment::Top);
    plotArea1.title()->setPosition({Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter)});

    worksheet.addChild(&plotArea1);

    for (Axis* axis : plotArea1.children<Axis>()) {
        axis->setMinorTicksLength(0);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->title()->setText(QStringLiteral("x"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("f(x)"));
        }
    }

    XYEquationCurve config21("f(x)=10^x");
    plotArea1.addChild(&config21);
    XYEquationCurve::EquationData eqData21 = config21.equationData();
    eqData21.type = XYEquationCurve::EquationType::Cartesian;
    eqData21.count = 100;
    eqData21.min = "0";
    eqData21.max = "1";
    eqData21.expression1 = "10^x";
    config21.setEquationData(eqData21);
    config21.recalculate();

    XYEquationCurve config22("f(x)=x");
    plotArea1.addChild(&config22);
    XYEquationCurve::EquationData eqData22 = config22.equationData();
    eqData22.type = XYEquationCurve::EquationType::Cartesian;
    eqData22.count = 100;
    eqData22.min = "0";
    eqData22.max = "10";
    eqData22.expression1 = "x";
    config22.setEquationData(eqData22);
    config22.recalculate();

    XYEquationCurve config23("f(x)=log(x)");
    plotArea1.addChild(&config23);
    XYEquationCurve::EquationData eqData23 = config23.equationData();
    eqData23.type = XYEquationCurve::EquationType::Cartesian;
    eqData23.count = 100;
    eqData23.min = "1";
    eqData23.max = "1000";
    eqData23.expression1 = "log(x)";
    config23.setEquationData(eqData23);
    config23.recalculate();

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX1 = plotArea1.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX1.setScale(RangeT::Scale::Log10);
    rangeX1.setRange(0.1, 1000);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX1);

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY1 = plotArea1.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY1.setRange(0, 10);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY1);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea2("xy-plot 2");
	plotArea2.setType(CartesianPlot::Type::FourAxes);
    
    PlotArea::BorderType border2 = plotArea2.plotArea()->borderType();
    border2.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea2.plotArea()->setBorderType(border2);

    plotArea2.setSymmetricPadding(true);
    plotArea2.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea2.setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    plotArea2.title()->setText(QStringLiteral("lin-log"));
    plotArea2.title()->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    plotArea2.title()->setVerticalAlignment(WorksheetElement::VerticalAlignment::Top);
    plotArea2.title()->setPosition({Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter)});

    worksheet.addChild(&plotArea2);

    for (Axis* axis : plotArea2.children<Axis>()) {
        axis->setMinorTicksLength(0);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->title()->setText(QStringLiteral("x"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("f(x)"));
        }
    }

    XYEquationCurve config31("f(x)=10^x");
    plotArea2.addChild(&config31);
    XYEquationCurve::EquationData eqData31 = config31.equationData();
    eqData31.type = XYEquationCurve::EquationType::Cartesian;
    eqData31.count = 1000;
    eqData31.min = "0";
    eqData31.max = "10";
    eqData31.expression1 = "10^x";
    config31.setEquationData(eqData31);
    config31.recalculate();

    XYEquationCurve config32("f(x)=x");
    plotArea2.addChild(&config32);
    XYEquationCurve::EquationData eqData32 = config32.equationData();
    eqData32.type = XYEquationCurve::EquationType::Cartesian;
    eqData32.count = 100;
    eqData32.min = "0";
    eqData32.max = "10";
    eqData32.expression1 = "x";
    config32.setEquationData(eqData32);
    config32.recalculate();

    XYEquationCurve config33("f(x)=log(x)");
    plotArea2.addChild(&config33);
    XYEquationCurve::EquationData eqData33 = config33.equationData();
    eqData33.type = XYEquationCurve::EquationType::Cartesian;
    eqData33.count = 1000;
    eqData33.min = "0";
    eqData33.max = "10";
    eqData33.expression1 = "log(x)";
    config33.setEquationData(eqData33);
    config33.recalculate();

    plotArea2.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX2 = plotArea2.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX2.setRange(0, 10);
    plotArea2.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX2);

    plotArea2.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY2 = plotArea2.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY2.setRange(0.1, 1000);
    rangeY2.setScale(RangeT::Scale::Log10);
    plotArea2.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY2);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea3("xy-plot 3");
	plotArea3.setType(CartesianPlot::Type::FourAxes);
    
    PlotArea::BorderType border3 = plotArea3.plotArea()->borderType();
    border3.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea3.plotArea()->setBorderType(border3);

    plotArea3.setSymmetricPadding(true);
    plotArea3.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea3.setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    plotArea3.title()->setText(QStringLiteral("log-log"));
    plotArea3.title()->setHorizontalAlignment(WorksheetElement::HorizontalAlignment::Center);
    plotArea3.title()->setVerticalAlignment(WorksheetElement::VerticalAlignment::Top);
    plotArea3.title()->setPosition({Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter)});

    worksheet.addChild(&plotArea3);

    for (Axis* axis : plotArea3.children<Axis>()) {
        axis->setMinorTicksLength(0);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->title()->setText(QStringLiteral("x"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("f(x)"));
        }
    }

    XYEquationCurve config41("f(x)=10^x");
    plotArea3.addChild(&config41);
    XYEquationCurve::EquationData eqData41 = config41.equationData();
    eqData41.type = XYEquationCurve::EquationType::Cartesian;
    eqData41.count = 1000;
    eqData41.min = "0";
    eqData41.max = "10";
    eqData41.expression1 = "10^x";
    config41.setEquationData(eqData41);
    config41.recalculate();

    XYEquationCurve config42("f(x)=x");
    plotArea3.addChild(&config42);
    XYEquationCurve::EquationData eqData42 = config42.equationData();
    eqData42.type = XYEquationCurve::EquationType::Cartesian;
    eqData42.count = 1000;
    eqData42.min = "0.1";
    eqData42.max = "1000";
    eqData42.expression1 = "x";
    config42.setEquationData(eqData42);
    config42.recalculate();

    XYEquationCurve config43("f(x)=log(x)");
    plotArea3.addChild(&config43);
    XYEquationCurve::EquationData eqData43 = config43.equationData();
    eqData43.type = XYEquationCurve::EquationType::Cartesian;
    eqData43.count = 10000;
    eqData43.min = "0.1";
    eqData43.max = "1000";
    eqData43.expression1 = "log(x)";
    config43.setEquationData(eqData43);
    config43.recalculate();

    plotArea3.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX3 = plotArea3.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX3.setScale(RangeT::Scale::Log10);
    rangeX3.setRange(0.1, 1000);
    plotArea3.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX3);

    plotArea3.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY3 = plotArea3.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY3.setScale(RangeT::Scale::Log10);
    rangeY3.setRange(0.1, 1000);
    plotArea3.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY3);

    worksheet.view()->show();

    app.exec();
}
