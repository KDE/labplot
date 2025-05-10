#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    Spreadsheet spreadsheet("HYG data");
    project.addChild(&spreadsheet);

    {
        AsciiFilter filter;

        auto p = filter.properties();
        p.headerEnabled = false;
        filter.setProperties(p);
        filter.readDataFromFile(QStringLiteral("Hertzsprung-Russel Diagram/HYG_data.txt"), &spreadsheet, AbstractFileFilter::ImportMode::Replace);

        if (!filter.lastError().isEmpty()) {
            std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
            return -1;
        }
    }

    qDebug() << "This is the number: " << spreadsheet.column(0)->rowCount();
    qDebug() << "This is it: " << spreadsheet.column(1)->doubleAt(119612);

    Spreadsheet spreadsheet2("temp to ci mapping");
    project.addChild(&spreadsheet2);

    {
        AsciiFilter filter2;

        auto p2 = filter2.properties();
        p2.headerEnabled = false;
        filter2.setProperties(p2);
        filter2.readDataFromFile(QStringLiteral("Hertzsprung-Russel Diagram/temp_to_ci_mapping.txt"), &spreadsheet2, AbstractFileFilter::ImportMode::Replace);

        if (!filter2.lastError().isEmpty()) {
            std::cout << "Import error: " << filter2.lastError().toStdString() << std::endl;
            return -1;
        }
    }

    Worksheet worksheet("Worksheet");
    project.addChild(&worksheet);

    worksheet.setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(25, Worksheet::Unit::Centimeter);
	worksheet.setPageRect(QRectF(0, 0, w, h));

    double ms = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
    worksheet.setLayout(Worksheet::Layout::VerticalLayout);
    worksheet.setLayoutTopMargin(ms);
	worksheet.setLayoutBottomMargin(ms);
	worksheet.setLayoutLeftMargin(ms);
	worksheet.setLayoutRightMargin(ms);

    worksheet.setLayoutHorizontalSpacing(ms);
    worksheet.setLayoutVerticalSpacing(ms);

    CartesianPlot plotArea("xy-plot");
	plotArea.setType(CartesianPlot::Type::FourAxes);
    plotArea.title()->setText(QStringLiteral("Hertzsprung-Russell diagram"));
    PlotArea::BorderType border = plotArea.plotArea()->borderType();
    border.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea.plotArea()->setBorderType(border);
    plotArea.setSymmetricPadding(false);
    plotArea.setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea.setVerticalPadding(Worksheet::convertToSceneUnits(2.8, Worksheet::Unit::Centimeter));
    plotArea.setRightPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea.setBottomPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    worksheet.addChild(&plotArea);

    for (Axis* axis : plotArea.children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom) {
            axis->title()->setText(QStringLiteral("Color Index (B-V)"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            axis->title()->setText(QStringLiteral("Absolute Magnitude"));
        } else if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Top) {
            axis->title()->setText(QStringLiteral("Temperature [K]"));
            axis->setTitleOffsetX(Worksheet::convertToSceneUnits(2, Worksheet::Unit::Point));
            axis->setTitleOffsetY(Worksheet::convertToSceneUnits(43, Worksheet::Unit::Point));
            axis->setLabelsOffset(Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
            axis->setLabelsPosition(Axis::LabelsPosition::In);
        }
    }

    XYCurve config1("config1");
    plotArea.addChild(&config1);
    config1.setPlotType(Plot::PlotType::Scatter);
    config1.setXColumn(spreadsheet.column(1));
    config1.setYColumn(spreadsheet.column(0));
    config1.symbol()->setStyle(Symbol::Style::Circle);
    config1.symbol()->setSize(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point));
    config1.setValuesType(XYCurve::ValuesType::NoValues);

    plotArea.enableAutoScale(CartesianCoordinateSystem::Dimension::X, 0, false);
    Range<double> rangeX = plotArea.range(CartesianCoordinateSystem::Dimension::X, 0);
    rangeX.setRange(-0.5, 2.5);
    plotArea.setRange(CartesianCoordinateSystem::Dimension::X, 0, rangeX);

    plotArea.enableAutoScale(CartesianCoordinateSystem::Dimension::Y, 0, false);
    Range<double> rangeY = plotArea.range(CartesianCoordinateSystem::Dimension::Y, 0);
    rangeY.setRange(20, -20);
    plotArea.setRange(CartesianCoordinateSystem::Dimension::Y, 0, rangeY);

    TextLabel textLabel("White Dwarfs");
    textLabel.setText(QStringLiteral("White Dwarfs"));
    worksheet.addChild(&textLabel);
    textLabel.setPosition({Worksheet::convertToSceneUnits(-1.8, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-8.7, Worksheet::Unit::Centimeter)});

    TextLabel textLabel1("Main Sequence");
    textLabel1.setText(QStringLiteral("Main Sequence"));
    worksheet.addChild(&textLabel1);
    textLabel1.setPosition({Worksheet::convertToSceneUnits(6.3, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-6.8, Worksheet::Unit::Centimeter)});

    TextLabel textLabel2("Giants");
    textLabel2.setText(QStringLiteral("Giants"));
    worksheet.addChild(&textLabel2);
    textLabel2.setPosition({Worksheet::convertToSceneUnits(5.4, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(2.1, Worksheet::Unit::Centimeter)});

    TextLabel textLabel3("Super Giants");
    textLabel3.setText(QStringLiteral("Super Giants"));
    worksheet.addChild(&textLabel3);
    textLabel3.setPosition({Worksheet::convertToSceneUnits(5.5, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(7.5, Worksheet::Unit::Centimeter)});

    worksheet.setTheme(QStringLiteral("Monokai"));

    worksheet.view()->show();

    app.exec();
}
