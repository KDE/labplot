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

    filter.readDataFromFile(QStringLiteral("Anatomy of a BoxPlot/data.txt"), &spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    Worksheet worksheet("Anatomy of a Box Plot");
    project.addChild(&worksheet);

    worksheet.setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);
	worksheet.setPageRect(QRectF(0, 0, w, h));

    worksheet.setLayout(Worksheet::Layout::VerticalLayout);

    worksheet.setLayoutTopMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet.setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet.setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet.setLayoutRightMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    worksheet.setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    worksheet.setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea("Plot");
	plotArea.setType(CartesianPlot::Type::FourAxes);

    plotArea.setSymmetricPadding(true);
    plotArea.setHorizontalPadding(Worksheet::convertToSceneUnits(2.4, Worksheet::Unit::Centimeter));
    plotArea.setVerticalPadding(Worksheet::convertToSceneUnits(1.3, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea.children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setVisible(false);
    }

    worksheet.addChild(&plotArea);

    BoxPlot boxPlot("BoxPlot");
    plotArea.addChild(&boxPlot);
    boxPlot.setDataColumns({spreadsheet.column(0)});
    boxPlot.setOrdering(BoxPlot::Ordering::None);
    boxPlot.setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot.setWhiskersRangeParameter(1.5);
    boxPlot.setWidthFactor(0.7);
    boxPlot.setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot.setVariableWidth(false);
    boxPlot.setNotchesEnabled(false);
    boxPlot.symbolMean()->setStyle(Symbol::Style::NoSymbols);
    boxPlot.symbolOutlier()->setStyle(Symbol::Style::Circle);
    boxPlot.symbolOutlier()->setSize(Worksheet::convertToSceneUnits(3, Worksheet::Unit::Point));
    boxPlot.symbolFarOut()->setStyle(Symbol::Style::Circle);
    boxPlot.symbolFarOut()->setSize(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
    boxPlot.setJitteringEnabled(false);
    boxPlot.setRugEnabled(false);
    boxPlot.setWhiskersCapSize(Worksheet::convertToSceneUnits(40.5, Worksheet::Unit::Point));
    boxPlot.recalc();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    QFont f;
    f.setPointSize(4);
    QTextEdit te;
    te.setFont(f);

    TextLabel textLabel1("Lower inner fence - label");
    worksheet.addChild(&textLabel1);
    te.setPlainText("Inner fence (not shown)");
    textLabel1.setText(te.toHtml());
    textLabel1.setPosition({Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-0.7, Worksheet::Unit::Centimeter)});

    TextLabel textLabel2("Lower outer fence - label");
    worksheet.addChild(&textLabel2);
    te.setPlainText("Outer fence (not shown)");
    textLabel2.setText(te.toHtml());
    textLabel2.setPosition({Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-2.1, Worksheet::Unit::Centimeter)});

    TextLabel textLabel3("Lower outliers - label");
    worksheet.addChild(&textLabel3);
    te.setPlainText("Lower outliers");
    textLabel3.setText(te.toHtml());
    textLabel3.setPosition({Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-1, Worksheet::Unit::Centimeter)});

    TextLabel textLabel4("Lower far outliers - label");
    worksheet.addChild(&textLabel4);
    te.setPlainText("Lower far outliers");
    textLabel4.setText(te.toHtml());
    textLabel4.setPosition({Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-2.5, Worksheet::Unit::Centimeter)});

    TextLabel textLabel5("Lower adjacent value - label");
    worksheet.addChild(&textLabel5);
    te.setPlainText("Lower adjacent value");
    textLabel5.setText(te.toHtml());
    textLabel5.setPosition({Worksheet::convertToSceneUnits(1.8, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-0.4, Worksheet::Unit::Centimeter)});

    TextLabel textLabel6("Median");
    worksheet.addChild(&textLabel6);
    te.setPlainText("Median");
    textLabel6.setText(te.toHtml());
    textLabel6.setPosition({Worksheet::convertToSceneUnits(1.2, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0.6, Worksheet::Unit::Centimeter)});

    TextLabel textLabel7("3rd Quartile");
    worksheet.addChild(&textLabel7);
    te.setPlainText("3rd Quartile");
    textLabel7.setText(te.toHtml());
    textLabel7.setPosition({Worksheet::convertToSceneUnits(1.4, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0.9, Worksheet::Unit::Centimeter)});

    TextLabel textLabel8("1st Quartile");
    worksheet.addChild(&textLabel8);
    te.setPlainText("1st Quartile");
    textLabel8.setText(te.toHtml());
    textLabel8.setPosition({Worksheet::convertToSceneUnits(1.4, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0.4, Worksheet::Unit::Centimeter)});

    TextLabel textLabel9("Upper adjacent value - label");
    worksheet.addChild(&textLabel9);
    te.setPlainText("Upper adjacent value");
    textLabel9.setText(te.toHtml());
    textLabel9.setPosition({Worksheet::convertToSceneUnits(1.8, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(1.7, Worksheet::Unit::Centimeter)});

    TextLabel textLabel10("Upper inner fence - label");
    worksheet.addChild(&textLabel10);
    te.setPlainText("Upper inner fence (not shown)");
    textLabel10.setText(te.toHtml());
    textLabel10.setPosition({Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(1.9, Worksheet::Unit::Centimeter)});

    TextLabel textLabel11("Upper outer fence - label");
    worksheet.addChild(&textLabel11);
    te.setPlainText("Upper outer fence (not shown)");
    textLabel11.setText(te.toHtml());
    textLabel11.setPosition({Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(2.8, Worksheet::Unit::Centimeter)});

    TextLabel textLabel12("Upper far outliers - label");
    worksheet.addChild(&textLabel12);
    te.setPlainText("Upper far outliers");
    textLabel12.setText(te.toHtml());
    textLabel12.setPosition({Worksheet::convertToSceneUnits(1.6, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(3.2, Worksheet::Unit::Centimeter)});

    TextLabel textLabel13("Upper outliers");
    worksheet.addChild(&textLabel13);
    te.setPlainText("Upper outliers");
    textLabel13.setText(te.toHtml());
    textLabel13.setPosition({Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(2.3, Worksheet::Unit::Centimeter)});

    TextLabel textLabel14("Upper whisker - label");
    worksheet.addChild(&textLabel14);
    te.setPlainText("Upper whisker");
    textLabel14.setText(te.toHtml());
    textLabel14.setPosition({Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(1.3, Worksheet::Unit::Centimeter)});

    TextLabel textLabel15("Lower whisker - label");
    worksheet.addChild(&textLabel15);
    te.setPlainText("Lower whisker");
    textLabel15.setText(te.toHtml());
    textLabel15.setPosition({Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter)});

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    ReferenceLine rl1(&plotArea, QStringLiteral("Lower inner fence"));
    plotArea.addChild(&rl1);
    rl1.setOrientation(ReferenceLine::Orientation::Horizontal);
    rl1.setPositionLogical({0, 0.2});
    rl1.line()->setStyle(Qt::PenStyle::DashLine);
    rl1.line()->setWidth(0);
    rl1.line()->setOpacity(0.8);

    ReferenceLine rl2(&plotArea, QStringLiteral("Lower outer fence"));
    plotArea.addChild(&rl2);
    rl2.setOrientation(ReferenceLine::Orientation::Horizontal);
    rl2.setPositionLogical({0, -10});
    rl2.line()->setStyle(Qt::PenStyle::DashLine);
    rl2.line()->setWidth(0);
    rl2.line()->setOpacity(0.8);

    ReferenceLine rl3(&plotArea, QStringLiteral("Upper inner fence"));
    plotArea.addChild(&rl3);
    rl3.setOrientation(ReferenceLine::Orientation::Horizontal);
    rl3.setPositionLogical({0, 19});
    rl3.line()->setStyle(Qt::PenStyle::DashLine);
    rl3.line()->setWidth(0);
    rl3.line()->setOpacity(0.8);

    ReferenceLine rl4(&plotArea, QStringLiteral("Upper outer fence"));
    plotArea.addChild(&rl4);
    rl4.setOrientation(ReferenceLine::Orientation::Horizontal);
    rl4.setPositionLogical({0, 25.5});
    rl4.line()->setStyle(Qt::PenStyle::DashLine);
    rl4.line()->setWidth(0);
    rl4.line()->setOpacity(0.8);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    plotArea.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX = plotArea.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX.setRange(0.5, 1.5);
    plotArea.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX);

    plotArea.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY = plotArea.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY.setRange(-20, 30);
    plotArea.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    worksheet.view()->show();

    app.exec();
}
