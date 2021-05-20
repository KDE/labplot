/***************************************************************************
    File                 : XYCurveDock.cpp
    Project              : LabPlot
    Description          : widget for XYCurve properties
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2021 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2021 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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

#include "XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/Worksheet.h"
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
  \class XYCurveDock
  \brief  Provides a widget for editing the properties of the XYCurves (2D-curves) currently selected in the project explorer.

  If more than one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYCurveDock::XYCurveDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);

	//"Symbol"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabSymbol);
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	hboxLayout->addWidget(symbolWidget);
	hboxLayout->setContentsMargins(2,2,2,2);
	hboxLayout->setSpacing(2);

	//Tab "Values"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabValues->layout());
	cbValuesColumn = new TreeViewComboBox(ui.tabValues);
	gridLayout->addWidget(cbValuesColumn, 2, 2, 1, 1);

	//add formats for numeric values
	ui.cbValuesNumericFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbValuesNumericFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbValuesNumericFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbValuesNumericFormat->addItem(i18n("Automatic (e)"), QVariant('g'));
	ui.cbValuesNumericFormat->addItem(i18n("Automatic (E)"), QVariant('G'));

	//add format for date, time and datetime values
	for (const auto& s : AbstractColumn::dateTimeFormats())
		ui.cbValuesDateTimeFormat->addItem(s, QVariant(s));

	ui.cbValuesDateTimeFormat->setEditable(true);

	//Tab "Filling"
	ui.cbFillingColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.bFillingOpen->setIcon( QIcon::fromTheme("document-open") );

	ui.leFillingFileName->setCompleter(new QCompleter(new QDirModel, this));

	//Tab "Error bars"
	gridLayout = qobject_cast<QGridLayout*>(ui.tabErrorBars->layout());

	cbXErrorPlusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbXErrorPlusColumn, 2, 2, 1, 1);

	cbXErrorMinusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbXErrorMinusColumn, 3, 2, 1, 1);

	cbYErrorPlusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbYErrorPlusColumn, 7, 2, 1, 1);

	cbYErrorMinusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbYErrorMinusColumn, 8, 2, 1, 1);

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	XYCurveDock::updateLocale();

	//Slots

	//Lines
	connect( ui.cbLineType, SIGNAL(currentIndexChanged(int)), this, SLOT(lineTypeChanged(int)) );
	connect( ui.sbLineInterpolationPointsCount, SIGNAL(valueChanged(int)), this, SLOT(lineInterpolationPointsCountChanged(int)) );
	connect( ui.chkLineSkipGaps, SIGNAL(clicked(bool)), this, SLOT(lineSkipGapsChanged(bool)) );
	connect( ui.chkLineIncreasingXOnly, &QCheckBox::clicked, this, &XYCurveDock::lineIncreasingXOnlyChanged );
	connect( ui.cbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(lineStyleChanged(int)) );
	connect( ui.kcbLineColor, SIGNAL(changed(QColor)), this, SLOT(lineColorChanged(QColor)) );
	connect( ui.sbLineWidth, SIGNAL(valueChanged(double)), this, SLOT(lineWidthChanged(double)) );
	connect( ui.sbLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(lineOpacityChanged(int)) );

	connect( ui.cbDropLineType, SIGNAL(currentIndexChanged(int)), this, SLOT(dropLineTypeChanged(int)) );
	connect( ui.cbDropLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(dropLineStyleChanged(int)) );
	connect( ui.kcbDropLineColor, SIGNAL(changed(QColor)), this, SLOT(dropLineColorChanged(QColor)) );
	connect( ui.sbDropLineWidth, SIGNAL(valueChanged(double)), this, SLOT(dropLineWidthChanged(double)) );
	connect( ui.sbDropLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(dropLineOpacityChanged(int)) );

	//Values
	connect( ui.cbValuesType, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesTypeChanged(int)) );
	connect( cbValuesColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(valuesColumnChanged(QModelIndex)) );
	connect( ui.cbValuesPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesPositionChanged(int)) );
	connect( ui.sbValuesDistance, SIGNAL(valueChanged(double)), this, SLOT(valuesDistanceChanged(double)) );
	connect( ui.sbValuesRotation, SIGNAL(valueChanged(int)), this, SLOT(valuesRotationChanged(int)) );
	connect( ui.sbValuesOpacity, SIGNAL(valueChanged(int)), this, SLOT(valuesOpacityChanged(int)) );
	connect(ui.cbValuesNumericFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::valuesNumericFormatChanged);
	connect(ui.sbValuesPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYCurveDock::valuesPrecisionChanged);
	connect(ui.cbValuesDateTimeFormat, &QComboBox::currentTextChanged, this, &XYCurveDock::valuesDateTimeFormatChanged);
	connect(ui.leValuesPrefix, &QLineEdit::textChanged, this, &XYCurveDock::valuesPrefixChanged);
	connect(ui.leValuesSuffix, &QLineEdit::textChanged, this, &XYCurveDock::valuesSuffixChanged);
	connect( ui.kfrValuesFont, SIGNAL(fontSelected(QFont)), this, SLOT(valuesFontChanged(QFont)) );
	connect( ui.kcbValuesColor, SIGNAL(changed(QColor)), this, SLOT(valuesColorChanged(QColor)) );

	//Filling
	connect( ui.cbFillingPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingPositionChanged(int)) );
	connect( ui.cbFillingType, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingTypeChanged(int)) );
	connect( ui.cbFillingColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingColorStyleChanged(int)) );
	connect( ui.cbFillingImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingImageStyleChanged(int)) );
	connect( ui.cbFillingBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingBrushStyleChanged(int)) );
	connect(ui.bFillingOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect(ui.leFillingFileName, &QLineEdit::textChanged, this, &XYCurveDock::fileNameChanged);
	connect( ui.leFillingFileName, SIGNAL(textChanged(QString)), this, SLOT(fileNameChanged()) );
	connect( ui.kcbFillingFirstColor, SIGNAL(changed(QColor)), this, SLOT(fillingFirstColorChanged(QColor)) );
	connect( ui.kcbFillingSecondColor, SIGNAL(changed(QColor)), this, SLOT(fillingSecondColorChanged(QColor)) );
	connect( ui.sbFillingOpacity, SIGNAL(valueChanged(int)), this, SLOT(fillingOpacityChanged(int)) );

	//Error bars
	connect( ui.cbXErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(xErrorTypeChanged(int)) );
	connect( cbXErrorPlusColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xErrorPlusColumnChanged(QModelIndex)) );
	connect( cbXErrorMinusColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xErrorMinusColumnChanged(QModelIndex)) );
	connect( ui.cbYErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(yErrorTypeChanged(int)) );
	connect( cbYErrorPlusColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yErrorPlusColumnChanged(QModelIndex)) );
	connect( cbYErrorMinusColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yErrorMinusColumnChanged(QModelIndex)) );
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

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::XYCurve);
	layout->addWidget(templateHandler);
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));

	ui.verticalLayout->addWidget(frame);

	retranslateUi();
	init();
}

XYCurveDock::~XYCurveDock() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;
}

void XYCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_teComment = uiGeneralTab.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	// Tab "General" (see xycurvedockgeneraltab.ui)
	auto* gridLayout = qobject_cast<QGridLayout*>(generalTab->layout());

	cbXColumn = new TreeViewComboBox(generalTab);
	cbXColumn->useCurrentIndexText(false);
	gridLayout->addWidget(cbXColumn, 4, 2, 1, 1);

	cbYColumn = new TreeViewComboBox(generalTab);
	cbYColumn->useCurrentIndexText(false);
	gridLayout->addWidget(cbYColumn, 5, 2, 1, 1);

	//General
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYCurveDock::nameChanged);
	connect(uiGeneralTab.teComment, &QTextEdit::textChanged, this, &XYCurveDock::commentChanged);
	connect(uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)));
	connect(cbXColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xColumnChanged(QModelIndex)));
	connect(cbYColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yColumnChanged(QModelIndex)));
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::plotRangeChanged);
}

void XYCurveDock::init() {
	m_initializing = true;

	//Line
	ui.cbLineType->addItem(i18n("None"));
	ui.cbLineType->addItem(i18n("Line"));
	ui.cbLineType->addItem(i18n("Horiz. Start"));
	ui.cbLineType->addItem(i18n("Vert. Start"));
	ui.cbLineType->addItem(i18n("Horiz. Midpoint"));
	ui.cbLineType->addItem(i18n("Vert. Midpoint"));
	ui.cbLineType->addItem(i18n("2-segments"));
	ui.cbLineType->addItem(i18n("3-segments"));
	ui.cbLineType->addItem(i18n("Cubic Spline (Natural)"));
	ui.cbLineType->addItem(i18n("Cubic Spline (Periodic)"));
	ui.cbLineType->addItem(i18n("Akima-spline (Natural)"));
	ui.cbLineType->addItem(i18n("Akima-spline (Periodic)"));

	QPainter pa;
	//TODO size of the icon depending on the actual height of the combobox?
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	ui.cbLineType->setIconSize(QSize(iconSize, iconSize));

	QPen pen(Qt::SolidPattern, 0);
	const QColor& color = (palette().color(QPalette::Base).lightness() < 128) ? Qt::white : Qt::black;
	pen.setColor(color);
	pa.setPen( pen );

	//no line
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.end();
	ui.cbLineType->setItemIcon(0, pm);

	//line
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(1, pm);

	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,17,3);
	pa.drawLine(17,3,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(2, pm);

	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,3,17);
	pa.drawLine(3,17,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(3, pm);

	//horizontal midpoint
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,10,3);
	pa.drawLine(10,3,10,17);
	pa.drawLine(10,17,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(4, pm);

	//vertical midpoint
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,3,10);
	pa.drawLine(3,10,17,10);
	pa.drawLine(17,10,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(5, pm);

	//2-segments
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 8,8,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,10,10);
	pa.end();
	ui.cbLineType->setItemIcon(6, pm);

	//3-segments
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 8,8,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(7, pm);

	//natural spline
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.rotate(45);
	pa.drawArc(2*sqrt(2),-4,17*sqrt(2),20,30*16,120*16);

	pa.end();
	ui.cbLineType->setItemIcon(8, pm);
	ui.cbLineType->setItemIcon(9, pm);
	ui.cbLineType->setItemIcon(10, pm);
	ui.cbLineType->setItemIcon(11, pm);

	GuiTools::updatePenStyles(ui.cbLineStyle, Qt::black);

	//Drop lines
	ui.cbDropLineType->addItem(i18n("No Drop Lines"));
	ui.cbDropLineType->addItem(i18n("Drop Lines, X"));
	ui.cbDropLineType->addItem(i18n("Drop Lines, Y"));
	ui.cbDropLineType->addItem(i18n("Drop Lines, XY"));
	ui.cbDropLineType->addItem(i18n("Drop Lines, X, Zero Baseline"));
	ui.cbDropLineType->addItem(i18n("Drop Lines, X, Min Baseline"));
	ui.cbDropLineType->addItem(i18n("Drop Lines, X, Max Baseline"));
	GuiTools::updatePenStyles(ui.cbDropLineStyle, Qt::black);

	m_initializing = false;

	//Values
	ui.cbValuesType->addItem(i18n("No Values"));
	ui.cbValuesType->addItem("x");
	ui.cbValuesType->addItem("y");
	ui.cbValuesType->addItem("x, y");
	ui.cbValuesType->addItem("(x, y)");
	ui.cbValuesType->addItem(i18n("Custom Column"));

	ui.cbValuesPosition->addItem(i18n("Above"));
	ui.cbValuesPosition->addItem(i18n("Below"));
	ui.cbValuesPosition->addItem(i18n("Left"));
	ui.cbValuesPosition->addItem(i18n("Right"));

	//Filling
	ui.cbFillingPosition->clear();
	ui.cbFillingPosition->addItem(i18n("None"));
	ui.cbFillingPosition->addItem(i18n("Above"));
	ui.cbFillingPosition->addItem(i18n("Below"));
	ui.cbFillingPosition->addItem(i18n("Zero Baseline"));
	ui.cbFillingPosition->addItem(i18n("Left"));
	ui.cbFillingPosition->addItem(i18n("Right"));

	ui.cbFillingType->clear();
	ui.cbFillingType->addItem(i18n("Color"));
	ui.cbFillingType->addItem(i18n("Image"));
	ui.cbFillingType->addItem(i18n("Pattern"));

	ui.cbFillingColorStyle->clear();
	ui.cbFillingColorStyle->addItem(i18n("Single Color"));
	ui.cbFillingColorStyle->addItem(i18n("Horizontal Gradient"));
	ui.cbFillingColorStyle->addItem(i18n("Vertical Gradient"));
	ui.cbFillingColorStyle->addItem(i18n("Diag. Gradient (From Top Left)"));
	ui.cbFillingColorStyle->addItem(i18n("Diag. Gradient (From Bottom Left)"));
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

	ui.cbXErrorType->addItem(i18n("No"));
	ui.cbXErrorType->addItem(i18n("Symmetric"));
	ui.cbXErrorType->addItem(i18n("Asymmetric"));

	ui.cbYErrorType->addItem(i18n("No"));
	ui.cbYErrorType->addItem(i18n("Symmetric"));
	ui.cbYErrorType->addItem(i18n("Asymmetric"));

	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, Qt::black);
}

void XYCurveDock::setModel() {
	m_aspectTreeModel->enablePlottableColumnsOnly(true);
	m_aspectTreeModel->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Datapicker,
	                       AspectType::DatapickerCurve, AspectType::Spreadsheet, AspectType::LiveDataSource,
	                       AspectType::Column, AspectType::Worksheet, AspectType::CartesianPlot,
	                       AspectType::XYFitCurve, AspectType::XYSmoothCurve, AspectType::CantorWorksheet};

	if (cbXColumn) {
		cbXColumn->setTopLevelClasses(list);
		cbYColumn->setTopLevelClasses(list);
	}
	cbValuesColumn->setTopLevelClasses(list);
	cbXErrorMinusColumn->setTopLevelClasses(list);
	cbXErrorPlusColumn->setTopLevelClasses(list);
	cbYErrorMinusColumn->setTopLevelClasses(list);
	cbYErrorPlusColumn->setTopLevelClasses(list);

	if (m_curve->inherits(AspectType::XYAnalysisCurve))
		//the model is use in the combobox for curve data sources -> allow to also select analysis curves
		list = {AspectType::Column, AspectType::XYCurve,
			AspectType::XYFitCurve, AspectType::XYIntegrationCurve, AspectType::XYInterpolationCurve,
			AspectType::XYSmoothCurve, AspectType::XYFourierFilterCurve, AspectType::XYFourierTransformCurve,
			AspectType::XYConvolutionCurve, AspectType::XYCorrelationCurve, AspectType::XYDataReductionCurve};
	else
		list = {AspectType::Column, AspectType::XYCurve};

	m_aspectTreeModel->setSelectableAspects(list);

	if (cbXColumn) {
		cbXColumn->setModel(m_aspectTreeModel);
		cbYColumn->setModel(m_aspectTreeModel);
	}
	cbValuesColumn->setModel(m_aspectTreeModel);
	cbXErrorMinusColumn->setModel(m_aspectTreeModel);
	cbXErrorPlusColumn->setModel(m_aspectTreeModel);
	cbYErrorMinusColumn->setModel(m_aspectTreeModel);
	cbYErrorPlusColumn->setModel(m_aspectTreeModel);

	//this function is called after the dock widget is initializes and the curves are set.
	//so, we use this function to finalize the initialization even though it's not related
	//to the actual set of the model (could also be solved by a new class XYAnalysisiCurveDock).

	//hide property widgets that are not relevant for analysis curves
	bool visible = (m_curve->type() == AspectType::XYCurve);
	ui.lLineType->setVisible(visible);
	ui.cbLineType->setVisible(visible);
	ui.lLineSkipGaps->setVisible(visible);
	ui.chkLineSkipGaps->setVisible(visible);
	ui.lLineIncreasingXOnly->setVisible(visible);
	ui.chkLineIncreasingXOnly->setVisible(visible);

	//remove the tab "Error bars" for analysis curves
	if (!visible)
		ui.tabWidget->removeTab(5);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYCurveDock::setCurves(QList<XYCurve*> list) {
	const Lock lock(m_initializing);
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = m_curve;
	Q_ASSERT(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	setModel();
	initGeneralTab();
	initTabs();

	QList<Symbol*> symbols;
	for (auto* curve : m_curvesList)
		symbols << curve->symbol();

	symbolWidget->setSymbols(symbols);
}

void XYCurveDock::initGeneralTab() {
	DEBUG("XYCurveDock::initGeneralTab()");
	//if there is more than one curve in the list, disable the content in the tab "general"
	if (m_curvesList.size() == 1) {
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.teComment->setEnabled(true);
		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.teComment->setText(m_curve->comment());
	} else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.teComment->setEnabled(false);
		uiGeneralTab.leName->setText(QString());
		uiGeneralTab.teComment->setText(QString());
	}

	//show the properties of the first curve
	cbXColumn->setColumn(m_curve->xColumn(), m_curve->xColumnPath());
	cbYColumn->setColumn(m_curve->yColumn(), m_curve->yColumnPath());
	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	updatePlotRanges();

	//Slots
	connect(m_curve, &XYCurve::aspectDescriptionChanged, this, &XYCurveDock::curveDescriptionChanged);
	connect(m_curve, &XYCurve::xColumnChanged, this, &XYCurveDock::curveXColumnChanged);
	connect(m_curve, &XYCurve::yColumnChanged, this, &XYCurveDock::curveYColumnChanged);
	connect(m_curve, &WorksheetElement::plotRangeListChanged, this, &XYCurveDock::updatePlotRanges);
	connect(m_curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &XYCurveDock::curveVisibilityChanged);
	DEBUG("XYCurveDock::initGeneralTab() DONE");
}

void XYCurveDock::initTabs() {
	//if there are more than one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1) {
		cbValuesColumn->setColumn(m_curve->valuesColumn(), m_curve->valuesColumnPath());
		cbXErrorPlusColumn->setColumn(m_curve->xErrorPlusColumn(), m_curve->xErrorPlusColumnPath());
		cbXErrorMinusColumn->setColumn(m_curve->xErrorMinusColumn(), m_curve->xErrorMinusColumnPath());
		cbYErrorPlusColumn->setColumn(m_curve->yErrorPlusColumn(), m_curve->yErrorPlusColumnPath());
		cbYErrorMinusColumn->setColumn(m_curve->yErrorMinusColumn(), m_curve->yErrorMinusColumnPath());
	} else {
		cbValuesColumn->setCurrentModelIndex(QModelIndex());
		cbXErrorPlusColumn->setCurrentModelIndex(QModelIndex());
		cbXErrorMinusColumn->setCurrentModelIndex(QModelIndex());
		cbYErrorPlusColumn->setCurrentModelIndex(QModelIndex());
		cbYErrorMinusColumn->setCurrentModelIndex(QModelIndex());
	}

	//show the properties of the first curve
	load();

	//Slots

	//Line-Tab
	connect(m_curve, SIGNAL(lineTypeChanged(XYCurve::LineType)), this, SLOT(curveLineTypeChanged(XYCurve::LineType)));
	connect(m_curve, SIGNAL(lineSkipGapsChanged(bool)), this, SLOT(curveLineSkipGapsChanged(bool)));
	connect(m_curve, &XYCurve::lineIncreasingXOnlyChanged, this, &XYCurveDock::curveLineIncreasingXOnlyChanged);
	connect(m_curve, SIGNAL(lineInterpolationPointsCountChanged(int)), this, SLOT(curveLineInterpolationPointsCountChanged(int)));
	connect(m_curve, SIGNAL(linePenChanged(QPen)), this, SLOT(curveLinePenChanged(QPen)));
	connect(m_curve, SIGNAL(lineOpacityChanged(qreal)), this, SLOT(curveLineOpacityChanged(qreal)));
	connect(m_curve, SIGNAL(dropLineTypeChanged(XYCurve::DropLineType)), this, SLOT(curveDropLineTypeChanged(XYCurve::DropLineType)));
	connect(m_curve, SIGNAL(dropLinePenChanged(QPen)), this, SLOT(curveDropLinePenChanged(QPen)));
	connect(m_curve, SIGNAL(dropLineOpacityChanged(qreal)), this, SLOT(curveDropLineOpacityChanged(qreal)));

	//Values-Tab
	connect(m_curve, SIGNAL(valuesTypeChanged(XYCurve::ValuesType)), this, SLOT(curveValuesTypeChanged(XYCurve::ValuesType)));
	connect(m_curve, SIGNAL(valuesColumnChanged(const AbstractColumn*)), this, SLOT(curveValuesColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(valuesPositionChanged(XYCurve::ValuesPosition)), this, SLOT(curveValuesPositionChanged(XYCurve::ValuesPosition)));
	connect(m_curve, SIGNAL(valuesDistanceChanged(qreal)), this, SLOT(curveValuesDistanceChanged(qreal)));
	connect(m_curve, SIGNAL(valuesOpacityChanged(qreal)), this, SLOT(curveValuesOpacityChanged(qreal)));
	connect(m_curve, SIGNAL(valuesRotationAngleChanged(qreal)), this, SLOT(curveValuesRotationAngleChanged(qreal)));
	connect(m_curve, &XYCurve::valuesNumericFormatChanged, this, &XYCurveDock::curveValuesNumericFormatChanged);
	connect(m_curve, &XYCurve::valuesPrecisionChanged, this, &XYCurveDock::curveValuesPrecisionChanged);
	connect(m_curve, &XYCurve::valuesDateTimeFormatChanged, this, &XYCurveDock::curveValuesDateTimeFormatChanged);
	connect(m_curve, SIGNAL(valuesPrefixChanged(QString)), this, SLOT(curveValuesPrefixChanged(QString)));
	connect(m_curve, SIGNAL(valuesSuffixChanged(QString)), this, SLOT(curveValuesSuffixChanged(QString)));
	connect(m_curve, SIGNAL(valuesFontChanged(QFont)), this, SLOT(curveValuesFontChanged(QFont)));
	connect(m_curve, SIGNAL(valuesColorChanged(QColor)), this, SLOT(curveValuesColorChanged(QColor)));

	//Filling-Tab
	connect( m_curve, SIGNAL(fillingPositionChanged(XYCurve::FillingPosition)), this, SLOT(curveFillingPositionChanged(XYCurve::FillingPosition)) );
	connect( m_curve, SIGNAL(fillingTypeChanged(PlotArea::BackgroundType)), this, SLOT(curveFillingTypeChanged(PlotArea::BackgroundType)) );
	connect( m_curve, SIGNAL(fillingColorStyleChanged(PlotArea::BackgroundColorStyle)), this, SLOT(curveFillingColorStyleChanged(PlotArea::BackgroundColorStyle)) );
	connect( m_curve, SIGNAL(fillingImageStyleChanged(PlotArea::BackgroundImageStyle)), this, SLOT(curveFillingImageStyleChanged(PlotArea::BackgroundImageStyle)) );
	connect( m_curve, SIGNAL(fillingBrushStyleChanged(Qt::BrushStyle)), this, SLOT(curveFillingBrushStyleChanged(Qt::BrushStyle)) );
	connect( m_curve, SIGNAL(fillingFirstColorChanged(QColor&)), this, SLOT(curveFillingFirstColorChanged(QColor&)) );
	connect( m_curve, SIGNAL(fillingSecondColorChanged(QColor&)), this, SLOT(curveFillingSecondColorChanged(QColor&)) );
	connect( m_curve, SIGNAL(fillingFileNameChanged(QString&)), this, SLOT(curveFillingFileNameChanged(QString&)) );
	connect( m_curve, SIGNAL(fillingOpacityChanged(float)), this, SLOT(curveFillingOpacityChanged(float)) );

	//"Error bars"-Tab
	connect(m_curve, SIGNAL(xErrorTypeChanged(XYCurve::ErrorType)), this, SLOT(curveXErrorTypeChanged(XYCurve::ErrorType)));
	connect(m_curve, SIGNAL(xErrorPlusColumnChanged(const AbstractColumn*)), this, SLOT(curveXErrorPlusColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(xErrorMinusColumnChanged(const AbstractColumn*)), this, SLOT(curveXErrorMinusColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(yErrorTypeChanged(XYCurve::ErrorType)), this, SLOT(curveYErrorTypeChanged(XYCurve::ErrorType)));
	connect(m_curve, SIGNAL(yErrorPlusColumnChanged(const AbstractColumn*)), this, SLOT(curveYErrorPlusColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(yErrorMinusColumnChanged(const AbstractColumn*)), this, SLOT(curveYErrorMinusColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(errorBarsCapSizeChanged(qreal)), this, SLOT(curveErrorBarsCapSizeChanged(qreal)));
	connect(m_curve, SIGNAL(errorBarsTypeChanged(XYCurve::ErrorBarsType)), this, SLOT(curveErrorBarsTypeChanged(XYCurve::ErrorBarsType)));
	connect(m_curve, SIGNAL(errorBarsPenChanged(QPen)), this, SLOT(curveErrorBarsPenChanged(QPen)));
	connect(m_curve, SIGNAL(errorBarsOpacityChanged(qreal)), this, SLOT(curveErrorBarsOpacityChanged(qreal)));
}

void XYCurveDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbLineWidth->setLocale(numberLocale);
	ui.sbDropLineWidth->setLocale(numberLocale);
	ui.sbValuesDistance->setLocale(numberLocale);
	ui.sbErrorBarsCapSize->setLocale(numberLocale);
	ui.sbErrorBarsWidth->setLocale(numberLocale);
	symbolWidget->updateLocale();
}

void XYCurveDock::updatePlotRanges() const {
	updatePlotRangeList(uiGeneralTab.cbPlotRanges);
}

//*************************************************************
//********** SLOTs for changes triggered in XYCurveDock ********
//*************************************************************
void XYCurveDock::retranslateUi() {
	ui.lLineSkipGaps->setToolTip(i18n("If checked, connect neighbour points with lines even if there are gaps (invalid or masked values) between them"));
	ui.chkLineSkipGaps->setToolTip(i18n("If checked, connect neighbour points with lines even if there are gaps (invalid or masked values) between them"));
	ui.lLineIncreasingXOnly->setToolTip(i18n("If checked, connect data points only for strictly increasing values of X"));
	ui.chkLineIncreasingXOnly->setToolTip(i18n("If checked, connect data points only for strictly increasing values of X"));
	//TODO:
// 	uiGeneralTab.lName->setText(i18n("Name"));
// 	uiGeneralTab.lComment->setText(i18n("Comment"));
// 	uiGeneralTab.chkVisible->setText(i18n("Visible"));
// 	uiGeneralTab.lXColumn->setText(i18n("x-data"));
// 	uiGeneralTab.lYColumn->setText(i18n("y-data"));

	//TODO updatePenStyles, updateBrushStyles for all comboboxes
}

void XYCurveDock::xColumnChanged(const QModelIndex& index) {
	updateValuesWidgets();

	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_curvesList)
		curve->setXColumn(column);
}

void XYCurveDock::yColumnChanged(const QModelIndex& index) {
	updateValuesWidgets();

	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_curvesList)
		curve->setYColumn(column);
}

void XYCurveDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setVisible(state);
}

// "Line"-tab
void XYCurveDock::lineTypeChanged(int index) {
	const auto lineType = XYCurve::LineType(index);

	if ( lineType == XYCurve::LineType::NoLine) {
		ui.chkLineSkipGaps->setEnabled(false);
		ui.cbLineStyle->setEnabled(false);
		ui.kcbLineColor->setEnabled(false);
		ui.sbLineWidth->setEnabled(false);
		ui.sbLineOpacity->setEnabled(false);
		ui.lLineInterpolationPointsCount->hide();
		ui.sbLineInterpolationPointsCount->hide();
	} else {
		ui.chkLineSkipGaps->setEnabled(true);
		ui.cbLineStyle->setEnabled(true);
		ui.kcbLineColor->setEnabled(true);
		ui.sbLineWidth->setEnabled(true);
		ui.sbLineOpacity->setEnabled(true);

		if (lineType == XYCurve::LineType::SplineCubicNatural || lineType == XYCurve::LineType::SplineCubicPeriodic
		        || lineType == XYCurve::LineType::SplineAkimaNatural || lineType == XYCurve::LineType::SplineAkimaPeriodic) {
			ui.lLineInterpolationPointsCount->show();
			ui.sbLineInterpolationPointsCount->show();
			ui.lLineSkipGaps->hide();
			ui.chkLineSkipGaps->hide();
		} else {
			ui.lLineInterpolationPointsCount->hide();
			ui.sbLineInterpolationPointsCount->hide();
			ui.lLineSkipGaps->show();
			ui.chkLineSkipGaps->show();
		}
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineType(lineType);
}

void XYCurveDock::lineSkipGapsChanged(bool skip) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineSkipGaps(skip);
}

void XYCurveDock::lineIncreasingXOnlyChanged(bool incr) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineIncreasingXOnly(incr);
}

void XYCurveDock::lineInterpolationPointsCountChanged(int count) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineInterpolationPointsCount(count);
}

void XYCurveDock::lineStyleChanged(int index) {
	if (index == -1 || m_initializing)
		return;

	const auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->linePen();
		pen.setStyle(penStyle);
		curve->setLinePen(pen);
	}
}

void XYCurveDock::lineColorChanged(const QColor& color) {
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

void XYCurveDock::lineWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->linePen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		curve->setLinePen(pen);
	}
}

void XYCurveDock::lineOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curvesList)
		curve->setLineOpacity(opacity);
}

void XYCurveDock::dropLineTypeChanged(int index) {
	const auto dropLineType = XYCurve::DropLineType(index);

	if ( dropLineType == XYCurve::DropLineType::NoDropLine) {
		ui.cbDropLineStyle->setEnabled(false);
		ui.kcbDropLineColor->setEnabled(false);
		ui.sbDropLineWidth->setEnabled(false);
		ui.sbDropLineOpacity->setEnabled(false);
	} else {
		ui.cbDropLineStyle->setEnabled(true);
		ui.kcbDropLineColor->setEnabled(true);
		ui.sbDropLineWidth->setEnabled(true);
		ui.sbDropLineOpacity->setEnabled(true);
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setDropLineType(dropLineType);
}

void XYCurveDock::dropLineStyleChanged(int index) {
	if (index == -1 || m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->dropLinePen();
		pen.setStyle(penStyle);
		curve->setDropLinePen(pen);
	}
}

void XYCurveDock::dropLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->dropLinePen();
		pen.setColor(color);
		curve->setDropLinePen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbDropLineStyle, color);
	m_initializing = false;
}

void XYCurveDock::dropLineWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->dropLinePen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		curve->setDropLinePen(pen);
	}
}

void XYCurveDock::dropLineOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curvesList)
		curve->setDropLineOpacity(opacity);
}

//Values-tab

/*!
  called when the type of the values (none, x, y, (x,y) etc.) was changed.
*/
void XYCurveDock::valuesTypeChanged(int index) {
	if (m_initializing)
		return;

	this->updateValuesWidgets();

	const auto type = XYCurve::ValuesType(index);
	for (auto* curve : m_curvesList)
		curve->setValuesType(type);
}

/*!
  called when the custom column for the values was changed.
*/
void XYCurveDock::valuesColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	this->updateValuesWidgets();

	auto* column = static_cast<Column*>(index.internalPointer());
	for (auto* curve : m_curvesList)
		curve->setValuesColumn(column);
}

/*!
  shows the formatting properties of the column \c column.
  Called, when a new column for the values was selected - either by changing the type of the values (none, x, y, etc.) or
  by selecting a new custom column for the values.
*/

/*!
  depending on the currently selected values column type (column mode) updates the widgets for the values column format,
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the values column was changed.
*/
void XYCurveDock::updateValuesWidgets() {
	const auto type{XYCurve::ValuesType(ui.cbValuesType->currentIndex())};
	bool showValues{type != XYCurve::ValuesType::NoValues};

	ui.cbValuesPosition->setEnabled(showValues);
	ui.sbValuesDistance->setEnabled(showValues);
	ui.sbValuesRotation->setEnabled(showValues);
	ui.sbValuesOpacity->setEnabled(showValues);
	ui.kfrValuesFont->setEnabled(showValues);
	ui.kcbValuesColor->setEnabled(showValues);

	bool hasInteger = false;
	bool hasNumeric = false;
	bool hasDateTime = false;

	if (type == XYCurve::ValuesType::CustomColumn) {
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


		const AbstractColumn* xColumn = nullptr;
		const AbstractColumn* yColumn = nullptr;
		switch (type) {
			case XYCurve::ValuesType::NoValues:
				break;
			case XYCurve::ValuesType::X:
				xColumn = m_curve->xColumn();
				break;
			case XYCurve::ValuesType::Y:
				yColumn = m_curve->yColumn();
				break;
			case XYCurve::ValuesType::XY:
			case XYCurve::ValuesType::XYBracketed:
				xColumn = m_curve->xColumn();
				yColumn = m_curve->yColumn();
				break;
			case XYCurve::ValuesType::CustomColumn:
				break;
		}

		hasInteger = (xColumn && (xColumn->columnMode() == AbstractColumn::ColumnMode::Integer || xColumn->columnMode() == AbstractColumn::ColumnMode::Integer))
						|| (yColumn && (yColumn->columnMode() == AbstractColumn::ColumnMode::Integer || yColumn->columnMode() == AbstractColumn::ColumnMode::Integer));

		hasNumeric = (xColumn && xColumn->columnMode() == AbstractColumn::ColumnMode::Numeric)
						|| (yColumn && yColumn->columnMode() == AbstractColumn::ColumnMode::Numeric);

		hasDateTime = (xColumn && xColumn->columnMode() == AbstractColumn::ColumnMode::DateTime)
						|| (yColumn && yColumn->columnMode() == AbstractColumn::ColumnMode::DateTime);
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

void XYCurveDock::valuesPositionChanged(int index) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesPosition(XYCurve::ValuesPosition(index));
}

void XYCurveDock::valuesDistanceChanged(double  value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesDistance( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void XYCurveDock::valuesRotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesRotationAngle(value);
}

void XYCurveDock::valuesOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curvesList)
		curve->setValuesOpacity(opacity);
}

void XYCurveDock::valuesNumericFormatChanged(int index) {
	if (m_initializing)
		return;

	char format = ui.cbValuesNumericFormat->itemData(index).toChar().toLatin1();
	for (auto* curve : m_curvesList)
		curve->setValuesNumericFormat(format);
}

void XYCurveDock::valuesDateTimeFormatChanged(const QString& format) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesDateTimeFormat(format);
}

void XYCurveDock::valuesPrecisionChanged(int precision) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesPrecision(precision);
}

void XYCurveDock::valuesPrefixChanged() {
	if (m_initializing)
		return;

	QString prefix = ui.leValuesPrefix->text();
	for (auto* curve : m_curvesList)
		curve->setValuesPrefix(prefix);
}

void XYCurveDock::valuesSuffixChanged() {
	if (m_initializing)
		return;

	QString suffix = ui.leValuesSuffix->text();
	for (auto* curve : m_curvesList)
		curve->setValuesSuffix(suffix);
}

void XYCurveDock::valuesFontChanged(const QFont& font) {
	if (m_initializing)
		return;

	QFont valuesFont = font;
	valuesFont.setPixelSize( Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point) );
	for (auto* curve : m_curvesList)
		curve->setValuesFont(valuesFont);
}

void XYCurveDock::valuesColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesColor(color);
}

//Filling-tab
void XYCurveDock::fillingPositionChanged(int index) {
	const auto fillingPosition{XYCurve::FillingPosition(index)};

	bool b = (fillingPosition != XYCurve::FillingPosition::NoFilling);
	ui.cbFillingType->setEnabled(b);
	ui.cbFillingColorStyle->setEnabled(b);
	ui.cbFillingBrushStyle->setEnabled(b);
	ui.cbFillingImageStyle->setEnabled(b);
	ui.kcbFillingFirstColor->setEnabled(b);
	ui.kcbFillingSecondColor->setEnabled(b);
	ui.leFillingFileName->setEnabled(b);
	ui.bFillingOpen->setEnabled(b);
	ui.sbFillingOpacity->setEnabled(b);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setFillingPosition(fillingPosition);
}

void XYCurveDock::fillingTypeChanged(int index) {
	const auto type = (PlotArea::BackgroundType)index;

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

void XYCurveDock::fillingColorStyleChanged(int index) {
	const auto style = (PlotArea::BackgroundColorStyle)index;

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

void XYCurveDock::fillingImageStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (PlotArea::BackgroundImageStyle)index;
	for (auto* curve : m_curvesList)
		curve->setFillingImageStyle(style);
}

void XYCurveDock::fillingBrushStyleChanged(int index) {
	if (index == -1 || m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* curve : m_curvesList)
		curve->setFillingBrushStyle(style);
}

void XYCurveDock::fillingFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setFillingFirstColor(c);

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, c);
	m_initializing = false;
}

void XYCurveDock::fillingSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setFillingSecondColor(c);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void XYCurveDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "XYCurveDock");
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

void XYCurveDock::fileNameChanged() {
	if (m_initializing)
		return;

	QString fileName = ui.leFillingFileName->text();
	for (auto* curve : m_curvesList)
		curve->setFillingFileName(fileName);
}

void XYCurveDock::fillingOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curvesList)
		curve->setFillingOpacity(opacity);
}

//"Error bars"-Tab
void XYCurveDock::xErrorTypeChanged(int index) const {
	if (index == 0) {
		//no error
		ui.lXErrorDataPlus->setVisible(false);
		cbXErrorPlusColumn->setVisible(false);
		ui.lXErrorDataMinus->setVisible(false);
		cbXErrorMinusColumn->setVisible(false);
	} else if (index == 1) {
		//symmetric error
		ui.lXErrorDataPlus->setVisible(true);
		cbXErrorPlusColumn->setVisible(true);
		ui.lXErrorDataMinus->setVisible(false);
		cbXErrorMinusColumn->setVisible(false);
		ui.lXErrorDataPlus->setText(i18n("Data, +-"));
	} else if (index == 2) {
		//asymmetric error
		ui.lXErrorDataPlus->setVisible(true);
		cbXErrorPlusColumn->setVisible(true);
		ui.lXErrorDataMinus->setVisible(true);
		cbXErrorMinusColumn->setVisible(true);
		ui.lXErrorDataPlus->setText(i18n("Data, +"));
	}

	bool b = (index!=0 || ui.cbYErrorType->currentIndex()!=0);
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
		curve->setXErrorType(XYCurve::ErrorType(index));
}

void XYCurveDock::xErrorPlusColumnChanged(const QModelIndex& index) const {
	Q_UNUSED(index);
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setXErrorPlusColumn(column);
}

void XYCurveDock::xErrorMinusColumnChanged(const QModelIndex& index) const {
	Q_UNUSED(index);
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setXErrorMinusColumn(column);
}

void XYCurveDock::yErrorTypeChanged(int index) const {
	if (index == 0) {
		//no error
		ui.lYErrorDataPlus->setVisible(false);
		cbYErrorPlusColumn->setVisible(false);
		ui.lYErrorDataMinus->setVisible(false);
		cbYErrorMinusColumn->setVisible(false);
	} else if (index == 1) {
		//symmetric error
		ui.lYErrorDataPlus->setVisible(true);
		cbYErrorPlusColumn->setVisible(true);
		ui.lYErrorDataMinus->setVisible(false);
		cbYErrorMinusColumn->setVisible(false);
		ui.lYErrorDataPlus->setText(i18n("Data, +-"));
	} else if (index == 2) {
		//asymmetric error
		ui.lYErrorDataPlus->setVisible(true);
		cbYErrorPlusColumn->setVisible(true);
		ui.lYErrorDataMinus->setVisible(true);
		cbYErrorMinusColumn->setVisible(true);
		ui.lYErrorDataPlus->setText(i18n("Data, +"));
	}

	bool b = (index!=0 || ui.cbXErrorType->currentIndex()!=0);
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
		curve->setYErrorType(XYCurve::ErrorType(index));
}

void XYCurveDock::yErrorPlusColumnChanged(const QModelIndex& index) const {
	Q_UNUSED(index);
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setYErrorPlusColumn(column);
}

void XYCurveDock::yErrorMinusColumnChanged(const QModelIndex& index) const {
	Q_UNUSED(index);
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setYErrorMinusColumn(column);
}

void XYCurveDock::errorBarsTypeChanged(int index) const {
	const auto type{XYCurve::ErrorBarsType(index)};
	bool b = (type == XYCurve::ErrorBarsType::WithEnds);
	ui.lErrorBarsCapSize->setVisible(b);
	ui.sbErrorBarsCapSize->setVisible(b);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setErrorBarsType(type);
}

void XYCurveDock::errorBarsCapSizeChanged(double value) const {
	if (m_initializing)
		return;

	float size = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : m_curvesList)
		curve->setErrorBarsCapSize(size);
}

void XYCurveDock::errorBarsStyleChanged(int index) const {
	if (index == -1 || m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->errorBarsPen();
		pen.setStyle(penStyle);
		curve->setErrorBarsPen(pen);
	}
}

void XYCurveDock::errorBarsColorChanged(const QColor& color) {
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

void XYCurveDock::errorBarsWidthChanged(double value) const {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->errorBarsPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		curve->setErrorBarsPen(pen);
	}
}

void XYCurveDock::errorBarsOpacityChanged(int value) const {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curvesList)
		curve->setErrorBarsOpacity(opacity);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.teComment->text())
		uiGeneralTab.teComment->setText(aspect->comment());
	m_initializing = false;
}

void XYCurveDock::curveXColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXColumn->setColumn(column, m_curve->xColumnPath());
	updateValuesWidgets();
	m_initializing = false;
}

void XYCurveDock::curveYColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbYColumn->setColumn(column, m_curve->yColumnPath());
	updateValuesWidgets();
	m_initializing = false;
}

void XYCurveDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	uiGeneralTab.chkVisible->setChecked(on);
	m_initializing = false;
}

//Line-Tab
void XYCurveDock::curveLineTypeChanged(XYCurve::LineType type) {
	m_initializing = true;
	ui.cbLineType->setCurrentIndex( (int) type);
	m_initializing = false;
}
void XYCurveDock::curveLineSkipGapsChanged(bool skip) {
	m_initializing = true;
	ui.chkLineSkipGaps->setChecked(skip);
	m_initializing = false;
}
void XYCurveDock::curveLineIncreasingXOnlyChanged(bool incr) {
	m_initializing = true;
	ui.chkLineIncreasingXOnly->setChecked(incr);
	m_initializing = false;
}
void XYCurveDock::curveLineInterpolationPointsCountChanged(int count) {
	m_initializing = true;
	ui.sbLineInterpolationPointsCount->setValue(count);
	m_initializing = false;
}
void XYCurveDock::curveLinePenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbLineStyle->setCurrentIndex( (int)pen.style());
	ui.kcbLineColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbLineStyle, pen.color());
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits( pen.widthF(), Worksheet::Unit::Point) );
	m_initializing = false;
}
void XYCurveDock::curveLineOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbLineOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}
void XYCurveDock::curveDropLineTypeChanged(XYCurve::DropLineType type) {
	m_initializing = true;
	ui.cbDropLineType->setCurrentIndex( (int)type );
	m_initializing = false;
}
void XYCurveDock::curveDropLinePenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbDropLineStyle->setCurrentIndex( (int) pen.style());
	ui.kcbDropLineColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbDropLineStyle, pen.color());
	ui.sbDropLineWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point) );
	m_initializing = false;
}
void XYCurveDock::curveDropLineOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbDropLineOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//Values-Tab
void XYCurveDock::curveValuesTypeChanged(XYCurve::ValuesType type) {
	m_initializing = true;
	ui.cbValuesType->setCurrentIndex((int) type);
	m_initializing = false;
}
void XYCurveDock::curveValuesColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbValuesColumn->setColumn(column, m_curve->valuesColumnPath());
	m_initializing = false;
}
void XYCurveDock::curveValuesPositionChanged(XYCurve::ValuesPosition position) {
	m_initializing = true;
	ui.cbValuesPosition->setCurrentIndex((int) position);
	m_initializing = false;
}
void XYCurveDock::curveValuesDistanceChanged(qreal distance) {
	m_initializing = true;
	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(distance, Worksheet::Unit::Point) );
	m_initializing = false;
}
void XYCurveDock::curveValuesRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbValuesRotation->setValue(angle);
	m_initializing = false;
}
void XYCurveDock::curveValuesNumericFormatChanged(char format) {
	m_initializing = true;
	ui.cbValuesNumericFormat->setCurrentIndex(ui.cbValuesNumericFormat->findData(format));
	m_initializing = false;
}
void XYCurveDock::curveValuesPrecisionChanged(int precision) {
	m_initializing = true;
	ui.sbValuesPrecision->setValue(precision);
	m_initializing = false;
}
void XYCurveDock::curveValuesDateTimeFormatChanged(const QString& format) {
	m_initializing = true;
	ui.cbValuesDateTimeFormat->setCurrentText(format);
	m_initializing = false;
}
void XYCurveDock::curveValuesOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbValuesOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}
void XYCurveDock::curveValuesPrefixChanged(const QString& prefix) {
	m_initializing = true;
	ui.leValuesPrefix->setText(prefix);
	m_initializing = false;
}
void XYCurveDock::curveValuesSuffixChanged(const QString& suffix) {
	m_initializing = true;
	ui.leValuesSuffix->setText(suffix);
	m_initializing = false;
}
void XYCurveDock::curveValuesFontChanged(QFont font) {
	m_initializing = true;
	font.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)) );
	ui.kfrValuesFont->setFont(font);
	m_initializing = false;
}
void XYCurveDock::curveValuesColorChanged(QColor color) {
	m_initializing = true;
	ui.kcbValuesColor->setColor(color);
	m_initializing = false;
}

//Filling
void XYCurveDock::curveFillingPositionChanged(XYCurve::FillingPosition position) {
	m_initializing = true;
	ui.cbFillingPosition->setCurrentIndex((int)position);
	m_initializing = false;
}
void XYCurveDock::curveFillingTypeChanged(PlotArea::BackgroundType type) {
	m_initializing = true;
	ui.cbFillingType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}
void XYCurveDock::curveFillingColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbFillingColorStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}
void XYCurveDock::curveFillingImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbFillingImageStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}
void XYCurveDock::curveFillingBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbFillingBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}
void XYCurveDock::curveFillingFirstColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbFillingFirstColor->setColor(color);
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, color);
	m_initializing = false;
}
void XYCurveDock::curveFillingSecondColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbFillingSecondColor->setColor(color);
	m_initializing = false;
}
void XYCurveDock::curveFillingFileNameChanged(QString& filename) {
	m_initializing = true;
	ui.leFillingFileName->setText(filename);
	m_initializing = false;
}
void XYCurveDock::curveFillingOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbFillingOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//"Error bars"-Tab
void XYCurveDock::curveXErrorTypeChanged(XYCurve::ErrorType type) {
	m_initializing = true;
	ui.cbXErrorType->setCurrentIndex((int) type);
	m_initializing = false;
}
void XYCurveDock::curveXErrorPlusColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXErrorPlusColumn->setColumn(column, m_curve->xErrorPlusColumnPath());
	m_initializing = false;
}
void XYCurveDock::curveXErrorMinusColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXErrorMinusColumn->setColumn(column, m_curve->xErrorMinusColumnPath());
	m_initializing = false;
}
void XYCurveDock::curveYErrorTypeChanged(XYCurve::ErrorType type) {
	m_initializing = true;
	ui.cbYErrorType->setCurrentIndex((int) type);
	m_initializing = false;
}
void XYCurveDock::curveYErrorPlusColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbYErrorPlusColumn->setColumn(column, m_curve->yErrorPlusColumnPath());
	m_initializing = false;
}

void XYCurveDock::curveYErrorMinusColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbYErrorMinusColumn->setColumn(column, m_curve->yErrorMinusColumnPath());
	m_initializing = false;
}
void XYCurveDock::curveErrorBarsCapSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbErrorBarsCapSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
	m_initializing = false;
}
void XYCurveDock::curveErrorBarsTypeChanged(XYCurve::ErrorBarsType type) {
	m_initializing = true;
	ui.cbErrorBarsType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}
void XYCurveDock::curveErrorBarsPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbErrorBarsStyle->setCurrentIndex( (int) pen.style());
	ui.kcbErrorBarsColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, pen.color());
	ui.sbErrorBarsWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point) );
	m_initializing = false;
}
void XYCurveDock::curveErrorBarsOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbErrorBarsOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void XYCurveDock::load() {
	//General
	//This data is read in XYCurveDock::setCurves().

	//Line
	ui.cbLineType->setCurrentIndex( (int) m_curve->lineType() );
	ui.chkLineSkipGaps->setChecked( m_curve->lineSkipGaps() );
	ui.sbLineInterpolationPointsCount->setValue( m_curve->lineInterpolationPointsCount() );
	ui.cbLineStyle->setCurrentIndex( (int) m_curve->linePen().style() );
	ui.kcbLineColor->setColor( m_curve->linePen().color() );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->linePen().widthF(), Worksheet::Unit::Point) );
	ui.sbLineOpacity->setValue( round(m_curve->lineOpacity()*100.0) );

	//Drop lines
	ui.cbDropLineType->setCurrentIndex( (int) m_curve->dropLineType() );
	ui.cbDropLineStyle->setCurrentIndex( (int) m_curve->dropLinePen().style() );
	ui.kcbDropLineColor->setColor( m_curve->dropLinePen().color() );
	ui.sbDropLineWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->dropLinePen().widthF(), Worksheet::Unit::Point) );
	ui.sbDropLineOpacity->setValue( round(m_curve->dropLineOpacity()*100.0) );

	//Values
	ui.cbValuesType->setCurrentIndex( (int) m_curve->valuesType() );
	ui.cbValuesPosition->setCurrentIndex( (int) m_curve->valuesPosition() );
	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(m_curve->valuesDistance(), Worksheet::Unit::Point) );
	ui.sbValuesRotation->setValue( m_curve->valuesRotationAngle() );
	ui.sbValuesOpacity->setValue( round(m_curve->valuesOpacity()*100.0) );
	ui.sbValuesPrecision->setValue(m_curve->valuesPrecision());
	ui.cbValuesNumericFormat->setCurrentIndex(ui.cbValuesNumericFormat->findData(m_curve->valuesNumericFormat()));
	ui.cbValuesDateTimeFormat->setCurrentText(m_curve->valuesDateTimeFormat());
	ui.leValuesPrefix->setText( m_curve->valuesPrefix() );
	ui.leValuesSuffix->setText( m_curve->valuesSuffix() );
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF( round(Worksheet::convertFromSceneUnits(valuesFont.pixelSize(), Worksheet::Unit::Point)) );
	ui.kfrValuesFont->setFont(valuesFont);
	ui.kcbValuesColor->setColor( m_curve->valuesColor() );
	this->updateValuesWidgets();

	//Filling
	ui.cbFillingPosition->setCurrentIndex( (int) m_curve->fillingPosition() );
	ui.cbFillingType->setCurrentIndex( (int)m_curve->fillingType() );
	ui.cbFillingColorStyle->setCurrentIndex( (int) m_curve->fillingColorStyle() );
	ui.cbFillingImageStyle->setCurrentIndex( (int) m_curve->fillingImageStyle() );
	ui.cbFillingBrushStyle->setCurrentIndex( (int) m_curve->fillingBrushStyle() );
	ui.leFillingFileName->setText( m_curve->fillingFileName() );
	ui.kcbFillingFirstColor->setColor( m_curve->fillingFirstColor() );
	ui.kcbFillingSecondColor->setColor( m_curve->fillingSecondColor() );
	ui.sbFillingOpacity->setValue( round(m_curve->fillingOpacity()*100.0) );

	//Error bars
	ui.cbXErrorType->setCurrentIndex( (int) m_curve->xErrorType() );
	ui.cbYErrorType->setCurrentIndex( (int) m_curve->yErrorType() );
	ui.cbErrorBarsType->setCurrentIndex( (int) m_curve->errorBarsType() );
	ui.sbErrorBarsCapSize->setValue( Worksheet::convertFromSceneUnits(m_curve->errorBarsCapSize(), Worksheet::Unit::Point) );
	ui.cbErrorBarsStyle->setCurrentIndex( (int) m_curve->errorBarsPen().style() );
	ui.kcbErrorBarsColor->setColor( m_curve->errorBarsPen().color() );
	ui.sbErrorBarsWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->errorBarsPen().widthF(), Worksheet::Unit::Point) );
	ui.sbErrorBarsOpacity->setValue( round(m_curve->errorBarsOpacity()*100.0) );

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, ui.kcbLineColor->color());
	GuiTools::updatePenStyles(ui.cbDropLineStyle, ui.kcbDropLineColor->color());
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, ui.kcbErrorBarsColor->color());
	m_initializing = false;
}

void XYCurveDock::loadConfigFromTemplate(KConfig& config) {
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

void XYCurveDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "XYCurve" );

	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.
	//This data is read in XYCurveDock::setCurves().

	//Line
	ui.cbLineType->setCurrentIndex( group.readEntry("LineType", (int) m_curve->lineType()) );
	ui.chkLineSkipGaps->setChecked( group.readEntry("LineSkipGaps", m_curve->lineSkipGaps()) );
	ui.sbLineInterpolationPointsCount->setValue( group.readEntry("LineInterpolationPointsCount", m_curve->lineInterpolationPointsCount()) );
	ui.cbLineStyle->setCurrentIndex( group.readEntry("LineStyle", (int) m_curve->linePen().style()) );
	ui.kcbLineColor->setColor( group.readEntry("LineColor", m_curve->linePen().color()) );
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LineWidth", m_curve->linePen().widthF()), Worksheet::Unit::Point) );
	ui.sbLineOpacity->setValue( round(group.readEntry("LineOpacity", m_curve->lineOpacity())*100.0) );

	//Drop lines
	ui.cbDropLineType->setCurrentIndex( group.readEntry("DropLineType", (int) m_curve->dropLineType()) );
	ui.cbDropLineStyle->setCurrentIndex( group.readEntry("DropLineStyle", (int) m_curve->dropLinePen().style()) );
	ui.kcbDropLineColor->setColor( group.readEntry("DropLineColor", m_curve->dropLinePen().color()) );
	ui.sbDropLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("DropLineWidth", m_curve->dropLinePen().widthF()), Worksheet::Unit::Point) );
	ui.sbDropLineOpacity->setValue( round(group.readEntry("DropLineOpacity", m_curve->dropLineOpacity())*100.0) );

	//Symbols
	symbolWidget->loadConfig(group);

	//Values
	ui.cbValuesType->setCurrentIndex( group.readEntry("ValuesType", (int) m_curve->valuesType()) );
	ui.cbValuesPosition->setCurrentIndex( group.readEntry("ValuesPosition", (int) m_curve->valuesPosition()) );
	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ValuesDistance", m_curve->valuesDistance()), Worksheet::Unit::Point) );
	ui.sbValuesRotation->setValue( group.readEntry("ValuesRotation", m_curve->valuesRotationAngle()) );
	ui.sbValuesOpacity->setValue( round(group.readEntry("ValuesOpacity",m_curve->valuesOpacity())*100.0) );
	ui.leValuesPrefix->setText( group.readEntry("ValuesPrefix", m_curve->valuesPrefix()) );
	ui.leValuesSuffix->setText( group.readEntry("ValuesSuffix", m_curve->valuesSuffix()) );
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF( round(Worksheet::convertFromSceneUnits(valuesFont.pixelSize(), Worksheet::Unit::Point)) );
	ui.kfrValuesFont->setFont( group.readEntry("ValuesFont", valuesFont) );
	ui.kcbValuesColor->setColor( group.readEntry("ValuesColor", m_curve->valuesColor()) );

	//Filling
	ui.cbFillingPosition->setCurrentIndex( group.readEntry("FillingPosition", (int) m_curve->fillingPosition()) );
	ui.cbFillingType->setCurrentIndex( group.readEntry("FillingType", (int) m_curve->fillingType()) );
	ui.cbFillingColorStyle->setCurrentIndex( group.readEntry("FillingColorStyle", (int) m_curve->fillingColorStyle()) );
	ui.cbFillingImageStyle->setCurrentIndex( group.readEntry("FillingImageStyle", (int) m_curve->fillingImageStyle()) );
	ui.cbFillingBrushStyle->setCurrentIndex( group.readEntry("FillingBrushStyle", (int) m_curve->fillingBrushStyle()) );
	ui.leFillingFileName->setText( group.readEntry("FillingFileName", m_curve->fillingFileName()) );
	ui.kcbFillingFirstColor->setColor( group.readEntry("FillingFirstColor", m_curve->fillingFirstColor()) );
	ui.kcbFillingSecondColor->setColor( group.readEntry("FillingSecondColor", m_curve->fillingSecondColor()) );
	ui.sbFillingOpacity->setValue( round(group.readEntry("FillingOpacity", m_curve->fillingOpacity())*100.0) );

	//Error bars
	ui.cbXErrorType->setCurrentIndex( group.readEntry("XErrorType", (int) m_curve->xErrorType()) );
	ui.cbYErrorType->setCurrentIndex( group.readEntry("YErrorType", (int) m_curve->yErrorType()) );
	ui.cbErrorBarsType->setCurrentIndex( group.readEntry("ErrorBarsType", (int) m_curve->errorBarsType()) );
	ui.sbErrorBarsCapSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ErrorBarsCapSize", m_curve->errorBarsCapSize()), Worksheet::Unit::Point) );
	ui.cbErrorBarsStyle->setCurrentIndex( group.readEntry("ErrorBarsStyle", (int) m_curve->errorBarsPen().style()) );
	ui.kcbErrorBarsColor->setColor( group.readEntry("ErrorBarsColor", m_curve->errorBarsPen().color()) );
	ui.sbErrorBarsWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ErrorBarsWidth", m_curve->errorBarsPen().widthF()), Worksheet::Unit::Point) );
	ui.sbErrorBarsOpacity->setValue( round(group.readEntry("ErrorBarsOpacity", m_curve->errorBarsOpacity())*100.0) );

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, ui.kcbLineColor->color());
	GuiTools::updatePenStyles(ui.cbDropLineStyle, ui.kcbDropLineColor->color());
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, ui.kcbErrorBarsColor->color());
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, ui.kcbFillingFirstColor->color());
	m_initializing = false;
}

void XYCurveDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("XYCurve");

	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.

	group.writeEntry("LineType", ui.cbLineType->currentIndex());
	group.writeEntry("LineSkipGaps", ui.chkLineSkipGaps->isChecked());
	group.writeEntry("LineInterpolationPointsCount", ui.sbLineInterpolationPointsCount->value() );
	group.writeEntry("LineStyle", ui.cbLineStyle->currentIndex());
	group.writeEntry("LineColor", ui.kcbLineColor->color());
	group.writeEntry("LineWidth", Worksheet::convertToSceneUnits(ui.sbLineWidth->value(), Worksheet::Unit::Point) );
	group.writeEntry("LineOpacity", ui.sbLineOpacity->value()/100.0);

	//Drop Line
	group.writeEntry("DropLineType", ui.cbDropLineType->currentIndex());
	group.writeEntry("DropLineStyle", ui.cbDropLineStyle->currentIndex());
	group.writeEntry("DropLineColor", ui.kcbDropLineColor->color());
	group.writeEntry("DropLineWidth", Worksheet::convertToSceneUnits(ui.sbDropLineWidth->value(), Worksheet::Unit::Point) );
	group.writeEntry("DropLineOpacity", ui.sbDropLineOpacity->value()/100.0);

	//Symbols
	symbolWidget->saveConfig(group);

	//Values
	group.writeEntry("ValuesType", ui.cbValuesType->currentIndex());
	group.writeEntry("ValuesPosition", ui.cbValuesPosition->currentIndex());
	group.writeEntry("ValuesDistance", Worksheet::convertToSceneUnits(ui.sbValuesDistance->value(), Worksheet::Unit::Point));
	group.writeEntry("ValuesRotation", ui.sbValuesRotation->value());
	group.writeEntry("ValuesOpacity", ui.sbValuesOpacity->value()/100.0);
	group.writeEntry("valuesNumericFormat", ui.cbValuesNumericFormat->currentText());
	group.writeEntry("valuesPrecision", ui.sbValuesPrecision->value());
	group.writeEntry("valuesDateTimeFormat", ui.cbValuesDateTimeFormat->currentText());
	group.writeEntry("ValuesPrefix", ui.leValuesPrefix->text());
	group.writeEntry("ValuesSuffix", ui.leValuesSuffix->text());
	group.writeEntry("ValuesFont", ui.kfrValuesFont->font());
	group.writeEntry("ValuesColor", ui.kcbValuesColor->color());

	//Filling
	group.writeEntry("FillingPosition", ui.cbFillingPosition->currentIndex());
	group.writeEntry("FillingType", ui.cbFillingType->currentIndex());
	group.writeEntry("FillingColorStyle", ui.cbFillingColorStyle->currentIndex());
	group.writeEntry("FillingImageStyle", ui.cbFillingImageStyle->currentIndex());
	group.writeEntry("FillingBrushStyle", ui.cbFillingBrushStyle->currentIndex());
	group.writeEntry("FillingFileName", ui.leFillingFileName->text());
	group.writeEntry("FillingFirstColor", ui.kcbFillingFirstColor->color());
	group.writeEntry("FillingSecondColor", ui.kcbFillingSecondColor->color());
	group.writeEntry("FillingOpacity", ui.sbFillingOpacity->value()/100.0);

	//Error bars
	group.writeEntry("XErrorType", ui.cbXErrorType->currentIndex());
	group.writeEntry("YErrorType", ui.cbYErrorType->currentIndex());
	group.writeEntry("ErrorBarsType", ui.cbErrorBarsType->currentIndex());
	group.writeEntry("ErrorBarsCapSize", Worksheet::convertToSceneUnits(ui.sbErrorBarsCapSize->value(), Worksheet::Unit::Point) );
	group.writeEntry("ErrorBarsStyle", ui.cbErrorBarsStyle->currentIndex());
	group.writeEntry("ErrorBarsColor", ui.kcbErrorBarsColor->color());
	group.writeEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(ui.sbErrorBarsWidth->value(), Worksheet::Unit::Point) );
	group.writeEntry("ErrorBarsOpacity", ui.sbErrorBarsOpacity->value()/100.0);

	config.sync();
}
