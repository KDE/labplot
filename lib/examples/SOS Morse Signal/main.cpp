#include <QApplication>

#include "labplot.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    Project project;

    AsciiFilter filter;

    auto p = filter.properties();
	p.headerEnabled = false;
	filter.setProperties(p);

	auto* spreadsheet = new Spreadsheet("data");
    project.addChild(spreadsheet);

    filter.readDataFromFile(QStringLiteral("SOS Morse Signal/data.txt"), spreadsheet, AbstractFileFilter::ImportMode::Replace);

    if (!filter.lastError().isEmpty()) {
		std::cout << "Import error: " << filter.lastError().toStdString() << std::endl;
		return -1;
	}

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* worksheet = new Worksheet("Worksheet");
    project.addChild(worksheet);

    worksheet->setUseViewSize(false);
    double w = Worksheet::convertToSceneUnits(25, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(25, Worksheet::Unit::Centimeter);
	worksheet->setPageRect(QRectF(0, 0, w, h));

    worksheet->setLayout(Worksheet::Layout::VerticalLayout);
    worksheet->setLayoutTopMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
	worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(13.5, Worksheet::Unit::Centimeter));

    worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));
    worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter));

    QFont fo7;
    fo7.setPointSizeF(Worksheet::convertToSceneUnits(7, Worksheet::Unit::Point));

    QFont fo10;
    fo10.setPointSizeF(10);
    QTextEdit te10;
    te10.setFont(fo10);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* plotArea1 = new CartesianPlot("Signal with noise");
	plotArea1->setType(CartesianPlot::Type::FourAxes);

    plotArea1->setSymmetricPadding(true);
    plotArea1->setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea1->setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border1 = plotArea1->plotArea()->borderType();
    border1.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border1.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea1->plotArea()->setBorderType(border1);
    plotArea1->plotArea()->borderLine()->setWidth(0);

    te10.setText(QStringLiteral("Signal with white noise"));
    plotArea1->title()->setText(te10.toHtml());

    plotArea1->enableAutoScale(CartesianCoordinateSystem::Dimension::X, -1, true);
    plotArea1->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, -1, true);

    for (Axis* axis : plotArea1->children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            te10.setText(QStringLiteral("t"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo7);
            axis->setScalingFactor(0.0001);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            te10.setText(QStringLiteral("signal"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo7);
        }
    }

    worksheet->addChild(plotArea1);

    auto* config11 = new XYCurve("signal");
    plotArea1->addChild(config11);
    config11->setPlotType(Plot::PlotType::Scatter);
    config11->setXColumn(spreadsheet->column(0));
    config11->setYColumn(spreadsheet->column(1));
    config11->setLineType(XYCurve::LineType::Line);
    config11->symbol()->setStyle(Symbol::Style::NoSymbols);
    config11->setValuesType(XYCurve::ValuesType::NoValues);
    config11->background()->setPosition(Background::Position::No);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* plotArea2 = new CartesianPlot("FFT");
	plotArea2->setType(CartesianPlot::Type::FourAxes);

    plotArea2->setSymmetricPadding(true);
    plotArea2->setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea2->setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border2 = plotArea2->plotArea()->borderType();
    border2.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border2.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea2->plotArea()->setBorderType(border2);
    plotArea2->plotArea()->borderLine()->setWidth(0);

    te10.setText(QStringLiteral("FFT"));
    plotArea2->title()->setText(te10.toHtml());

    plotArea2->enableAutoScale(CartesianCoordinateSystem::Dimension::X, -1, true);
    plotArea2->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, -1, true);

    for (Axis* axis : plotArea2->children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            te10.setText(QStringLiteral("frequency"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo7);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            te10.setText(QStringLiteral("amplitude"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo7);
        }
    }

    worksheet->addChild(plotArea2);

    auto* config21 = new XYFourierTransformCurve("Fourier transform");
    plotArea2->addChild(config21);
    config21->setXDataColumn(spreadsheet->column(0));
    config21->setYDataColumn(spreadsheet->column(1));
    XYFourierTransformCurve::TransformData tData21 = config21->transformData();
    tData21.type = nsl_dft_result_amplitude;
    tData21.windowType = nsl_sf_window_uniform;
    tData21.twoSided = false;
    tData21.shifted = false;
    tData21.xScale = nsl_dft_xscale_frequency;
    tData21.autoRange = true;
    config21->setTransformData(tData21);
    config21->setLineInterpolationPointsCount(1);
    config21->symbol()->setStyle(Symbol::Style::NoSymbols);
    config21->setValuesType(XYCurve::ValuesType::NoValues);
    config21->background()->setPosition(Background::Position::No);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    auto* plotArea3 = new CartesianPlot("Signal after Fourier Filter");
	plotArea3->setType(CartesianPlot::Type::FourAxes);

    plotArea3->setSymmetricPadding(true);
    plotArea3->setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
    plotArea3->setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

    PlotArea::BorderType border3 = plotArea3->plotArea()->borderType();
    border3.setFlag(PlotArea::BorderTypeFlags::BorderLeft, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderTop, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderRight, true);
	border3.setFlag(PlotArea::BorderTypeFlags::BorderBottom, true);
    plotArea3->plotArea()->setBorderType(border3);
    plotArea3->plotArea()->borderLine()->setWidth(0);

    te10.setText(QStringLiteral("Filtered signal"));
    plotArea3->title()->setText(te10.toHtml());

    plotArea3->enableAutoScale(CartesianCoordinateSystem::Dimension::X, -1, true);
    plotArea3->enableAutoScale(CartesianCoordinateSystem::Dimension::Y, -1, true);

    for (Axis* axis : plotArea3->children<Axis>()) {
        if (axis->orientation() == WorksheetElement::Orientation::Horizontal && axis->position() == Axis::Position::Bottom){
            te10.setText(QStringLiteral("t"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo7);
            axis->setScalingFactor(0.0001);
        } else if (axis->orientation() == WorksheetElement::Orientation::Vertical && axis->position() == Axis::Position::Left) {
            te10.setText(QStringLiteral("signal"));
            axis->title()->setText(te10.toHtml());
            axis->majorGridLine()->setStyle(Qt::PenStyle::SolidLine);
            axis->minorGridLine()->setStyle(Qt::PenStyle::DotLine);
            axis->setLabelsFont(fo7);
        }
    }

    worksheet->addChild(plotArea3);

    auto* config31 = new XYFourierFilterCurve("Fourier filter");
    plotArea3->addChild(config31);
    config31->setXDataColumn(spreadsheet->column(0));
    config31->setYDataColumn(spreadsheet->column(1));
    XYFourierFilterCurve::FilterData fData31 = config31->filterData();
    fData31.type = nsl_filter_type_band_pass;
    fData31.form = nsl_filter_form_ideal;
    fData31.unit = nsl_filter_cutoff_unit_frequency;
    fData31.cutoff = 0.0497;
    fData31.unit2 = nsl_filter_cutoff_unit_frequency;
    fData31.cutoff2 = 0.0503;
    fData31.autoRange = true;
    config31->setFilterData(fData31);
    config31->setLineInterpolationPointsCount(1);
    config31->symbol()->setStyle(Symbol::Style::NoSymbols);
    config31->setValuesType(XYCurve::ValuesType::NoValues);
    config31->background()->setPosition(Background::Position::No);

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    QFont f5i;
    f5i.setPointSize(5);
    f5i.setItalic(true);
    QTextEdit te5i;
    te5i.setFont(f5i);

    QFont f8;
    f8.setPointSize(8);
    QTextEdit te8;
    te8.setFont(f8);

    QTextEdit te;

    auto* textLabel1 = new TextLabel("Signal Info");
    worksheet->addChild(textLabel1);

    te.setFontPointSize(8);
    te.append("The data was generated using the following Octave code:\n");
    te.setFontPointSize(5);
    te.setFontItalic(true);
    te.append("t=1:1:4000;\nf=.05; % Signal frequency\nnoise=4; % RMS random noise\ns=sin(2*pi*f*t);\nspace=zeros(size(t)); % Small interval of silence\ndit=[space s ];\ndash=[space s s s s s];\ness=[space dit dit dit space]; % the letter \"S\" in Morse Code\noh=[space dash dash dash space];  % the letter \"O\" in Morse Code\nsos=[space ess oh ess space];  % \"SOS\" in Morse Code, surrounded by spaces\nsignal=std(sos);\nSNR=signal/std(noise.*randn(size(sos))) % Signal-To-Noise ratio\nnsos=sos+noise.*randn(size(sos));  % Add lots of random white noise\nnsos=nsos./max(sos);\n");
    te.setFontPointSize(8);
    te.setFontItalic(false);
    te.append("It creates a pulsed fixed frequency sine wave that spells out \"SOS\"\nin Morse code and adds random white noise so that the SNR is poor.");

    textLabel1->setText(te.toHtml());
    textLabel1->setPosition({Worksheet::convertToSceneUnits(5.9, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(8, Worksheet::Unit::Centimeter)});

    auto* textLabel2 = new TextLabel("Lower outer fence - label");
    worksheet->addChild(textLabel2);
    te8.setText("The white noise has a frequency spectrum that is spread out over<br>the entire range of frequencies. The signal itself is concentrated<br>mostly at a fixed frequency 0.05 but the presence of the Morse<br>Code pulses spreads out its spectrum.");
    textLabel2->setText(te8.toHtml());
    textLabel2->setPosition({Worksheet::convertToSceneUnits(5.9, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(0, Worksheet::Unit::Centimeter)});

    auto* textLabel3 = new TextLabel("Fourier Filter Info");
    worksheet->addChild(textLabel3);
    te8.setText("A Fourier bandpass filter tuned to the signal frequency isolates the<br>signal from the noise.<br><br>Try out different values for the width of the bandpass filter to see<br>how and when the signal starts emerging from the noise.");
    textLabel3->setText(te8.toHtml());
    textLabel3->setPosition({Worksheet::convertToSceneUnits(5.9, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(-8, Worksheet::Unit::Centimeter)});

    // ###################################################################################################################################################################
    // ###################################################################################################################################################################

    worksheet->view()->show();

    app.exec();
}
