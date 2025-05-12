#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    auto* f = new Folder("SR-UWB");
    project.addChild(f);

    AsciiFilter filter;

    auto p = filter.properties();
	p.headerEnabled = false;
	filter.setProperties(p);

	auto* spreadsheet = new Spreadsheet("Q=1.25, tr=5ps");
    project.addChild(spreadsheet);

    filter.readDataFromFile(QStringLiteral("Time response of a super regenerative receiver/data.txt"), spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* worksheet = new Worksheet("Worksheet");
    project.addChild(worksheet);

    worksheet->setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(13.2, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(12.7, Worksheet::Unit::Centimeter);
	worksheet->setPageRect(QRectF(0, 0, w, h));

    worksheet->setLayout(Worksheet::Layout::VerticalLayout);
    worksheet->setLayoutTopMargin(Worksheet::convertToSceneUnits(0.3, Worksheet::Unit::Centimeter));
	worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));
	worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));
	worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));

    worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    auto* plotArea = new CartesianPlot("Response plot");
	plotArea->setType(CartesianPlot::Type::FourAxes);

    plotArea->setSymmetricPadding(false);
    plotArea->setHorizontalPadding(Worksheet::convertToSceneUnits(3, Worksheet::Unit::Centimeter));
    plotArea->setVerticalPadding(Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter));
    plotArea->setRightPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea->setBottomPadding(Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter));

    Range<double> rangeX = plotArea->range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX.setRange(2E-09, 5E-09);
    plotArea->setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX);

    Range<double> rangeY = plotArea->range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY.setRange(-0.00035, 0.00035);
    plotArea->setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY);

    plotArea->enableAutoScale(CartesianCoordinateSystem::Dimension::X, -1, false);
    plotArea->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, -1, false);

    QTextEdit te;
    te.setText(QStringLiteral("Time response of SuperRegenerative receiverQ=1,25  ·  tr=5ps  ·  wid=343,8ps  ·  delay=0ps"));
    te.setHtml(te.toHtml().replace("receiver", "receiver<br>"));
    plotArea->title()->setText(te.toHtml());

    QFont fo1;
    fo1.setPointSizeF(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Point));

    for (Axis* axis : plotArea->children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            axis->title()->setText(QStringLiteral("Time [s]"));
            axis->setRangeType(Axis::RangeType::Custom);
            axis->setRange(2E-09, 5E-09);
            axis->setScalingFactor(1E+09);
            axis->setLabelsSuffix("ns");
            axis->setMinorTicksAutoNumber(false);
            axis->setMinorTicksNumber(4);
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo1);
            axis->setLabelsAutoPrecision(false);
            axis->setLabelsPrecision(1);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("Voltage [V]"));
            axis->setRangeType(Axis::RangeType::Custom);
            axis->setRange(-0.0004, 0.0004);
            axis->setScalingFactor(1000);
            axis->setLabelsSuffix("mV");
            axis->setMinorTicksAutoNumber(false);
            axis->setMinorTicksNumber(4);
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo1);
        }
    }

    worksheet->addChild(plotArea);

    auto* config1 = new XYCurve("Received pulse (x100)");
    plotArea->addChild(config1);
    config1->setPlotType(Plot::PlotType::Line);
    config1->setXColumn(spreadsheet->column(0));
    config1->setYColumn(spreadsheet->column(1));
    config1->setLineType(XYCurve::LineType::Line);
    config1->symbol()->setStyle(Symbol::Style::NoSymbols);
    config1->setValuesType(XYCurve::ValuesType::NoValues);

    auto* config2 = new XYCurve("Response");
    plotArea->addChild(config2);
    config2->setPlotType(Plot::PlotType::Line);
    config2->setXColumn(spreadsheet->column(0));
    config2->setYColumn(spreadsheet->column(2));
    config2->setLineType(XYCurve::LineType::Line);
    config2->symbol()->setStyle(Symbol::Style::NoSymbols);
    config2->setValuesType(XYCurve::ValuesType::NoValues);

    auto* config3 = new XYHilbertTransformCurve("Envelope");
    plotArea->addChild(config3);
    config3->setXDataColumn(spreadsheet->column(0));
    config3->setYDataColumn(spreadsheet->column(2));
    config3->setLineInterpolationPointsCount(1);
    XYHilbertTransformCurve::TransformData configData = config3->transformData();
    configData.type = nsl_hilbert_result_envelope;
    config3->setTransformData(configData);

    auto* cp = new CustomPoint(plotArea, "Gmax");
	plotArea->addChild(cp);
    cp->setCoordinateSystemIndex(plotArea->defaultCoordinateSystemIndex());
    cp->setCoordinateBindingEnabled(true);
    cp->setPositionLogical({3.355E-09, 0.000312});

    auto* rl1 = new ReferenceLine(plotArea, QStringLiteral("MaxTimeLine"));
    plotArea->addChild(rl1);
    rl1->setOrientation(ReferenceLine::Orientation::Vertical);
    rl1->setPositionLogical({3.355E-09, 0});
    rl1->line()->setStyle(Qt::PenStyle::DashLine);
    rl1->line()->setWidth(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point));
    rl1->line()->setOpacity(1);

    auto* rl2 = new ReferenceLine(plotArea, QStringLiteral("MaxAmpLine"));
    plotArea->addChild(rl2);
    rl2->setOrientation(ReferenceLine::Orientation::Horizontal);
    rl2->setPositionLogical({0, 0.000312493});
    rl2->line()->setStyle(Qt::PenStyle::DashLine);
    rl2->line()->setWidth(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point));
    rl2->line()->setOpacity(1);

    auto* textLabel1 = new TextLabel("MaxAmpText");
    worksheet->addChild(textLabel1);

    QFont fo21;
    fo21.setPointSize(8);
    QTextEdit te1;
    te1.setFont(fo21);
    te1.setTextColor(rl1->line()->color());

    te1.setPlainText("0,312 mV");
    textLabel1->setText(te1.toHtml());
    textLabel1->setPosition({Worksheet::convertToSceneUnits(4.1, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(3.3, Worksheet::Unit::Centimeter)});

    auto* textLabel2 = new TextLabel("MaxTimeText");
    worksheet->addChild(textLabel2);

    QFont fo22;
    fo22.setPointSize(8);
    QTextEdit te2;
    te2.setFont(fo22);
    te2.setTextColor(rl1->line()->color());

    te2.setPlainText("3,355 ns");
    textLabel2->setText(te2.toHtml());
    textLabel2->setPosition({Worksheet::convertToSceneUnits(1.3, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-3, Worksheet::Unit::Centimeter)});

    plotArea->retransform();

    worksheet->view()->show();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    app.exec();
}
