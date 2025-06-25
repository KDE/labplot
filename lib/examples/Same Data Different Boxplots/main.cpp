#include <QApplication>
#include <qnamespace.h>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    AsciiFilter filter1;

    auto p1 = filter1.properties();
	p1.headerEnabled = false;
	filter1.setProperties(p1);

	auto* spreadsheet1 = new Spreadsheet("Data");
    project.addChild(spreadsheet1);

    filter1.readDataFromFile(QStringLiteral("Same Data Different Boxplots/data.txt"), spreadsheet1, AbstractFileFilter::ImportMode::Replace);

    if (!filter1.lastError().isEmpty()) {
		std::cout << "Import error: " << filter1.lastError().toStdString() << std::endl;
		return -1;
	}

    AsciiFilter filter2;

    auto p2 = filter2.properties();
	p2.headerEnabled = false;
	filter2.setProperties(p2);

	auto* spreadsheet2 = new Spreadsheet("Labels");
    project.addChild(spreadsheet2);

    filter2.readDataFromFile(QStringLiteral("Same Data Different Boxplots/labels.txt"), spreadsheet2, AbstractFileFilter::ImportMode::Replace);

    if (!filter2.lastError().isEmpty()) {
		std::cout << "Import error: " << filter2.lastError().toStdString() << std::endl;
		return -1;
	}

    auto* worksheet = new Worksheet("Boxplots");
    project.addChild(worksheet);

    worksheet->setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	worksheet->setPageRect(QRectF(0, 0, w, h));

    worksheet->setLayout(Worksheet::Layout::GridLayout);
    worksheet->setLayoutRowCount(3);
    worksheet->setLayoutColumnCount(3);

    worksheet->setLayoutTopMargin(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
	worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));
    worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter));

    QFont fo7;
    fo7.setPointSize(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Point));

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* plotArea1 = new CartesianPlot("Version 1");
	plotArea1->setType(CartesianPlot::Type::FourAxes);

    plotArea1->setSymmetricPadding(true);
    plotArea1->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea1->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea1->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
        }
    }

    plotArea1->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea1->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea1);

    auto* boxPlot1 = new BoxPlot("BoxPlot");
    plotArea1->addChild(boxPlot1);
    boxPlot1->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot1->setOrdering(BoxPlot::Ordering::None);
    boxPlot1->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot1->setWhiskersRangeParameter(1.5);
    boxPlot1->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot1->setVariableWidth(false);
    boxPlot1->setNotchesEnabled(false);
    boxPlot1->symbolMean()->setStyle(Symbol::Style::NoSymbols);
    boxPlot1->symbolMedian()->setStyle(Symbol::Style::NoSymbols);
    boxPlot1->symbolOutlier()->setStyle(Symbol::Style::NoSymbols);
    boxPlot1->symbolFarOut()->setStyle(Symbol::Style::Plus);
    boxPlot1->symbolFarOut()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    boxPlot1->symbolData()->setStyle(Symbol::Style::NoSymbols);
    boxPlot1->symbolWhiskerEnd()->setStyle(Symbol::Style::NoSymbols);
    boxPlot1->setJitteringEnabled(true);
    boxPlot1->setRugEnabled(false);
    boxPlot1->whiskersLine()->setStyle(Qt::PenStyle::DashLine);
    boxPlot1->whiskersCapLine()->setStyle(Qt::PenStyle::NoPen);
    boxPlot1->backgroundAt(0)->setEnabled(false);
    boxPlot1->backgroundAt(1)->setEnabled(false);
    boxPlot1->recalc();

    auto* plotArea2 = new CartesianPlot("Version 2");
	plotArea2->setType(CartesianPlot::Type::FourAxes);

    plotArea2->setSymmetricPadding(true);
    plotArea2->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea2->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea2->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
        }
    }

    plotArea2->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea2->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea2);

    auto* boxPlot2 = new BoxPlot("BoxPlot");
    plotArea2->addChild(boxPlot2);
    boxPlot2->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot2->setOrdering(BoxPlot::Ordering::None);
    boxPlot2->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot2->setWhiskersRangeParameter(1.5);
    boxPlot2->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot2->setVariableWidth(false);
    boxPlot2->setNotchesEnabled(false);
    boxPlot2->symbolMean()->setStyle(Symbol::Style::Circle);
    boxPlot2->symbolMean()->setColor("white");
    boxPlot2->symbolMean()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    boxPlot2->symbolMedian()->setStyle(Symbol::Style::NoSymbols);
    boxPlot2->symbolOutlier()->setStyle(Symbol::Style::Circle);
    boxPlot2->symbolOutlier()->setColor("black");
    boxPlot2->symbolOutlier()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    boxPlot2->symbolFarOut()->setStyle(Symbol::Style::Plus);
    boxPlot2->symbolFarOut()->setColor("black");
    boxPlot2->symbolFarOut()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    boxPlot2->symbolData()->setStyle(Symbol::Style::NoSymbols);
    boxPlot2->symbolWhiskerEnd()->setStyle(Symbol::Style::NoSymbols);
    boxPlot2->setJitteringEnabled(true);
    boxPlot2->setRugEnabled(false);
    boxPlot2->whiskersLine()->setStyle(Qt::PenStyle::DashLine);
    boxPlot2->whiskersCapLine()->setStyle(Qt::PenStyle::SolidLine);
    boxPlot2->setWhiskersCapSize(Worksheet::convertToSceneUnits(25, Worksheet::Unit::Point));
    boxPlot2->backgroundAt(0)->setEnabled(true);
    boxPlot2->backgroundAt(0)->setFirstColor("black");
    boxPlot2->backgroundAt(0)->setOpacity(1);
    boxPlot2->backgroundAt(1)->setEnabled(true);
    boxPlot2->backgroundAt(1)->setFirstColor("black");
    boxPlot2->backgroundAt(1)->setOpacity(1);
    boxPlot2->recalc();

    auto* plotArea3 = new CartesianPlot("Version 3");
	plotArea3->setType(CartesianPlot::Type::FourAxes);

    plotArea3->setSymmetricPadding(true);
    plotArea3->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea3->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea3->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
        }
    }

    plotArea3->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea3->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea3);

    auto* boxPlot3 = new BoxPlot("BoxPlot");
    plotArea3->addChild(boxPlot3);
    boxPlot3->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot3->setOrdering(BoxPlot::Ordering::None);
    boxPlot3->setWidthFactor(0.8);
    boxPlot3->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot3->setWhiskersRangeParameter(1.5);
    boxPlot3->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot3->setVariableWidth(true);
    boxPlot3->setNotchesEnabled(true);
    boxPlot3->symbolMean()->setStyle(Symbol::Style::Square);
    boxPlot3->symbolMean()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    boxPlot3->symbolMedian()->setStyle(Symbol::Style::NoSymbols);
    boxPlot3->symbolOutlier()->setStyle(Symbol::Style::NoSymbols);
    boxPlot3->symbolFarOut()->setStyle(Symbol::Style::Plus);
    boxPlot3->symbolFarOut()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    boxPlot3->symbolData()->setStyle(Symbol::Style::NoSymbols);
    boxPlot3->symbolWhiskerEnd()->setStyle(Symbol::Style::NoSymbols);
    boxPlot3->setJitteringEnabled(true);
    boxPlot3->setRugEnabled(false);
    boxPlot3->whiskersLine()->setStyle(Qt::PenStyle::SolidLine);
    boxPlot3->whiskersCapLine()->setStyle(Qt::PenStyle::NoPen);
    boxPlot3->backgroundAt(0)->setEnabled(true);
    boxPlot3->backgroundAt(0)->setOpacity(0.3);
    boxPlot3->backgroundAt(1)->setEnabled(true);
    boxPlot3->backgroundAt(1)->setOpacity(0.3);
    boxPlot3->recalc();

    auto* plotArea4 = new CartesianPlot("Version 4");
	plotArea4->setType(CartesianPlot::Type::FourAxes);

    plotArea4->setSymmetricPadding(true);
    plotArea4->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea4->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea4->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
            axis->setMajorTicksDirection(Axis::TicksDirection::enum_type::ticksOut);
            axis->setMajorTicksLength(Worksheet::convertToSceneUnits(2, Worksheet::Unit::Point));
        }
    }

    plotArea4->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea4->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea4);

    auto* boxPlot4 = new BoxPlot("BoxPlot");
    plotArea4->addChild(boxPlot4);
    boxPlot4->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot4->setOrdering(BoxPlot::Ordering::None);
    boxPlot4->setWidthFactor(0.8);
    boxPlot4->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot4->setWhiskersRangeParameter(1.5);
    boxPlot4->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot4->setVariableWidth(true);
    boxPlot4->setNotchesEnabled(false);
    boxPlot4->symbolMean()->setStyle(Symbol::Style::Square);
    boxPlot4->symbolMean()->setSize(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
    boxPlot4->symbolMedian()->setStyle(Symbol::Style::NoSymbols);
    boxPlot4->symbolOutlier()->setStyle(Symbol::Style::Diamond);
    boxPlot4->symbolOutlier()->setSize(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Point));
    boxPlot4->symbolFarOut()->setStyle(Symbol::Style::Diamond);
    boxPlot4->symbolFarOut()->setSize(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Point));
    boxPlot4->symbolData()->setStyle(Symbol::Style::Circle);
    boxPlot4->symbolData()->setSize(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
    boxPlot4->symbolData()->setOpacity(0.15);
    boxPlot4->symbolData()->setColor("black");
    boxPlot4->symbolWhiskerEnd()->setStyle(Symbol::Style::NoSymbols);
    boxPlot4->setJitteringEnabled(true);
    boxPlot4->setRugEnabled(false);
    boxPlot4->whiskersLine()->setStyle(Qt::PenStyle::SolidLine);
    boxPlot4->whiskersCapLine()->setStyle(Qt::PenStyle::NoPen);
    boxPlot4->backgroundAt(0)->setEnabled(false);
    boxPlot4->backgroundAt(1)->setEnabled(false);
    boxPlot4->recalc();

    auto* plotArea5 = new CartesianPlot("Version 5");
	plotArea5->setType(CartesianPlot::Type::FourAxes);

    plotArea5->setSymmetricPadding(true);
    plotArea5->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea5->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea5->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
            axis->majorTicksLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
            axis->majorTicksLine()->setStyle(Qt::PenStyle::NoPen);
        } else {
            axis->setVisible(false);
        }
    }

    plotArea5->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea5->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea5);

    auto* boxPlot5 = new BoxPlot("BoxPlot");
    plotArea5->addChild(boxPlot5);
    boxPlot5->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot5->setOrdering(BoxPlot::Ordering::None);
    boxPlot5->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot5->setWhiskersRangeParameter(1.5);
    boxPlot5->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot5->setVariableWidth(false);
    boxPlot5->setNotchesEnabled(false);
    boxPlot5->symbolMean()->setStyle(Symbol::Style::Circle);
    boxPlot5->symbolMean()->setSize(Worksheet::convertToSceneUnits(4, Worksheet::Unit::Point));
    boxPlot5->symbolMean()->setColor("black");
    boxPlot5->setWhiskersCapSize(0);
    boxPlot5->whiskersCapLine()->setColor("black");
    boxPlot5->setJitteringEnabled(true);
    for (int i = 0; i < spreadsheet1->columnCount(); i++) {
        boxPlot5->medianLineAt(i)->setStyle(Qt::PenStyle::NoPen);
        boxPlot5->borderLineAt(i)->setStyle(Qt::PenStyle::NoPen);
        boxPlot5->backgroundAt(i)->setEnabled(false);
    }

    auto* plotArea6 = new CartesianPlot("Version 6");
	plotArea6->setType(CartesianPlot::Type::FourAxes);

    plotArea6->setSymmetricPadding(true);
    plotArea6->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea6->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea6->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
        }
    }

    plotArea6->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea6->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea6);

    auto* boxPlot6 = new BoxPlot("BoxPlot");
    plotArea6->addChild(boxPlot6);
    boxPlot6->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot6->setOrdering(BoxPlot::Ordering::None);
    boxPlot6->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot6->setWhiskersRangeParameter(1.5);
    boxPlot6->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot6->setVariableWidth(false);
    boxPlot6->setNotchesEnabled(false);
    boxPlot6->symbolMean()->setStyle(Symbol::Style::NoSymbols);
    boxPlot6->symbolOutlier()->setStyle(Symbol::Style::Circle);
    boxPlot6->symbolOutlier()->setSize(Worksheet::convertToSceneUnits(3, Worksheet::Unit::Point));
    boxPlot6->symbolFarOut()->setStyle(Symbol::Style::Circle);
    boxPlot6->symbolFarOut()->setSize(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
    boxPlot6->setJitteringEnabled(false);
    boxPlot6->setRugEnabled(false);
    boxPlot6->setWhiskersCapSize(Worksheet::convertToSceneUnits(40.5, Worksheet::Unit::Point));
    boxPlot6->recalc();

    auto* plotArea7 = new CartesianPlot("Version 7");
	plotArea7->setType(CartesianPlot::Type::FourAxes);

    plotArea7->setSymmetricPadding(true);
    plotArea7->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea7->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea7->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
        }
    }

    plotArea7->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea7->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea7);

    auto* boxPlot7 = new BoxPlot("BoxPlot");
    plotArea7->addChild(boxPlot7);
    boxPlot7->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot7->setOrdering(BoxPlot::Ordering::None);
    boxPlot7->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot7->setWhiskersRangeParameter(1.5);
    boxPlot7->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot7->setVariableWidth(false);
    boxPlot7->setNotchesEnabled(false);
    boxPlot7->symbolMean()->setStyle(Symbol::Style::NoSymbols);
    boxPlot7->symbolOutlier()->setStyle(Symbol::Style::Circle);
    boxPlot7->symbolOutlier()->setSize(Worksheet::convertToSceneUnits(3, Worksheet::Unit::Point));
    boxPlot7->symbolFarOut()->setStyle(Symbol::Style::Circle);
    boxPlot7->symbolFarOut()->setSize(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
    boxPlot7->setJitteringEnabled(false);
    boxPlot7->setRugEnabled(false);
    boxPlot7->setWhiskersCapSize(Worksheet::convertToSceneUnits(40.5, Worksheet::Unit::Point));
    boxPlot7->recalc();

    auto* plotArea8 = new CartesianPlot("Version 8");
	plotArea8->setType(CartesianPlot::Type::FourAxes);

    plotArea8->setSymmetricPadding(true);
    plotArea8->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea8->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea8->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
        }
    }

    plotArea8->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea8->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea8);

    auto* boxPlot8 = new BoxPlot("BoxPlot");
    plotArea8->addChild(boxPlot8);
    boxPlot8->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot8->setOrdering(BoxPlot::Ordering::None);
    boxPlot8->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot8->setWhiskersRangeParameter(1.5);
    boxPlot8->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot8->setVariableWidth(false);
    boxPlot8->setNotchesEnabled(false);
    boxPlot8->symbolMean()->setStyle(Symbol::Style::NoSymbols);
    boxPlot8->symbolOutlier()->setStyle(Symbol::Style::Circle);
    boxPlot8->symbolOutlier()->setSize(Worksheet::convertToSceneUnits(3, Worksheet::Unit::Point));
    boxPlot8->symbolFarOut()->setStyle(Symbol::Style::Circle);
    boxPlot8->symbolFarOut()->setSize(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
    boxPlot8->setJitteringEnabled(false);
    boxPlot8->setRugEnabled(false);
    boxPlot8->setWhiskersCapSize(Worksheet::convertToSceneUnits(40.5, Worksheet::Unit::Point));
    boxPlot8->recalc();

    auto* plotArea9 = new CartesianPlot("Version 9");
	plotArea9->setType(CartesianPlot::Type::FourAxes);

    plotArea9->setSymmetricPadding(true);
    plotArea9->setHorizontalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));
    plotArea9->setVerticalPadding(Worksheet::convertToSceneUnits(0.7, Worksheet::Unit::Centimeter));

    for (Axis* axis : plotArea9->children<Axis>()) {
        axis->title()->setText(QStringLiteral(""));
        axis->title()->setVisible(false);
        axis->setMinorTicksDirection(Axis::TicksDirection::enum_type::noTicks);
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->setMajorTicksType(Axis::TicksType::CustomColumn);
            axis->setMajorTicksColumn(spreadsheet2->column(0));
            axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
            axis->setLabelsTextColumn(spreadsheet2->column(1));
            axis->setLabelsFont(fo7);
            axis->majorGridLine()->setStyle(Qt::PenStyle::NoPen);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->setLabelsFont(fo7);
        }
    }

    plotArea9->enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, true);
    plotArea9->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, true);

    worksheet->addChild(plotArea9);

    auto* boxPlot9 = new BoxPlot("BoxPlot");
    plotArea9->addChild(boxPlot9);
    boxPlot9->setDataColumns({spreadsheet1->column(0), spreadsheet1->column(1)});
    boxPlot9->setOrdering(BoxPlot::Ordering::None);
    boxPlot9->setWhiskersType(BoxPlot::WhiskersType::IQR);
    boxPlot9->setWhiskersRangeParameter(1.5);
    boxPlot9->setOrientation(BoxPlot::Orientation::Vertical);
    boxPlot9->setVariableWidth(false);
    boxPlot9->setNotchesEnabled(false);
    boxPlot9->symbolMean()->setStyle(Symbol::Style::NoSymbols);
    boxPlot9->symbolOutlier()->setStyle(Symbol::Style::Circle);
    boxPlot9->symbolOutlier()->setSize(Worksheet::convertToSceneUnits(3, Worksheet::Unit::Point));
    boxPlot9->symbolFarOut()->setStyle(Symbol::Style::Circle);
    boxPlot9->symbolFarOut()->setSize(Worksheet::convertToSceneUnits(6, Worksheet::Unit::Point));
    boxPlot9->setJitteringEnabled(false);
    boxPlot9->setRugEnabled(false);
    boxPlot9->setWhiskersCapSize(Worksheet::convertToSceneUnits(40.5, Worksheet::Unit::Point));
    boxPlot9->recalc();

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* textLabel1= new TextLabel("Text Label");
    worksheet->addChild(textLabel1);

    QTextEdit te;
    te.clear();
    te.setFontPointSize(14);
    te.append("Same Data, Different Boxplots");

    textLabel1->setText(te.toHtml());
    textLabel1->setPosition({Worksheet::convertToSceneUnits(-0.3, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(9.2, Worksheet::Unit::Centimeter)});

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    worksheet->view()->show();

    app.exec();
}
