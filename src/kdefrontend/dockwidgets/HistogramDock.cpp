/***************************************************************************
    File                 : HistogramDock.cpp
    Project              : LabPlot
    Description          : widget for Histogram properties
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Anu Mittal (anu22mittal@gmail.com)
    Copyright            : (C) 2018-2021 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "HistogramDock.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "kdefrontend/widgets/SymbolWidget.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include <QCompleter>
#include <QDir>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>
#include <QPainter>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class HistogramDock
  \brief  Provides a widget for editing the properties of the Histograms (2D-curves) currently selected in the project explorer.

  If more than one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/
HistogramDock::HistogramDock(QWidget* parent) : BaseDock(parent), cbDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 3, 2, 1, 1);

	//Tab "Symbols"
	auto* hboxLayout = new QHBoxLayout(ui.tabSymbol);
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	hboxLayout->addWidget(symbolWidget);
	hboxLayout->setContentsMargins(2,2,2,2);
	hboxLayout->setSpacing(2);

	//Tab "Values"
	gridLayout = qobject_cast<QGridLayout*>(ui.tabValues->layout());
	cbValuesColumn = new TreeViewComboBox(ui.tabValues);
	gridLayout->addWidget(cbValuesColumn, 2, 2, 1, 1);

	//add formats for numeric values
	ui.cbValuesNumericFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbValuesNumericFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbValuesNumericFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbValuesNumericFormat->addItem(i18n("Automatic (e)"), QVariant('g'));
	ui.cbValuesNumericFormat->addItem(i18n("Automatic (E)"), QVariant('G'));

	//add format for date, time and datetime values
	for (const auto& s : AbstractColumn::dateFormats())
		ui.cbValuesDateTimeFormat->addItem(s, QVariant(s));

	for (const auto& s : AbstractColumn::timeFormats())
		ui.cbValuesDateTimeFormat->addItem(s, QVariant(s));

	for (const auto& s1 : AbstractColumn::dateFormats()) {
		for (const auto& s2 : AbstractColumn::timeFormats())
			ui.cbValuesDateTimeFormat->addItem(s1 + ' ' + s2, QVariant(s1 + ' ' + s2));
	}

	ui.cbValuesDateTimeFormat->setEditable(true);

	//Tab "Filling"
	ui.cbFillingColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.bFillingOpen->setIcon( QIcon::fromTheme("document-open") );

	ui.leFillingFileName->setCompleter(new QCompleter(new QDirModel, this));

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	ui.leBinWidth->setValidator(new QDoubleValidator(ui.leBinWidth));
	ui.leBinRangesMin->setValidator(new QDoubleValidator(ui.leBinRangesMin));
	ui.leBinRangesMax->setValidator(new QDoubleValidator(ui.leBinRangesMax));

	//Slots
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &HistogramDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &HistogramDock::commentChanged);
	connect( ui.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( cbDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(dataColumnChanged(QModelIndex)) );
	connect( ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)) );
	connect( ui.cbOrientation, SIGNAL(currentIndexChanged(int)), this, SLOT(orientationChanged(int)));
	connect(ui.cbNormalization, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::normalizationChanged);
	connect( ui.cbBinningMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(binningMethodChanged(int)) );
	connect(ui.sbBinCount, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &HistogramDock::binCountChanged);
	connect(ui.leBinWidth, &QLineEdit::textChanged, this, &HistogramDock::binWidthChanged);
	connect( ui.chkAutoBinRanges, &QCheckBox::stateChanged, this, &HistogramDock::autoBinRangesChanged );
	connect( ui.leBinRangesMin, &QLineEdit::textChanged, this, &HistogramDock::binRangesMinChanged );
	connect( ui.leBinRangesMax, &QLineEdit::textChanged, this, &HistogramDock::binRangesMaxChanged );
	connect(ui.dteBinRangesMin, &QDateTimeEdit::dateTimeChanged, this, &HistogramDock::binRangesMinDateTimeChanged);
	connect(ui.dteBinRangesMax, &QDateTimeEdit::dateTimeChanged, this, &HistogramDock::binRangesMaxDateTimeChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::plotRangeChanged);

	//Line
	connect(ui.cbLineType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HistogramDock::lineTypeChanged);
	connect(ui.cbLineStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HistogramDock::lineStyleChanged);
	connect(ui.kcbLineColor, &KColorButton::changed, this, &HistogramDock::lineColorChanged);
	connect(ui.sbLineWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &HistogramDock::lineWidthChanged);
	connect(ui.sbLineOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &HistogramDock::lineOpacityChanged);

	//Values
	connect( ui.cbValuesType, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesTypeChanged(int)) );
	connect( cbValuesColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(valuesColumnChanged(QModelIndex)) );
	connect( ui.cbValuesPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesPositionChanged(int)) );
	connect( ui.sbValuesDistance, SIGNAL(valueChanged(double)), this, SLOT(valuesDistanceChanged(double)) );
	connect( ui.sbValuesRotation, SIGNAL(valueChanged(int)), this, SLOT(valuesRotationChanged(int)) );
	connect( ui.sbValuesOpacity, SIGNAL(valueChanged(int)), this, SLOT(valuesOpacityChanged(int)) );
	connect(ui.cbValuesNumericFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistogramDock::valuesNumericFormatChanged);
	connect(ui.sbValuesPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &HistogramDock::valuesPrecisionChanged);
	connect(ui.cbValuesDateTimeFormat, &QComboBox::currentTextChanged, this, &HistogramDock::valuesDateTimeFormatChanged);
	connect(ui.leValuesPrefix, &QLineEdit::textChanged, this, &HistogramDock::valuesPrefixChanged);
	connect(ui.leValuesSuffix, &QLineEdit::textChanged, this, &HistogramDock::valuesSuffixChanged);
	connect( ui.kfrValuesFont, SIGNAL(fontSelected(QFont)), this, SLOT(valuesFontChanged(QFont)) );
	connect( ui.kcbValuesColor, SIGNAL(changed(QColor)), this, SLOT(valuesColorChanged(QColor)) );

	//Filling
	connect(ui.chkFillingEnabled, &QCheckBox::stateChanged, this, &HistogramDock::fillingEnabledChanged);
	connect( ui.cbFillingType, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingTypeChanged(int)) );
	connect( ui.cbFillingColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingColorStyleChanged(int)) );
	connect( ui.cbFillingImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingImageStyleChanged(int)) );
	connect( ui.cbFillingBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingBrushStyleChanged(int)) );
	connect( ui.bFillingOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect(ui.leFillingFileName, &QLineEdit::textChanged, this, &HistogramDock::fileNameChanged);
	connect( ui.leFillingFileName, SIGNAL(textChanged(QString)), this, SLOT(fileNameChanged()) );
	connect( ui.kcbFillingFirstColor, SIGNAL(changed(QColor)), this, SLOT(fillingFirstColorChanged(QColor)) );
	connect( ui.kcbFillingSecondColor, SIGNAL(changed(QColor)), this, SLOT(fillingSecondColorChanged(QColor)) );
	connect( ui.sbFillingOpacity, SIGNAL(valueChanged(int)), this, SLOT(fillingOpacityChanged(int)) );

	//Error bars
	connect( ui.cbErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(errorTypeChanged(int)) );
	connect( ui.cbErrorBarsType, SIGNAL(currentIndexChanged(int)), this, SLOT(errorBarsTypeChanged(int)) );
	connect( ui.sbErrorBarsCapSize, SIGNAL(valueChanged(double)), this, SLOT(errorBarsCapSizeChanged(double)) );
	connect( ui.cbErrorBarsStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(errorBarsStyleChanged(int)) );
	connect( ui.kcbErrorBarsColor, SIGNAL(changed(QColor)), this, SLOT(errorBarsColorChanged(QColor)) );
	connect( ui.sbErrorBarsWidth, SIGNAL(valueChanged(double)), this, SLOT(errorBarsWidthChanged(double)) );
	connect( ui.sbErrorBarsOpacity, SIGNAL(valueChanged(int)), this, SLOT(errorBarsOpacityChanged(int)) );

	//template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Histogram);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &HistogramDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &HistogramDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &HistogramDock::info);

	ui.verticalLayout->addWidget(frame);

	retranslateUi();
	init();

	//TODO: activate the tab for error-bars again once the functionality is implemented
	ui.tabWidget->removeTab(5);
}

HistogramDock::~HistogramDock() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;
}

void HistogramDock::init() {
	//General
	//bins option
	ui.cbBinningMethod->addItem(i18n("By Number"));
	ui.cbBinningMethod->addItem(i18n("By Width"));
	ui.cbBinningMethod->addItem(i18n("Square-root"));
	ui.cbBinningMethod->addItem(i18n("Rice"));
	ui.cbBinningMethod->addItem(i18n("Sturges"));
	ui.cbBinningMethod->addItem(i18n("Doane"));
	ui.cbBinningMethod->addItem(i18n("Scott"));

	//histogram type
	ui.cbType->addItem(i18n("Ordinary Histogram"));
	ui.cbType->addItem(i18n("Cumulative Histogram"));
// 	ui.cbType->addItem(i18n("AvgShifted Histogram"));

	//Orientation
	ui.cbOrientation->addItem(i18n("Vertical"));
	ui.cbOrientation->addItem(i18n("Horizontal"));

	//Normalization
	ui.cbNormalization->addItem(i18n("Count"));
	ui.cbNormalization->addItem(i18n("Probability"));
	ui.cbNormalization->addItem(i18n("Count Density"));
	ui.cbNormalization->addItem(i18n("Probability Density"));

	//Line
	ui.cbLineType->addItem(i18n("None"));
	ui.cbLineType->addItem(i18n("Bars"));
	ui.cbLineType->addItem(i18n("Envelope"));
	ui.cbLineType->addItem(i18n("Drop Lines"));

	GuiTools::updatePenStyles(ui.cbLineStyle, Qt::black);
	m_initializing = false;

	//Values
	ui.cbValuesType->addItem(i18n("No Values"));
	ui.cbValuesType->addItem(i18n("Frequency"));
	ui.cbValuesType->addItem(i18n("Custom Column"));

	ui.cbValuesPosition->addItem(i18n("Above"));
	ui.cbValuesPosition->addItem(i18n("Below"));
	ui.cbValuesPosition->addItem(i18n("Left"));
	ui.cbValuesPosition->addItem(i18n("Right"));

	//Filling
	ui.cbFillingType->clear();
	ui.cbFillingType->addItem(i18n("Color"));
	ui.cbFillingType->addItem(i18n("Image"));
	ui.cbFillingType->addItem(i18n("Pattern"));

	ui.cbFillingColorStyle->clear();
	ui.cbFillingColorStyle->addItem(i18n("Single Color"));
	ui.cbFillingColorStyle->addItem(i18n("Horizontal Linear Gradient"));
	ui.cbFillingColorStyle->addItem(i18n("Vertical Linear Gradient"));
	ui.cbFillingColorStyle->addItem(i18n("Diagonal Linear Gradient (Start From Top Left)"));
	ui.cbFillingColorStyle->addItem(i18n("Diagonal Linear Gradient (Start From Bottom Left)"));
	ui.cbFillingColorStyle->addItem(i18n("Radial Gradient"));

	ui.cbFillingImageStyle->clear();
	ui.cbFillingImageStyle->addItem(i18n("Scaled and Cropped"));
	ui.cbFillingImageStyle->addItem(i18n("Scaled"));
	ui.cbFillingImageStyle->addItem(i18n("Scaled, Keep Proportions"));
	ui.cbFillingImageStyle->addItem(i18n("Centered"));
	ui.cbFillingImageStyle->addItem(i18n("Tiled"));
	ui.cbFillingImageStyle->addItem(i18n("Center Tiled"));
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, Qt::SolidPattern);

	//Error-bars
	QPainter pa;
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawLine(3,10,17,10);//vert. line
	pa.drawLine(10,3,10,17);//hor. line
	pa.end();
	ui.cbErrorBarsType->addItem(i18n("Bars"));
	ui.cbErrorBarsType->setItemIcon(0, pm);

	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,17,10); //vert. line
	pa.drawLine(10,3,10,17); //hor. line
	pa.drawLine(7,3,13,3); //upper cap
	pa.drawLine(7,17,13,17); //bottom cap
	pa.drawLine(3,7,3,13); //left cap
	pa.drawLine(17,7,17,13); //right cap
	pa.end();
	ui.cbErrorBarsType->addItem(i18n("Bars with Ends"));
	ui.cbErrorBarsType->setItemIcon(1, pm);

	ui.cbErrorType->addItem(i18n("No Errors"));

	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, Qt::black);
}

void HistogramDock::setModel() {
	m_aspectTreeModel->enablePlottableColumnsOnly(true);
	m_aspectTreeModel->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Datapicker,
	                       AspectType::DatapickerCurve, AspectType::Spreadsheet, AspectType::LiveDataSource,
	                       AspectType::Column, AspectType::Worksheet, AspectType::CartesianPlot,
	                       AspectType::XYFitCurve, AspectType::XYSmoothCurve, AspectType::CantorWorksheet};

	cbDataColumn->setTopLevelClasses(list);
	cbValuesColumn->setTopLevelClasses(list);

	list = {AspectType::Column};
	m_aspectTreeModel->setSelectableAspects(list);

	cbDataColumn->setModel(m_aspectTreeModel);
	cbValuesColumn->setModel(m_aspectTreeModel);
}

void HistogramDock::setCurves(QList<Histogram*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	setModel();

	QList<Symbol*> symbols;
	for (auto* curve : m_curvesList)
		symbols << curve->symbol();
	symbolWidget->setSymbols(symbols);

	SET_NUMBER_LOCALE
	ui.sbLineWidth->setLocale(numberLocale);
	ui.sbValuesDistance->setLocale(numberLocale);
	ui.sbErrorBarsCapSize->setLocale(numberLocale);
	ui.sbErrorBarsWidth->setLocale(numberLocale);
	symbolWidget->updateLocale();

	//if there are more then one curve in the list, disable the content in the tab "general"
	if (m_curvesList.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.lXColumn->setEnabled(true);
		cbDataColumn->setEnabled(true);

		cbDataColumn->setColumn(m_curve->dataColumn(), m_curve->dataColumnPath());
		cbValuesColumn->setColumn(m_curve->valuesColumn(), m_curve->valuesColumnPath());
		ui.leName->setText(m_curve->name());
		ui.leComment->setText(m_curve->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.lXColumn->setEnabled(false);
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());
		cbValuesColumn->setCurrentModelIndex(QModelIndex());

		ui.leName->setText(QString());
		ui.leComment->setText(QString());
	}

	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first curve
	ui.cbType->setCurrentIndex(m_curve->type());
	ui.cbOrientation->setCurrentIndex(m_curve->orientation());
	ui.cbNormalization->setCurrentIndex(m_curve->normalization());
	ui.cbBinningMethod->setCurrentIndex(m_curve->binningMethod());
	ui.sbBinCount->setValue(m_curve->binCount());
	ui.leBinWidth->setText(numberLocale.toString(m_curve->binWidth()));
	ui.chkAutoBinRanges->setChecked(m_curve->autoBinRanges());
	ui.leBinRangesMin->setText( numberLocale.toString(m_curve->binRangesMin()) );
	ui.leBinRangesMax->setText( numberLocale.toString(m_curve->binRangesMax()) );
	ui.chkVisible->setChecked( m_curve->isVisible() );

	//handle numeric vs. datetime widgets
	//TODO: we need to react on range format changes in the plot in general,
	//add signal-slot connection for this
	const auto* plot = static_cast<const CartesianPlot*>(m_curve->parent(AspectType::CartesianPlot));
	ui.dteBinRangesMin->setDisplayFormat(plot->xRangeDateTimeFormat());
	ui.dteBinRangesMax->setDisplayFormat(plot->xRangeDateTimeFormat());
	ui.dteBinRangesMin->setDateTime(QDateTime::fromMSecsSinceEpoch(m_curve->binRangesMin()));
	ui.dteBinRangesMax->setDateTime(QDateTime::fromMSecsSinceEpoch(m_curve->binRangesMax()));

	bool numeric = (plot->xRangeFormat() == RangeT::Format::Numeric);

	ui.lBinRangesMin->setVisible(numeric);
	ui.lBinRangesMax->setVisible(numeric);
	ui.leBinRangesMin->setVisible(numeric);
	ui.leBinRangesMax->setVisible(numeric);

	ui.lBinRangesMinDateTime->setVisible(!numeric);
	ui.dteBinRangesMin->setVisible(!numeric);
	ui.lBinRangesMaxDateTime->setVisible(!numeric);
	ui.dteBinRangesMax->setVisible(!numeric);

	//load the remaining properties
	KConfig config(QString(), KConfig::SimpleConfig);
	loadConfig(config);

	updatePlotRanges();

	//Slots
	//General-tab
	connect(m_curve, &Histogram::aspectDescriptionChanged, this, &HistogramDock::curveDescriptionChanged);
	connect(m_curve, &Histogram::dataColumnChanged, this, &HistogramDock::curveDataColumnChanged);
	connect(m_curve, &Histogram::typeChanged, this, &HistogramDock::curveTypeChanged);
	connect(m_curve, &Histogram::orientationChanged, this, &HistogramDock::curveOrientationChanged);
	connect(m_curve, &Histogram::normalizationChanged, this, &HistogramDock::curveNormalizationChanged);
	connect(m_curve, &Histogram::binningMethodChanged, this, &HistogramDock::curveBinningMethodChanged);
	connect(m_curve, &Histogram::binCountChanged, this, &HistogramDock::curveBinCountChanged);
	connect(m_curve, &Histogram::binWidthChanged, this, &HistogramDock::curveBinWidthChanged);
	connect(m_curve, &Histogram::autoBinRangesChanged, this, &HistogramDock::curveAutoBinRangesChanged);
	connect(m_curve, &Histogram::binRangesMinChanged, this, &HistogramDock::curveBinRangesMinChanged);
	connect(m_curve, &Histogram::binRangesMaxChanged, this, &HistogramDock::curveBinRangesMaxChanged);
	connect(m_curve, &Histogram::visibilityChanged, this, &HistogramDock::curveVisibilityChanged);

	//Line-tab
	connect(m_curve, &Histogram::linePenChanged, this, &HistogramDock::curveLinePenChanged);
	connect(m_curve, &Histogram::lineOpacityChanged, this, &HistogramDock::curveLineOpacityChanged);

	//Values-Tab
	connect(m_curve, &Histogram::valuesTypeChanged, this, &HistogramDock::curveValuesTypeChanged);
	connect(m_curve, &Histogram::valuesColumnChanged, this, &HistogramDock::curveValuesColumnChanged);
	connect(m_curve, &Histogram::valuesPositionChanged, this, &HistogramDock::curveValuesPositionChanged);
	connect(m_curve, &Histogram::valuesDistanceChanged, this, &HistogramDock::curveValuesDistanceChanged);
	connect(m_curve, &Histogram::valuesOpacityChanged, this, &HistogramDock::curveValuesOpacityChanged);
	connect(m_curve, &Histogram::valuesRotationAngleChanged, this, &HistogramDock::curveValuesRotationAngleChanged);
	connect(m_curve, &Histogram::valuesNumericFormatChanged, this, &HistogramDock::curveValuesNumericFormatChanged);
	connect(m_curve, &Histogram::valuesPrecisionChanged, this, &HistogramDock::curveValuesPrecisionChanged);
	connect(m_curve, &Histogram::valuesDateTimeFormatChanged, this, &HistogramDock::curveValuesDateTimeFormatChanged);
	connect(m_curve, &Histogram::valuesPrefixChanged, this, &HistogramDock::curveValuesPrefixChanged);
	connect(m_curve, &Histogram::valuesSuffixChanged, this, &HistogramDock::curveValuesSuffixChanged);
	connect(m_curve, &Histogram::valuesFontChanged, this, &HistogramDock::curveValuesFontChanged);
	connect(m_curve, &Histogram::valuesColorChanged, this, &HistogramDock::curveValuesColorChanged);

	//Filling-Tab
	connect( m_curve, &Histogram::fillingEnabledChanged, this, &HistogramDock::curveFillingEnabledChanged);
	connect( m_curve, &Histogram::fillingTypeChanged, this, &HistogramDock::curveFillingTypeChanged);
	connect( m_curve, &Histogram::fillingColorStyleChanged, this, &HistogramDock::curveFillingColorStyleChanged);
	connect( m_curve, &Histogram::fillingImageStyleChanged, this, &HistogramDock::curveFillingImageStyleChanged);
	connect( m_curve, &Histogram::fillingBrushStyleChanged, this, &HistogramDock::curveFillingBrushStyleChanged);
	connect( m_curve, &Histogram::fillingFirstColorChanged, this, &HistogramDock::curveFillingFirstColorChanged);
	connect( m_curve, &Histogram::fillingSecondColorChanged, this, &HistogramDock::curveFillingSecondColorChanged);
	connect( m_curve, &Histogram::fillingFileNameChanged, this, &HistogramDock::curveFillingFileNameChanged);
	connect( m_curve, &Histogram::fillingOpacityChanged, this, &HistogramDock::curveFillingOpacityChanged);

	//"Error bars"-Tab
	connect(m_curve, &Histogram::errorTypeChanged, this, &HistogramDock::curveErrorTypeChanged);
	connect(m_curve, &Histogram::errorBarsCapSizeChanged, this, &HistogramDock::curveErrorBarsCapSizeChanged);
	connect(m_curve, &Histogram::errorBarsTypeChanged, this, &HistogramDock::curveErrorBarsTypeChanged);
	connect(m_curve, &Histogram::errorBarsPenChanged, this, &HistogramDock::curveErrorBarsPenChanged);
	connect(m_curve, &Histogram::errorBarsOpacityChanged, this, &HistogramDock::curveErrorBarsOpacityChanged);

	m_initializing = false;
}

void HistogramDock::retranslateUi() {
	//TODO:
// 	ui.lName->setText(i18n("Name"));
// 	ui.lComment->setText(i18n("Comment"));
// 	ui.chkVisible->setText(i18n("Visible"));
// 	ui.lXColumn->setText(i18n("x-data"));
// 	ui.lYColumn->setText(i18n("y-data"));

	//TODO updatePenStyles, updateBrushStyles for all comboboxes
}
void HistogramDock::updatePlotRanges() const {
	const int cSystemCount{ m_curve->coordinateSystemCount() };
	const int cSystemIndex{ m_curve->coordinateSystemIndex() };
	DEBUG(Q_FUNC_INFO << ", plot ranges count: " << cSystemCount)
	DEBUG(Q_FUNC_INFO << ", current plot range: " << cSystemIndex+1)

	// fill ui.cbPlotRanges
	ui.cbPlotRanges->clear();
	for (int i{0}; i < cSystemCount; i++)
		ui.cbPlotRanges->addItem( QString::number(i+1) + QLatin1String(" : ") + m_curve->coordinateSystemInfo(i) );
	ui.cbPlotRanges->setCurrentIndex(cSystemIndex);
	// disable when there is only on plot range
	ui.cbPlotRanges->setEnabled(cSystemCount == 1 ? false : true);
}

//*************************************************************
//**** SLOTs for changes triggered in HistogramDock *****
//*************************************************************

// "General"-tab
void HistogramDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setVisible(state);
}

void HistogramDock::typeChanged(int index) {
	if (m_initializing)
		return;

	auto histogramType = Histogram::HistogramType(index);
	for (auto* curve : m_curvesList)
		curve->setType(histogramType);
}

void HistogramDock::dataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_curvesList)
		curve->setDataColumn(column);
}

void HistogramDock::orientationChanged(int index) {
	if (m_initializing)
		return;

	auto orientation = Histogram::HistogramOrientation(index);
	for (auto* curve : m_curvesList)
		curve->setOrientation(orientation);
}

void HistogramDock::normalizationChanged(int index) {
	if (m_initializing)
		return;

	auto normalization = Histogram::HistogramNormalization(index);
	for (auto* curve : m_curvesList)
		curve->setNormalization(normalization);
}

void HistogramDock::binningMethodChanged(int index) {
	const auto binningMethod = Histogram::BinningMethod(index);
	if (binningMethod == Histogram::ByNumber) {
		ui.lBinCount->show();
		ui.sbBinCount->show();
		ui.lBinWidth->hide();
		ui.leBinWidth->hide();
	} else if (binningMethod == Histogram::ByWidth) {
		ui.lBinCount->hide();
		ui.sbBinCount->hide();
		ui.lBinWidth->show();
		ui.leBinWidth->show();
	} else {
		ui.lBinCount->hide();
		ui.sbBinCount->hide();
		ui.lBinWidth->hide();
		ui.leBinWidth->hide();
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setBinningMethod(binningMethod);
}

void HistogramDock::binCountChanged(int value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setBinCount(value);
}

void HistogramDock::binWidthChanged() {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double width{numberLocale.toDouble(ui.leBinWidth->text(), &ok)};
	if (ok) {
		for (auto* curve : m_curvesList)
			curve->setBinWidth(width);
	}
}

void HistogramDock::autoBinRangesChanged(int state) {
	bool checked = (state == Qt::Checked);
	ui.leBinRangesMin->setEnabled(!checked);
	ui.leBinRangesMax->setEnabled(!checked);
	ui.dteBinRangesMin->setEnabled(!checked);
	ui.dteBinRangesMax->setEnabled(!checked);

	if (m_initializing)
		return;

	for (auto* hist : m_curvesList)
		hist->setAutoBinRanges(checked);
}

void HistogramDock::binRangesMinChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double min{numberLocale.toDouble(value, &ok)};
	if (ok) {
		for (auto* hist : m_curvesList)
			hist->setBinRangesMin(min);
	}
}

void HistogramDock::binRangesMaxChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double max{numberLocale.toDouble(value, &ok)};
	if (ok) {
		for (auto* hist : m_curvesList)
			hist->setBinRangesMax(max);
	}
}

void HistogramDock::binRangesMinDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 min = dateTime.toMSecsSinceEpoch();
	for (auto* hist : m_curvesList)
		hist->setBinRangesMin(min);
}

void HistogramDock::binRangesMaxDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 max = dateTime.toMSecsSinceEpoch();
	for (auto* hist : m_curvesList)
		hist->setBinRangesMax(max);
}

void HistogramDock::plotRangeChanged(int index) {
	DEBUG(Q_FUNC_INFO << ", index = " << index)
	const auto* plot = dynamic_cast<const CartesianPlot*>(m_curve->parentAspect());
	if (index < 0 || index > plot->coordinateSystemCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}

	if (index != m_curve->coordinateSystemIndex()) {
		m_curve->setCoordinateSystemIndex(index);
		updateLocale();		// update line edits
		m_curve->retransform();	// redraw
	}
}

//Line tab
void HistogramDock::lineTypeChanged(int index) {
	auto lineType = Histogram::LineType(index);

	if ( lineType == Histogram::NoLine) {
		ui.cbLineStyle->setEnabled(false);
		ui.kcbLineColor->setEnabled(false);
		ui.sbLineWidth->setEnabled(false);
		ui.sbLineOpacity->setEnabled(false);
	} else {
		ui.cbLineStyle->setEnabled(true);
		ui.kcbLineColor->setEnabled(true);
		ui.sbLineWidth->setEnabled(true);
		ui.sbLineOpacity->setEnabled(true);
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineType(lineType);
}

void HistogramDock::lineStyleChanged(int index) {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->linePen();
		pen.setStyle(penStyle);
		curve->setLinePen(pen);
	}
}

void HistogramDock::lineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->linePen();
		pen.setColor(color);
		curve->setLinePen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, color);
	m_initializing = false;
}

void HistogramDock::lineWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->linePen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		curve->setLinePen(pen);
	}
}

void HistogramDock::lineOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curvesList)
		curve->setLineOpacity(opacity);
}

//Values tab
/*!
  called when the type of the values (none, x, y, (x,y) etc.) was changed.
*/
void HistogramDock::valuesTypeChanged(int index) {
	if (m_initializing)
		return;

	this->updateValuesWidgets();

	auto valuesType = Histogram::ValuesType(index);
	for (auto* curve : m_curvesList)
		curve->setValuesType(valuesType);
}

/*!
  depending on the currently selected values column type (column mode) updates the widgets for the values column format,
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the values column was changed.
*/
void HistogramDock::updateValuesWidgets() {
	const auto type = Histogram::ValuesType(ui.cbValuesType->currentIndex());
	bool showValues = (type != Histogram::NoValues);

	ui.cbValuesPosition->setEnabled(showValues);
	ui.sbValuesDistance->setEnabled(showValues);
	ui.sbValuesRotation->setEnabled(showValues);
	ui.sbValuesOpacity->setEnabled(showValues);
	ui.kfrValuesFont->setEnabled(showValues);
	ui.kcbValuesColor->setEnabled(showValues);

	bool hasInteger = false;
	bool hasNumeric = false;
	bool hasDateTime = false;

	if (type == Histogram::ValuesCustomColumn) {
		ui.lValuesColumn->show();
		cbValuesColumn->show();

		auto* column = static_cast<Column*>(cbValuesColumn->currentModelIndex().internalPointer());
		if (column) {
			if (column->columnMode() == AbstractColumn::ColumnMode::Numeric)
				hasNumeric = true;
			else if (column->columnMode() == AbstractColumn::ColumnMode::Integer || column->columnMode() == AbstractColumn::ColumnMode::BigInt)
				hasInteger = true;
			else if (column->columnMode() == AbstractColumn::ColumnMode::DateTime)
				hasDateTime = true;
		}
	} else {
		ui.lValuesColumn->hide();
		cbValuesColumn->hide();

		if (type == Histogram::ValuesBinEntries)
			hasInteger = true;
	}

	//hide all the format related widgets first and
	//then show only what is required depending of the column mode(s)
	ui.lValuesFormat->hide();
	ui.lValuesNumericFormat->hide();
	ui.cbValuesNumericFormat->hide();
	ui.lValuesPrecision->hide();
	ui.sbValuesPrecision->hide();
	ui.lValuesDateTimeFormat->hide();
	ui.cbValuesDateTimeFormat->hide();

	if (hasNumeric || hasInteger) {
		ui.lValuesFormat->show();
		ui.lValuesNumericFormat->show();
		ui.cbValuesNumericFormat->show();
	}

	//precision is only available for Numeric
	if (hasNumeric) {
		ui.lValuesPrecision->show();
		ui.sbValuesPrecision->show();
	}

	if (hasDateTime) {
		ui.lValuesFormat->show();
		ui.lValuesDateTimeFormat->show();
		ui.cbValuesDateTimeFormat->show();
	}
}

/*!
  called when the custom column for the values was changed.
*/
void HistogramDock::valuesColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	this->updateValuesWidgets();

	auto* column = static_cast<Column*>(index.internalPointer());
	for (auto* curve : m_curvesList)
		curve->setValuesColumn(column);
}

void HistogramDock::valuesPositionChanged(int index) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesPosition(Histogram::ValuesPosition(index));
}

void HistogramDock::valuesDistanceChanged(double  value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesDistance( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void HistogramDock::valuesRotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesRotationAngle(value);
}

void HistogramDock::valuesOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curvesList)
		curve->setValuesOpacity(opacity);
}

void HistogramDock::valuesNumericFormatChanged(int index) {
	if (m_initializing)
		return;

	char format = ui.cbValuesNumericFormat->itemData(index).toChar().toLatin1();
	for (auto* curve : m_curvesList)
		curve->setValuesNumericFormat(format);
}

void HistogramDock::valuesPrecisionChanged(int precision) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesPrecision(precision);
}

void HistogramDock::valuesDateTimeFormatChanged(const QString& format) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesDateTimeFormat(format);
}

void HistogramDock::valuesPrefixChanged() {
	if (m_initializing)
		return;

	QString prefix = ui.leValuesPrefix->text();
	for (auto* curve : m_curvesList)
		curve->setValuesPrefix(prefix);
}

void HistogramDock::valuesSuffixChanged() {
	if (m_initializing)
		return;

	QString suffix = ui.leValuesSuffix->text();
	for (auto* curve : m_curvesList)
		curve->setValuesSuffix(suffix);
}

void HistogramDock::valuesFontChanged(const QFont& font) {
	if (m_initializing)
		return;

	QFont valuesFont = font;
	valuesFont.setPixelSize( Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point) );
	for (auto* curve : m_curvesList)
		curve->setValuesFont(valuesFont);
}

void HistogramDock::valuesColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesColor(color);
}

//Filling-tab
void HistogramDock::fillingEnabledChanged(int state) {
	ui.cbFillingType->setEnabled(state);
	ui.cbFillingColorStyle->setEnabled(state);
	ui.cbFillingBrushStyle->setEnabled(state);
	ui.cbFillingImageStyle->setEnabled(state);
	ui.kcbFillingFirstColor->setEnabled(state);
	ui.kcbFillingSecondColor->setEnabled(state);
	ui.leFillingFileName->setEnabled(state);
	ui.bFillingOpen->setEnabled(state);
	ui.sbFillingOpacity->setEnabled(state);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setFillingEnabled(state);
}

void HistogramDock::fillingTypeChanged(int index) {
	auto type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::BackgroundType::Color) {
		ui.lFillingColorStyle->show();
		ui.cbFillingColorStyle->show();
		ui.lFillingImageStyle->hide();
		ui.cbFillingImageStyle->hide();
		ui.lFillingBrushStyle->hide();
		ui.cbFillingBrushStyle->hide();

		ui.lFillingFileName->hide();
		ui.leFillingFileName->hide();
		ui.bFillingOpen->hide();

		ui.lFillingFirstColor->show();
		ui.kcbFillingFirstColor->show();

		auto style = (PlotArea::BackgroundColorStyle) ui.cbFillingColorStyle->currentIndex();
		if (style == PlotArea::BackgroundColorStyle::SingleColor) {
			ui.lFillingFirstColor->setText(i18n("Color:"));
			ui.lFillingSecondColor->hide();
			ui.kcbFillingSecondColor->hide();
		} else {
			ui.lFillingFirstColor->setText(i18n("First color:"));
			ui.lFillingSecondColor->show();
			ui.kcbFillingSecondColor->show();
		}
	} else if (type == PlotArea::BackgroundType::Image) {
		ui.lFillingColorStyle->hide();
		ui.cbFillingColorStyle->hide();
		ui.lFillingImageStyle->show();
		ui.cbFillingImageStyle->show();
		ui.lFillingBrushStyle->hide();
		ui.cbFillingBrushStyle->hide();
		ui.lFillingFileName->show();
		ui.leFillingFileName->show();
		ui.bFillingOpen->show();

		ui.lFillingFirstColor->hide();
		ui.kcbFillingFirstColor->hide();
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();
	} else if (type == PlotArea::BackgroundType::Pattern) {
		ui.lFillingFirstColor->setText(i18n("Color:"));
		ui.lFillingColorStyle->hide();
		ui.cbFillingColorStyle->hide();
		ui.lFillingImageStyle->hide();
		ui.cbFillingImageStyle->hide();
		ui.lFillingBrushStyle->show();
		ui.cbFillingBrushStyle->show();
		ui.lFillingFileName->hide();
		ui.leFillingFileName->hide();
		ui.bFillingOpen->hide();

		ui.lFillingFirstColor->show();
		ui.kcbFillingFirstColor->show();
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setFillingType(type);
}

void HistogramDock::fillingColorStyleChanged(int index) {
	auto style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::BackgroundColorStyle::SingleColor) {
		ui.lFillingFirstColor->setText(i18n("Color:"));
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();
	} else {
		ui.lFillingFirstColor->setText(i18n("First color:"));
		ui.lFillingSecondColor->show();
		ui.kcbFillingSecondColor->show();
		ui.lFillingBrushStyle->hide();
		ui.cbFillingBrushStyle->hide();
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setFillingColorStyle(style);
}

void HistogramDock::fillingImageStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (PlotArea::BackgroundImageStyle)index;
	for (auto* curve : m_curvesList)
		curve->setFillingImageStyle(style);
}

void HistogramDock::fillingBrushStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* curve : m_curvesList)
		curve->setFillingBrushStyle(style);
}

void HistogramDock::fillingFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setFillingFirstColor(c);
}

void HistogramDock::fillingSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setFillingSecondColor(c);
}

//"Error bars"-Tab
void HistogramDock::errorTypeChanged(int index) const {
	bool b = (index != 0);
	ui.lErrorData->setVisible(b);
	ui.lErrorFormat->setVisible(b);
	ui.lErrorBarsType->setVisible(b);
	ui.cbErrorBarsType->setVisible(b);
	ui.lErrorBarsStyle->setVisible(b);
	ui.cbErrorBarsStyle->setVisible(b);
	ui.lErrorBarsColor->setVisible(b);
	ui.kcbErrorBarsColor->setVisible(b);
	ui.lErrorBarsWidth->setVisible(b);
	ui.sbErrorBarsWidth->setVisible(b);
	ui.lErrorBarsOpacity->setVisible(b);
	ui.sbErrorBarsOpacity->setVisible(b);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setErrorType(Histogram::ErrorType(index));
}

void HistogramDock::errorBarsTypeChanged(int index) const {
	auto type{XYCurve::ErrorBarsType(index)};
	bool b = (type == XYCurve::ErrorBarsType::WithEnds);
	ui.lErrorBarsCapSize->setVisible(b);
	ui.sbErrorBarsCapSize->setVisible(b);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setErrorBarsType(type);
}

void HistogramDock::errorBarsCapSizeChanged(double value) const {
	if (m_initializing)
		return;

	double size = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : m_curvesList)
		curve->setErrorBarsCapSize(size);
}

void HistogramDock::errorBarsStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->errorBarsPen();
		pen.setStyle(penStyle);
		curve->setErrorBarsPen(pen);
	}
}

void HistogramDock::errorBarsColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->errorBarsPen();
		pen.setColor(color);
		curve->setErrorBarsPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, color);
	m_initializing = false;
}

void HistogramDock::errorBarsWidthChanged(double value) const {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->errorBarsPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		curve->setErrorBarsPen(pen);
	}
}

void HistogramDock::errorBarsOpacityChanged(int value) const {
	if (m_initializing)
		return;

	qreal opacity = (double)value/100.;
	for (auto* curve : m_curvesList)
		curve->setErrorBarsOpacity(opacity);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void HistogramDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "HistogramDock");
	QString dir = conf.readEntry("LastImageDir", "");

	QString formats;
	for (const QByteArray& format : QImageReader::supportedImageFormats()) {
		QString f = "*." + QString(format.constData());
		if (f == QLatin1String("*.svg"))
			continue;
		formats.isEmpty() ? formats += f : formats += ' ' + f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leFillingFileName->setText( path );

	for (auto* curve : m_curvesList)
		curve->setFillingFileName(path);
}

void HistogramDock::fileNameChanged() {
	if (m_initializing)
		return;

	QString fileName = ui.leFillingFileName->text();
	for (auto* curve : m_curvesList)
		curve->setFillingFileName(fileName);
}

void HistogramDock::fillingOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (double)value/100.;
	for (auto* curve : m_curvesList)
		curve->setFillingOpacity(opacity);
}

//*************************************************************
//*********** SLOTs for changes triggered in Histogram *******
//*************************************************************

//General-Tab
void HistogramDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());

	m_initializing = false;
}

void HistogramDock::curveDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbDataColumn->setColumn(column, m_curve->dataColumnPath());
	m_initializing = false;
}

void HistogramDock::curveTypeChanged(Histogram::HistogramType type) {
	m_initializing = true;
	ui.cbType->setCurrentIndex((int)type);
	m_initializing = false;
}

void HistogramDock::curveOrientationChanged(Histogram::HistogramOrientation orientation) {
	m_initializing = true;
	ui.cbOrientation->setCurrentIndex((int)orientation);
	m_initializing = false;
}

void HistogramDock::curveNormalizationChanged(Histogram::HistogramNormalization normalization) {
	m_initializing = true;
	ui.cbNormalization->setCurrentIndex((int)normalization);
	m_initializing = false;
}

void HistogramDock::curveBinningMethodChanged(Histogram::BinningMethod method) {
	m_initializing = true;
	ui.cbBinningMethod->setCurrentIndex((int)method);
	m_initializing = false;
}

void HistogramDock::curveBinCountChanged(int count) {
	m_initializing = true;
	ui.sbBinCount->setValue(count);
	m_initializing = false;
}

void HistogramDock::curveBinWidthChanged(double width) {
	if (m_initializing)return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leBinWidth->setText(numberLocale.toString(width));
}

void HistogramDock::curveAutoBinRangesChanged(bool value) {
	m_initializing = true;
	ui.chkAutoBinRanges->setChecked(value);
	m_initializing = false;
}

void HistogramDock::curveBinRangesMinChanged(double value) {
	if (m_initializing)return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leBinRangesMin->setText(numberLocale.toString(value));
	ui.dteBinRangesMin->setDateTime(QDateTime::fromMSecsSinceEpoch(value));
}

void HistogramDock::curveBinRangesMaxChanged(double value) {
	if (m_initializing)return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leBinRangesMax->setText(numberLocale.toString(value));
	ui.dteBinRangesMax->setDateTime(QDateTime::fromMSecsSinceEpoch(value));
}

//Line-Tab
void HistogramDock::curveLineTypeChanged(Histogram::LineType type) {
	m_initializing = true;
	ui.cbLineType->setCurrentIndex((int)type);
	m_initializing = false;
}
void HistogramDock::curveLinePenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbLineStyle->setCurrentIndex( (int)pen.style());
	ui.kcbLineColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbLineStyle, pen.color());
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits( pen.widthF(), Worksheet::Unit::Point) );
	m_initializing = false;
}
void HistogramDock::curveLineOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbLineOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//Values-Tab
void HistogramDock::curveValuesTypeChanged(Histogram::ValuesType type) {
	m_initializing = true;
	ui.cbValuesType->setCurrentIndex((int) type);
	m_initializing = false;
}
void HistogramDock::curveValuesColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbValuesColumn->setColumn(column, m_curve->valuesColumnPath());
	m_initializing = false;
}
void HistogramDock::curveValuesPositionChanged(Histogram::ValuesPosition position) {
	m_initializing = true;
	ui.cbValuesPosition->setCurrentIndex((int) position);
	m_initializing = false;
}
void HistogramDock::curveValuesDistanceChanged(qreal distance) {
	m_initializing = true;
	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(distance, Worksheet::Unit::Point) );
	m_initializing = false;
}
void HistogramDock::curveValuesRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbValuesRotation->setValue(angle);
	m_initializing = false;
}
void HistogramDock::curveValuesOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbValuesOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}
void HistogramDock::curveValuesNumericFormatChanged(char format) {
	m_initializing = true;
	ui.cbValuesNumericFormat->setCurrentIndex(ui.cbValuesNumericFormat->findData(format));
	m_initializing = false;
}
void HistogramDock::curveValuesPrecisionChanged(int precision) {
	m_initializing = true;
	ui.sbValuesPrecision->setValue(precision);
	m_initializing = false;
}
void HistogramDock::curveValuesDateTimeFormatChanged(const QString& format) {
	m_initializing = true;
	ui.cbValuesDateTimeFormat->setCurrentText(format);
	m_initializing = false;
}
void HistogramDock::curveValuesPrefixChanged(const QString& prefix) {
	m_initializing = true;
  	ui.leValuesPrefix->setText(prefix);
	m_initializing = false;
}
void HistogramDock::curveValuesSuffixChanged(const QString& suffix) {
	m_initializing = true;
  	ui.leValuesSuffix->setText(suffix);
	m_initializing = false;
}
void HistogramDock::curveValuesFontChanged(QFont font) {
	m_initializing = true;
	font.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)) );
  	ui.kfrValuesFont->setFont(font);
	m_initializing = false;
}
void HistogramDock::curveValuesColorChanged(QColor color) {
	m_initializing = true;
  	ui.kcbValuesColor->setColor(color);
	m_initializing = false;
}

void HistogramDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//Filling
void HistogramDock::curveFillingEnabledChanged(bool status) {
	m_initializing = true;
	ui.chkFillingEnabled->setChecked(status);
	m_initializing = false;
}
void HistogramDock::curveFillingTypeChanged(PlotArea::BackgroundType type) {
	m_initializing = true;
	ui.cbFillingType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}
void HistogramDock::curveFillingColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbFillingColorStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}
void HistogramDock::curveFillingImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbFillingImageStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}
void HistogramDock::curveFillingBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbFillingBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}
void HistogramDock::curveFillingFirstColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbFillingFirstColor->setColor(color);
	m_initializing = false;
}
void HistogramDock::curveFillingSecondColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbFillingSecondColor->setColor(color);
	m_initializing = false;
}
void HistogramDock::curveFillingFileNameChanged(QString& filename) {
	m_initializing = true;
	ui.leFillingFileName->setText(filename);
	m_initializing = false;
}
void HistogramDock::curveFillingOpacityChanged(double opacity) {
	m_initializing = true;
	ui.sbFillingOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//"Error bars"-Tab
void HistogramDock::curveErrorTypeChanged(Histogram::ErrorType type) {
	m_initializing = true;
	ui.cbErrorType->setCurrentIndex((int)type);
	m_initializing = false;
}
void HistogramDock::curveErrorBarsCapSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbErrorBarsCapSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
	m_initializing = false;
}
void HistogramDock::curveErrorBarsTypeChanged(XYCurve::ErrorBarsType type) {
	m_initializing = true;
	ui.cbErrorBarsType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}
void HistogramDock::curveErrorBarsPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbErrorBarsStyle->setCurrentIndex( (int) pen.style());
	ui.kcbErrorBarsColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, pen.color());
	ui.sbErrorBarsWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Unit::Point) );
	m_initializing = false;
}
void HistogramDock::curveErrorBarsOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbErrorBarsOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void HistogramDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("Histogram"));

  	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.
	//This data is read in HistogramDock::setCurves().

	//Line
	ui.cbLineType->setCurrentIndex( group.readEntry("LineType", (int) m_curve->lineType()) );
	ui.cbLineStyle->setCurrentIndex( group.readEntry("LineStyle", (int) m_curve->linePen().style()) );
	ui.kcbLineColor->setColor( group.readEntry("LineColor", m_curve->linePen().color()) );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LineWidth", m_curve->linePen().widthF()), Worksheet::Unit::Point) );
	ui.sbLineOpacity->setValue( round(group.readEntry("LineOpacity", m_curve->lineOpacity())*100.0) );

	//Symbols
	symbolWidget->loadConfig(group);

  	//Values
  	ui.cbValuesType->setCurrentIndex( group.readEntry("ValuesType", (int) m_curve->valuesType()) );
  	ui.cbValuesPosition->setCurrentIndex( group.readEntry("ValuesPosition", (int) m_curve->valuesPosition()) );
	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ValuesDistance", m_curve->valuesDistance()), Worksheet::Unit::Point) );
	ui.sbValuesRotation->setValue( group.readEntry("ValuesRotation", m_curve->valuesRotationAngle()) );
	ui.sbValuesOpacity->setValue( round(group.readEntry("ValuesOpacity",m_curve->valuesOpacity())*100.0) );
	this->updateValuesWidgets();
  	ui.leValuesPrefix->setText( group.readEntry("ValuesPrefix", m_curve->valuesPrefix()) );
  	ui.leValuesSuffix->setText( group.readEntry("ValuesSuffix", m_curve->valuesSuffix()) );
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF( round(Worksheet::convertFromSceneUnits(valuesFont.pixelSize(), Worksheet::Unit::Point)) );
  	ui.kfrValuesFont->setFont( group.readEntry("ValuesFont", valuesFont) );
  	ui.kcbValuesColor->setColor( group.readEntry("ValuesColor", m_curve->valuesColor()) );

	//Filling
	ui.chkFillingEnabled->setChecked( group.readEntry("FillingEnabled", m_curve->fillingEnabled()) );
	ui.cbFillingType->setCurrentIndex( group.readEntry("FillingType", (int) m_curve->fillingType()) );
	ui.cbFillingColorStyle->setCurrentIndex( group.readEntry("FillingColorStyle", (int) m_curve->fillingColorStyle()) );
	ui.cbFillingImageStyle->setCurrentIndex( group.readEntry("FillingImageStyle", (int) m_curve->fillingImageStyle()) );
	ui.cbFillingBrushStyle->setCurrentIndex( group.readEntry("FillingBrushStyle", (int) m_curve->fillingBrushStyle()) );
	ui.leFillingFileName->setText( group.readEntry("FillingFileName", m_curve->fillingFileName()) );
	ui.kcbFillingFirstColor->setColor( group.readEntry("FillingFirstColor", m_curve->fillingFirstColor()) );
	ui.kcbFillingSecondColor->setColor( group.readEntry("FillingSecondColor", m_curve->fillingSecondColor()) );
	ui.sbFillingOpacity->setValue( round(group.readEntry("FillingOpacity", m_curve->fillingOpacity())*100.0) );
	fillingEnabledChanged(ui.chkFillingEnabled->isChecked()); //update the box filling widgets

	//Error bars
	ui.cbErrorType->setCurrentIndex( group.readEntry("ErrorType", (int) m_curve->errorType()) );
	ui.cbErrorBarsType->setCurrentIndex( group.readEntry("ErrorBarsType", (int) m_curve->errorBarsType()) );
	ui.sbErrorBarsCapSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ErrorBarsCapSize", m_curve->errorBarsCapSize()), Worksheet::Unit::Point) );
	ui.cbErrorBarsStyle->setCurrentIndex( group.readEntry("ErrorBarsStyle", (int) m_curve->errorBarsPen().style()) );
	ui.kcbErrorBarsColor->setColor( group.readEntry("ErrorBarsColor", m_curve->errorBarsPen().color()) );
	ui.sbErrorBarsWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ErrorBarsWidth", m_curve->errorBarsPen().widthF()),Worksheet::Unit::Point) );
	ui.sbErrorBarsOpacity->setValue( round(group.readEntry("ErrorBarsOpacity", m_curve->errorBarsOpacity())*100.0) );
}

void HistogramDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_curvesList.size();
	if (size > 1)
		m_curve->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_curve->beginMacro(i18n("%1: template \"%2\" loaded", m_curve->name(), name));

	this->loadConfig(config);

	m_curve->endMacro();
}

void HistogramDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("Histogram");

	//Line
	group.writeEntry("LineType", ui.cbLineType->currentIndex());
	group.writeEntry("LineStyle", ui.cbLineStyle->currentIndex());
	group.writeEntry("LineColor", ui.kcbLineColor->color());
	group.writeEntry("LineWidth", Worksheet::convertToSceneUnits(ui.sbLineWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("LineOpacity", ui.sbLineOpacity->value()/100.0);

	//Symbols
	symbolWidget->saveConfig(group);

	//Values
	group.writeEntry("ValuesType", ui.cbValuesType->currentIndex());
	group.writeEntry("ValuesPosition", ui.cbValuesPosition->currentIndex());
	group.writeEntry("ValuesDistance", Worksheet::convertToSceneUnits(ui.sbValuesDistance->value(), Worksheet::Unit::Point));
	group.writeEntry("ValuesRotation", ui.sbValuesRotation->value());
	group.writeEntry("ValuesOpacity", ui.sbValuesOpacity->value()/100.0);
	group.writeEntry("ValuesPrefix", ui.leValuesPrefix->text());
	group.writeEntry("ValuesSuffix", ui.leValuesSuffix->text());
	group.writeEntry("ValuesFont", ui.kfrValuesFont->font());
	group.writeEntry("ValuesColor", ui.kcbValuesColor->color());

	//Filling
	group.writeEntry("FillingEnabled", ui.chkFillingEnabled->isChecked());
	group.writeEntry("FillingType", ui.cbFillingType->currentIndex());
	group.writeEntry("FillingColorStyle", ui.cbFillingColorStyle->currentIndex());
	group.writeEntry("FillingImageStyle", ui.cbFillingImageStyle->currentIndex());
	group.writeEntry("FillingBrushStyle", ui.cbFillingBrushStyle->currentIndex());
	group.writeEntry("FillingFileName", ui.leFillingFileName->text());
	group.writeEntry("FillingFirstColor", ui.kcbFillingFirstColor->color());
	group.writeEntry("FillingSecondColor", ui.kcbFillingSecondColor->color());
	group.writeEntry("FillingOpacity", ui.sbFillingOpacity->value()/100.0);

	config.sync();
}
