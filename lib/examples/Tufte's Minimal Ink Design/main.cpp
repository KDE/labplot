#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    AsciiFilter filter;

    auto p = filter.properties();
	p.headerEnabled = false;
	filter.setProperties(p);

	Spreadsheet spreadsheet("Data");
    project.addChild(&spreadsheet);

    filter.readDataFromFile(QStringLiteral("Tufte's Minimal Ink Design/data.txt"), &spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    Worksheet worksheet("Standard vs. Tufte's Minimal Ink Design");
    project.addChild(&worksheet);

    worksheet.setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(15, Worksheet::Unit::Centimeter);
	worksheet.setPageRect(QRectF(0, 0, w, h));

    worksheet.setLayout(Worksheet::Layout::GridLayout);
    worksheet.setLayoutRowCount(2);
    worksheet.setLayoutColumnCount(3);

    worksheet.setLayoutTopMargin(Worksheet::convertToSceneUnits(1.2, Worksheet::Unit::Centimeter));
	worksheet.setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.3, Worksheet::Unit::Centimeter));
	worksheet.setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet.setLayoutRightMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    worksheet.setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));
    worksheet.setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(1.6, Worksheet::Unit::Centimeter));

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea1("Scatterplot - Standard");
	plotArea1.setType(CartesianPlot::Type::FourAxes);

    plotArea1.setSymmetricPadding(true);
    plotArea1.setHorizontalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea1.setVerticalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea1.children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
    }

    worksheet.addChild(&plotArea1);

    XYCurve config1("config1");
    plotArea1.addChild(&config1);
    config1.setPlotType(Plot::PlotType::Scatter);
    config1.setXColumn(spreadsheet.column(0));
    config1.setYColumn(spreadsheet.column(1));
    config1.symbol()->setStyle(Symbol::Style::Circle);
    config1.symbol()->setSize(Worksheet::convertToSceneUnits(4, Worksheet::Unit::Point));
    config1.setValuesType(XYCurve::ValuesType::NoValues);

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX1 = plotArea1.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX1.setRange(0.5, 5.5);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX1);

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY1 = plotArea1.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY1.setRange(-3, 3);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY1);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea2("Histograms - Standard");
	plotArea2.setType(CartesianPlot::Type::FourAxes);

    plotArea2.setSymmetricPadding(true);
    plotArea2.setHorizontalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea2.setVerticalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea2.children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
    }

    worksheet.addChild(&plotArea2);

    Histogram config21("config21");
    plotArea2.addChild(&config21);
    config21.setDataColumn(spreadsheet.column(0));
    config21.setType(Histogram::Type::Ordinary);
    config21.setOrientation(Histogram::Orientation::Vertical);
    config21.setNormalization(Histogram::Normalization::Count);
    config21.setBinningMethod(Histogram::BinningMethod::SquareRoot);
    config21.setAutoBinRanges(true);
    config21.line()->setHistogramLineType(Histogram::LineType::Bars);
    config21.symbol()->setStyle(Symbol::Style::NoSymbols);
    config21.value()->setType(Value::Type::NoValues);
    config21.background()->setType(Background::Type::Color);
    config21.background()->setColorStyle(Background::ColorStyle::SingleColor);
    config21.background()->setFirstColor(config21.color());
    config21.background()->setOpacity(0.4);
    config21.background()->setEnabled(true);

    Histogram config22("config22");
    plotArea2.addChild(&config22);
    config22.setDataColumn(spreadsheet.column(1));
    config22.setType(Histogram::Type::Ordinary);
    config22.setOrientation(Histogram::Orientation::Vertical);
    config22.setNormalization(Histogram::Normalization::Count);
    config22.setBinningMethod(Histogram::BinningMethod::SquareRoot);
    config22.setAutoBinRanges(true);
    config22.line()->setHistogramLineType(Histogram::LineType::Bars);
    config22.symbol()->setStyle(Symbol::Style::NoSymbols);
    config22.value()->setType(Value::Type::NoValues);
    config22.background()->setType(Background::Type::Color);
    config22.background()->setColorStyle(Background::ColorStyle::SingleColor);
    config22.background()->setFirstColor(config22.color());
    config22.background()->setOpacity(0.4);
    config22.background()->setEnabled(true);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea3("Boxplots - Standard");
	plotArea3.setType(CartesianPlot::Type::FourAxes);

    plotArea3.setSymmetricPadding(true);
    plotArea3.setHorizontalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea3.setVerticalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea3.children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
    }

    worksheet.addChild(&plotArea3);

    BoxPlot config31("config31");
    plotArea3.addChild(&config31);
    config31.setDataColumns({spreadsheet.column(0), spreadsheet.column(1)});
    config31.setOrdering(BoxPlot::Ordering::None);
    config31.setWhiskersType(BoxPlot::WhiskersType::IQR);
    config31.setWhiskersRangeParameter(1.5);
    config31.setOrientation(BoxPlot::Orientation::Vertical);
    config31.setVariableWidth(false);
    config31.setNotchesEnabled(false);
    config31.symbolMean()->setStyle(Symbol::Style::Square);
    config31.setJitteringEnabled(true);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea4("Scatterplot - Tufte");
	plotArea4.setType(CartesianPlot::Type::FourAxes);

    plotArea4.setSymmetricPadding(true);
    plotArea4.setHorizontalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea4.setVerticalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea4.children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
    }

    worksheet.addChild(&plotArea4);

    XYCurve config4("config4");
    plotArea4.addChild(&config4);
    config4.setPlotType(Plot::PlotType::Scatter);
    config4.setXColumn(spreadsheet.column(0));
    config4.setYColumn(spreadsheet.column(1));
    config4.symbol()->setStyle(Symbol::Style::Circle);
    config4.symbol()->setSize(Worksheet::convertToSceneUnits(4, Worksheet::Unit::Point));
    config4.symbol()->setColor("black");
    config4.setValuesType(XYCurve::ValuesType::NoValues);

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX4 = plotArea1.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX4.setRange(0.5, 5.5);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX4);

    plotArea1.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY4 = plotArea1.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY4.setRange(-3, 3);
    plotArea1.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY4);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea5("Histograms - Tufte");
	plotArea5.setType(CartesianPlot::Type::FourAxes);

    plotArea5.setSymmetricPadding(true);
    plotArea5.setHorizontalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea5.setVerticalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea5.children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
    }

    worksheet.addChild(&plotArea5);

    Histogram config51("config51");
    plotArea5.addChild(&config51);
    config51.setDataColumn(spreadsheet.column(0));
    config51.setType(Histogram::Type::Ordinary);
    config51.setOrientation(Histogram::Orientation::Vertical);
    config51.setNormalization(Histogram::Normalization::Count);
    config51.setBinningMethod(Histogram::BinningMethod::SquareRoot);
    config51.setAutoBinRanges(true);
    config51.line()->setHistogramLineType(Histogram::LineType::HalfBars);
    config51.line()->setColor("black");
    config51.symbol()->setStyle(Symbol::Style::NoSymbols);
    config51.value()->setType(Value::Type::NoValues);
    config51.background()->setEnabled(false);

    Histogram config52("config52");
    plotArea5.addChild(&config52);
    config52.setDataColumn(spreadsheet.column(1));
    config52.setType(Histogram::Type::Ordinary);
    config52.setOrientation(Histogram::Orientation::Vertical);
    config52.setNormalization(Histogram::Normalization::Count);
    config52.setBinningMethod(Histogram::BinningMethod::SquareRoot);
    config52.setAutoBinRanges(true);
    config52.line()->setHistogramLineType(Histogram::LineType::HalfBars);
    config52.line()->setColor("black");
    config52.symbol()->setStyle(Symbol::Style::NoSymbols);
    config52.value()->setType(Value::Type::NoValues);
    config52.background()->setEnabled(false);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    CartesianPlot plotArea6("Boxplots - Tufte");
	plotArea6.setType(CartesianPlot::Type::FourAxes);

    plotArea6.setSymmetricPadding(true);
    plotArea6.setHorizontalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));
    plotArea6.setVerticalPadding(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea6.children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
    }

    worksheet.addChild(&plotArea6);

    BoxPlot config61("config61");
    plotArea6.addChild(&config61);
    config61.setDataColumns({spreadsheet.column(0), spreadsheet.column(1)});
    config61.setOrdering(BoxPlot::Ordering::None);
    config61.setWhiskersType(BoxPlot::WhiskersType::IQR);
    config61.setWhiskersRangeParameter(1.5);
    config61.setOrientation(BoxPlot::Orientation::Vertical);
    config61.setVariableWidth(false);
    config61.setNotchesEnabled(false);
    config61.symbolMean()->setStyle(Symbol::Style::Circle);
    config61.symbolMean()->setSize(Worksheet::convertToSceneUnits(4, Worksheet::Unit::Point));
    config61.symbolMean()->setColor("black");
    config61.setWhiskersCapSize(0);
    config61.whiskersCapLine()->setColor("black");
    config61.setJitteringEnabled(true);
    
    for (int i = 0; i < spreadsheet.columnCount(); i++) {
        config61.medianLineAt(i)->setStyle(Qt::PenStyle::NoPen);
        config61.borderLineAt(i)->setStyle(Qt::PenStyle::NoPen);
        config61.backgroundAt(i)->setEnabled(false);
    }

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    TextLabel textLabel1("Tufte's Minimal Ink Design");
    textLabel1.setText(QStringLiteral("Tufte's Minimal Ink Design"));
    worksheet.addChild(&textLabel1);
    textLabel1.setPosition({Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-0.7, Worksheet::Unit::Centimeter)});

    TextLabel textLabel2("Standard Design");
    textLabel2.setText(QStringLiteral("Standard Design"));
    worksheet.addChild(&textLabel2);
    textLabel2.setPosition({Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(6.8, Worksheet::Unit::Centimeter)});

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    worksheet.view()->show();

    app.exec();
}
