/***************************************************************************
    File                 : XYCurveDock.cpp
    Project              : LabPlot
    Description          : widget for XYCurve properties
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2017 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
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
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/*!
  \class XYCurveDock
  \brief  Provides a widget for editing the properties of the XYCurves (2D-curves) currently selected in the project explorer.

  If more than one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYCurveDock::XYCurveDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);

	//Tab "Values"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabValues->layout());
	cbValuesColumn = new TreeViewComboBox(ui.tabValues);
	gridLayout->addWidget(cbValuesColumn, 2, 2, 1, 1);

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

	//Symbol
	connect( ui.cbSymbolStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolsStyleChanged(int)) );
	connect( ui.sbSymbolSize, SIGNAL(valueChanged(double)), this, SLOT(symbolsSizeChanged(double)) );
	connect( ui.sbSymbolRotation, SIGNAL(valueChanged(int)), this, SLOT(symbolsRotationChanged(int)) );
	connect( ui.sbSymbolOpacity, SIGNAL(valueChanged(int)), this, SLOT(symbolsOpacityChanged(int)) );

	connect( ui.cbSymbolFillingStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolsFillingStyleChanged(int)) );
	connect( ui.kcbSymbolFillingColor, SIGNAL(changed(QColor)), this, SLOT(symbolsFillingColorChanged(QColor)) );

	connect( ui.cbSymbolBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolsBorderStyleChanged(int)) );
	connect( ui.kcbSymbolBorderColor, SIGNAL(changed(QColor)), this, SLOT(symbolsBorderColorChanged(QColor)) );
	connect( ui.sbSymbolBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(symbolsBorderWidthChanged(double)) );

	//Values
	connect( ui.cbValuesType, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesTypeChanged(int)) );
	connect( cbValuesColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(valuesColumnChanged(QModelIndex)) );
	connect( ui.cbValuesPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesPositionChanged(int)) );
	connect( ui.sbValuesDistance, SIGNAL(valueChanged(double)), this, SLOT(valuesDistanceChanged(double)) );
	connect( ui.sbValuesRotation, SIGNAL(valueChanged(int)), this, SLOT(valuesRotationChanged(int)) );
	connect( ui.sbValuesOpacity, SIGNAL(valueChanged(int)), this, SLOT(valuesOpacityChanged(int)) );

	//TODO connect( ui.cbValuesFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesColumnFormatChanged(int)) );
	connect( ui.leValuesPrefix, SIGNAL(returnPressed()), this, SLOT(valuesPrefixChanged()) );
	connect( ui.leValuesSuffix, SIGNAL(returnPressed()), this, SLOT(valuesSuffixChanged()) );
	connect( ui.kfrValuesFont, SIGNAL(fontSelected(QFont)), this, SLOT(valuesFontChanged(QFont)) );
	connect( ui.kcbValuesColor, SIGNAL(changed(QColor)), this, SLOT(valuesColorChanged(QColor)) );

	//Filling
	connect( ui.cbFillingPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingPositionChanged(int)) );
	connect( ui.cbFillingType, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingTypeChanged(int)) );
	connect( ui.cbFillingColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingColorStyleChanged(int)) );
	connect( ui.cbFillingImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingImageStyleChanged(int)) );
	connect( ui.cbFillingBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingBrushStyleChanged(int)) );
	connect(ui.bFillingOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect( ui.leFillingFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
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

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::XYCurve);
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
	m_leComment = uiGeneralTab.leComment;
	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(generalTab->layout());

	cbXColumn = new TreeViewComboBox(generalTab);
	cbXColumn->useCurrentIndexText(false);
	gridLayout->addWidget(cbXColumn, 2, 2, 1, 1);

	cbYColumn = new TreeViewComboBox(generalTab);
	cbYColumn->useCurrentIndexText(false);
	gridLayout->addWidget(cbYColumn, 3, 2, 1, 1);

	//General
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYCurveDock::nameChanged);
	connect(uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYCurveDock::commentChanged);
	connect(uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)));
	connect(cbXColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xColumnChanged(QModelIndex)));
	connect(cbYColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yColumnChanged(QModelIndex)));
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

	//Symbols
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, Qt::black);

	ui.cbSymbolStyle->setIconSize(QSize(iconSize, iconSize));
	QTransform trafo;
	trafo.scale(15, 15);

	ui.cbSymbolStyle->addItem(i18n("None"));
	for (int i = 1; i < 19; ++i) {	//TODO: use enum count
		const auto style = (Symbol::Style)i;
		pm.fill(Qt::transparent);
		pa.begin(&pm);
		pa.setPen(pen);
		pa.setRenderHint(QPainter::Antialiasing);
		pa.translate(iconSize/2,iconSize/2);
		pa.drawPath(trafo.map(Symbol::pathFromStyle(style)));
		pa.end();
		ui.cbSymbolStyle->addItem(QIcon(pm), Symbol::nameFromStyle(style));
	}

	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, Qt::black);
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
	                       AspectType::XYFitCurve, AspectType::CantorWorksheet};

	if (cbXColumn) {
		cbXColumn->setTopLevelClasses(list);
		cbYColumn->setTopLevelClasses(list);
	}
	cbValuesColumn->setTopLevelClasses(list);
	cbXErrorMinusColumn->setTopLevelClasses(list);
	cbXErrorPlusColumn->setTopLevelClasses(list);
	cbYErrorMinusColumn->setTopLevelClasses(list);
	cbYErrorPlusColumn->setTopLevelClasses(list);

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

	if (cbXColumn) {
		QString path = m_curve->xColumnPath().split("/").last();
		if (m_curve->xColumn()) {
			path += QString("\t ")+m_curve->xColumn()->plotDesignationString();
			cbXColumn->setInvalid(false);
		} else
			cbXColumn->setInvalid(true, i18n("The column")+ " \""+ m_curve->xColumnPath() + "\"\n"+ i18n("is not available. If a new column at this path is created, it is linked to this curve. If you wanna hold this column, don't change anything in this combobox."));
		cbXColumn->setText(path);

		path = m_curve->yColumnPath().split("/").last();
		if (m_curve->yColumn()) {
			path += QString("\t ")+m_curve->yColumn()->plotDesignationString();
			cbYColumn->setInvalid(false);
		} else
			cbYColumn->setInvalid(true, i18n("The column")+ " \""+ m_curve->xColumnPath() + "\"\n"+ i18n("is not available. If a new column at this path is created, it is linked to this curve. If you wanna hold this column, don't change anything in this combobox."));
		cbYColumn->setText(path);
	}
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	setModel();
	initGeneralTab();
	initTabs();
	m_initializing = false;
}

void XYCurveDock::initGeneralTab() {
	DEBUG("XYCurveDock::initGeneralTab()");
	//if there are more than one curve in the list, disable the content in the tab "general"
	if (m_curvesList.size() == 1) {
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.leComment->setEnabled(true);

		uiGeneralTab.lXColumn->setEnabled(true);
		cbXColumn->setEnabled(true);
		uiGeneralTab.lYColumn->setEnabled(true);
		cbYColumn->setEnabled(true);

		DEBUG("setModelIndexFromAspect()");
		this->setModelIndexFromAspect(cbXColumn, m_curve->xColumn());
		this->setModelIndexFromAspect(cbYColumn, m_curve->yColumn());

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.leComment->setText(m_curve->comment());
	} else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.leComment->setEnabled(false);

		uiGeneralTab.lXColumn->setEnabled(false);
		cbXColumn->setEnabled(false);
		uiGeneralTab.lYColumn->setEnabled(false);
		cbYColumn->setEnabled(false);

		cbXColumn->setCurrentModelIndex(QModelIndex());
		cbYColumn->setCurrentModelIndex(QModelIndex());

		uiGeneralTab.leName->setText(QString());
		uiGeneralTab.leComment->setText(QString());
	}

	checkColumnAvailability(cbXColumn, m_curve->xColumn(), m_curve->xColumnPath());
	checkColumnAvailability(cbYColumn, m_curve->yColumn(), m_curve->yColumnPath());
	checkColumnAvailability(cbValuesColumn, m_curve->valuesColumn(), m_curve->valuesColumnPath());
	checkColumnAvailability(cbXErrorPlusColumn, m_curve->xErrorPlusColumn(), m_curve->xErrorPlusColumnPath());
	checkColumnAvailability(cbXErrorMinusColumn, m_curve->xErrorMinusColumn(), m_curve->xErrorMinusColumnPath());
	checkColumnAvailability(cbYErrorPlusColumn, m_curve->yErrorPlusColumn(), m_curve->yErrorPlusColumnPath());
	checkColumnAvailability(cbYErrorMinusColumn, m_curve->yErrorMinusColumn(), m_curve->yErrorMinusColumnPath());

	//show the properties of the first curve
	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_curve, SIGNAL(xColumnChanged(const AbstractColumn*)), this, SLOT(curveXColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(yColumnChanged(const AbstractColumn*)), this, SLOT(curveYColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(visibilityChanged(bool)), this, SLOT(curveVisibilityChanged(bool)));
	DEBUG("XYCurveDock::initGeneralTab() DONE");
}

void XYCurveDock::initTabs() {
	//if there are more than one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1) {
		this->setModelIndexFromAspect(cbValuesColumn, m_curve->valuesColumn());
		this->setModelIndexFromAspect(cbXErrorPlusColumn, m_curve->xErrorPlusColumn());
		this->setModelIndexFromAspect(cbXErrorMinusColumn, m_curve->xErrorMinusColumn());
		this->setModelIndexFromAspect(cbYErrorPlusColumn, m_curve->yErrorPlusColumn());
		this->setModelIndexFromAspect(cbYErrorMinusColumn, m_curve->yErrorMinusColumn());
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

	//Symbol-Tab
	connect(m_curve, SIGNAL(symbolsStyleChanged(Symbol::Style)), this, SLOT(curveSymbolsStyleChanged(Symbol::Style)));
	connect(m_curve, SIGNAL(symbolsSizeChanged(qreal)), this, SLOT(curveSymbolsSizeChanged(qreal)));
	connect(m_curve, SIGNAL(symbolsRotationAngleChanged(qreal)), this, SLOT(curveSymbolsRotationAngleChanged(qreal)));
	connect(m_curve, SIGNAL(symbolsOpacityChanged(qreal)), this, SLOT(curveSymbolsOpacityChanged(qreal)));
	connect(m_curve, SIGNAL(symbolsBrushChanged(QBrush)), this, SLOT(curveSymbolsBrushChanged(QBrush)));
	connect(m_curve, SIGNAL(symbolsPenChanged(QPen)), this, SLOT(curveSymbolsPenChanged(QPen)));

	//Values-Tab
	connect(m_curve, SIGNAL(valuesTypeChanged(XYCurve::ValuesType)), this, SLOT(curveValuesTypeChanged(XYCurve::ValuesType)));
	connect(m_curve, SIGNAL(valuesColumnChanged(const AbstractColumn*)), this, SLOT(curveValuesColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(valuesPositionChanged(XYCurve::ValuesPosition)), this, SLOT(curveValuesPositionChanged(XYCurve::ValuesPosition)));
	connect(m_curve, SIGNAL(valuesDistanceChanged(qreal)), this, SLOT(curveValuesDistanceChanged(qreal)));
	connect(m_curve, SIGNAL(valuesOpacityChanged(qreal)), this, SLOT(curveValuesOpacityChanged(qreal)));
	connect(m_curve, SIGNAL(valuesRotationAngleChanged(qreal)), this, SLOT(curveValuesRotationAngleChanged(qreal)));
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

/*!
  depending on the currently selected values column type (column mode) updates the widgets for the values column format,
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the values column was changed.

  synchronize this function with ColumnDock::updateFormat.
*/
void XYCurveDock::updateValuesFormatWidgets(const AbstractColumn::ColumnMode columnMode) {
	ui.cbValuesFormat->clear();

	switch (columnMode) {
	case AbstractColumn::Numeric:
		ui.cbValuesFormat->addItem(i18n("Decimal"), QVariant('f'));
		ui.cbValuesFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
		ui.cbValuesFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
		ui.cbValuesFormat->addItem(i18n("Automatic (e)"), QVariant('g'));
		ui.cbValuesFormat->addItem(i18n("Automatic (E)"), QVariant('G'));
		break;
	case AbstractColumn::Integer:
		break;
	case AbstractColumn::Text:
		ui.cbValuesFormat->addItem(i18n("Text"), QVariant());
		break;
	case AbstractColumn::Month:
		ui.cbValuesFormat->addItem(i18n("Number without Leading Zero"), QVariant("M"));
		ui.cbValuesFormat->addItem(i18n("Number with Leading Zero"), QVariant("MM"));
		ui.cbValuesFormat->addItem(i18n("Abbreviated Month Name"), QVariant("MMM"));
		ui.cbValuesFormat->addItem(i18n("Full Month Name"), QVariant("MMMM"));
		break;
	case AbstractColumn::Day:
		ui.cbValuesFormat->addItem(i18n("Number without Leading Zero"), QVariant("d"));
		ui.cbValuesFormat->addItem(i18n("Number with Leading Zero"), QVariant("dd"));
		ui.cbValuesFormat->addItem(i18n("Abbreviated Day Name"), QVariant("ddd"));
		ui.cbValuesFormat->addItem(i18n("Full Day Name"), QVariant("dddd"));
		break;
	case AbstractColumn::DateTime: {
			for (const auto& s : AbstractColumn::dateFormats())
				ui.cbValuesFormat->addItem(s, QVariant(s));

			for (const auto& s : AbstractColumn::timeFormats())
				ui.cbValuesFormat->addItem(s, QVariant(s));

			for (const auto& s1 : AbstractColumn::dateFormats()) {
				for (const auto& s2 : AbstractColumn::timeFormats())
					ui.cbValuesFormat->addItem(s1 + ' ' + s2, QVariant(s1 + ' ' + s2));
			}
			break;
		}
	}

	if (columnMode == AbstractColumn::Numeric) {
		ui.lValuesPrecision->show();
		ui.sbValuesPrecision->show();
	} else {
		ui.lValuesPrecision->hide();
		ui.sbValuesPrecision->hide();
	}

	if (columnMode == AbstractColumn::Text) {
		ui.lValuesFormatTop->hide();
		ui.lValuesFormat->hide();
		ui.cbValuesFormat->hide();
	} else {
		ui.lValuesFormatTop->show();
		ui.lValuesFormat->show();
		ui.cbValuesFormat->show();
	}

	if (columnMode == AbstractColumn::DateTime) {
		ui.cbValuesFormat->setCurrentItem("yyyy-MM-dd hh:mm:ss.zzz");
		ui.cbValuesFormat->setEditable(true);
	} else {
		ui.cbValuesFormat->setCurrentIndex(0);
		ui.cbValuesFormat->setEditable(false);
	}
}

void XYCurveDock::checkColumnAvailability(TreeViewComboBox* cb, const AbstractColumn* column, const QString columnPath) {
	if (!cb)
		return;// normally it shouldn't be called

	// don't make the comboboxes red for initially created curves
	if (!column && columnPath.isEmpty()) {
		cb->setText("");
		cb->setInvalid(false);
		return;
	}

	if (column) {
		// current index text should be used
		cb->useCurrentIndexText(true);
		cb->setInvalid(false);
	} else {
		cb->useCurrentIndexText(false);
		cb->setInvalid(true, i18n("The column \"%1\"\nis not available anymore. It will be automatically used once it is created again.", columnPath));
	}
	cb->setText(columnPath.split("/").last());
}

/*!
  shows the formatting properties of the column \c column.
  Called, when a new column for the values was selected - either by changing the type of the values (none, x, y, etc.) or
  by selecting a new custom column for the values.
*/
void XYCurveDock::showValuesColumnFormat(const Column* column) {
	if (!column) {
		// no valid column is available
		// -> hide all the format properties widgets (equivalent to showing the properties of the column mode "Text")
		this->updateValuesFormatWidgets(AbstractColumn::Text);
	} else {
		AbstractColumn::ColumnMode columnMode = column->columnMode();

		//update the format widgets for the new column mode
		this->updateValuesFormatWidgets(columnMode);

		//show the actual formatting properties
		switch (columnMode) {
		case AbstractColumn::Numeric: {
			const auto* filter = static_cast<Double2StringFilter*>(column->outputFilter());
			ui.cbValuesFormat->setCurrentIndex(ui.cbValuesFormat->findData(filter->numericFormat()));
			ui.sbValuesPrecision->setValue(filter->numDigits());
			break;
		}
		case AbstractColumn::Integer:
		case AbstractColumn::Text:
			break;
		case AbstractColumn::Month:
		case AbstractColumn::Day:
		case AbstractColumn::DateTime: {
			const auto* filter = static_cast<DateTime2StringFilter*>(column->outputFilter());
			DEBUG("	column values format = " << filter->format().toStdString());
			ui.cbValuesFormat->setCurrentIndex(ui.cbValuesFormat->findData(filter->format()));
			break;
		}
		}
	}
}

void XYCurveDock::setModelIndexFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (aspect)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(aspect));
	else
		cb->setCurrentModelIndex(QModelIndex());
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

	if ( lineType == XYCurve::NoLine) {
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

		if (lineType == XYCurve::SplineCubicNatural || lineType == XYCurve::SplineCubicPeriodic
		        || lineType == XYCurve::SplineAkimaNatural || lineType == XYCurve::SplineAkimaPeriodic) {
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
	if (m_initializing)
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
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
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

	if ( dropLineType == XYCurve::NoDropLine) {
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
	if (m_initializing)
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
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
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

//"Symbol"-tab
void XYCurveDock::symbolsStyleChanged(int index) {
	const auto style = Symbol::Style(index);

	if (style == Symbol::NoSymbols) {
		ui.sbSymbolSize->setEnabled(false);
		ui.sbSymbolRotation->setEnabled(false);
		ui.sbSymbolOpacity->setEnabled(false);

		ui.kcbSymbolFillingColor->setEnabled(false);
		ui.cbSymbolFillingStyle->setEnabled(false);

		ui.cbSymbolBorderStyle->setEnabled(false);
		ui.kcbSymbolBorderColor->setEnabled(false);
		ui.sbSymbolBorderWidth->setEnabled(false);
	} else {
		ui.sbSymbolSize->setEnabled(true);
		ui.sbSymbolRotation->setEnabled(true);
		ui.sbSymbolOpacity->setEnabled(true);

		//enable/disable the symbol filling options in the GUI depending on the currently selected symbol.
		if (style != Symbol::Line && style != Symbol::Cross) {
			ui.cbSymbolFillingStyle->setEnabled(true);
			bool noBrush = (Qt::BrushStyle(ui.cbSymbolFillingStyle->currentIndex()) == Qt::NoBrush);
			ui.kcbSymbolFillingColor->setEnabled(!noBrush);
		} else {
			ui.kcbSymbolFillingColor->setEnabled(false);
			ui.cbSymbolFillingStyle->setEnabled(false);
		}

		ui.cbSymbolBorderStyle->setEnabled(true);
		bool noLine = (Qt::PenStyle(ui.cbSymbolBorderStyle->currentIndex()) == Qt::NoPen);
		ui.kcbSymbolBorderColor->setEnabled(!noLine);
		ui.sbSymbolBorderWidth->setEnabled(!noLine);
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setSymbolsStyle(style);
}

void XYCurveDock::symbolsSizeChanged(double value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setSymbolsSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void XYCurveDock::symbolsRotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setSymbolsRotationAngle(value);
}

void XYCurveDock::symbolsOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve : m_curvesList)
		curve->setSymbolsOpacity(opacity);
}

void XYCurveDock::symbolsFillingStyleChanged(int index) {
	const auto brushStyle = Qt::BrushStyle(index);
	ui.kcbSymbolFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	if (m_initializing)
		return;

	QBrush brush;
	for (auto* curve : m_curvesList) {
		brush = curve->symbolsBrush();
		brush.setStyle(brushStyle);
		curve->setSymbolsBrush(brush);
	}
}

void XYCurveDock::symbolsFillingColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QBrush brush;
	for (auto* curve : m_curvesList) {
		brush = curve->symbolsBrush();
		brush.setColor(color);
		curve->setSymbolsBrush(brush);
	}

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, color );
	m_initializing = false;
}

void XYCurveDock::symbolsBorderStyleChanged(int index) {
	const auto penStyle = Qt::PenStyle(index);

	if ( penStyle == Qt::NoPen ) {
		ui.kcbSymbolBorderColor->setEnabled(false);
		ui.sbSymbolBorderWidth->setEnabled(false);
	} else {
		ui.kcbSymbolBorderColor->setEnabled(true);
		ui.sbSymbolBorderWidth->setEnabled(true);
	}

	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->symbolsPen();
		pen.setStyle(penStyle);
		curve->setSymbolsPen(pen);
	}
}

void XYCurveDock::symbolsBorderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->symbolsPen();
		pen.setColor(color);
		curve->setSymbolsPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, color);
	m_initializing = false;
}

void XYCurveDock::symbolsBorderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve : m_curvesList) {
		pen = curve->symbolsPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
		curve->setSymbolsPen(pen);
	}
}

//Values-tab

/*!
  called when the type of the values (none, x, y, (x,y) etc.) was changed.
*/
void XYCurveDock::valuesTypeChanged(int index) {
	const auto valuesType = XYCurve::ValuesType(index);

	if (valuesType == XYCurve::NoValues) {
		//no values are to paint -> deactivate all the pertinent widgets
		ui.cbValuesPosition->setEnabled(false);
		ui.lValuesColumn->hide();
		cbValuesColumn->hide();
		ui.sbValuesDistance->setEnabled(false);
		ui.sbValuesRotation->setEnabled(false);
		ui.sbValuesOpacity->setEnabled(false);
		ui.cbValuesFormat->setEnabled(false);
		ui.cbValuesFormat->setEnabled(false);
		ui.sbValuesPrecision->setEnabled(false);
		ui.leValuesPrefix->setEnabled(false);
		ui.leValuesSuffix->setEnabled(false);
		ui.kfrValuesFont->setEnabled(false);
		ui.kcbValuesColor->setEnabled(false);
	} else {
		ui.cbValuesPosition->setEnabled(true);
		ui.sbValuesDistance->setEnabled(true);
		ui.sbValuesRotation->setEnabled(true);
		ui.sbValuesOpacity->setEnabled(true);
		ui.cbValuesFormat->setEnabled(true);
		ui.sbValuesPrecision->setEnabled(true);
		ui.leValuesPrefix->setEnabled(true);
		ui.leValuesSuffix->setEnabled(true);
		ui.kfrValuesFont->setEnabled(true);
		ui.kcbValuesColor->setEnabled(true);

		const Column* column;
		if (valuesType == XYCurve::ValuesCustomColumn) {
			ui.lValuesColumn->show();
			cbValuesColumn->show();

			column = static_cast<Column*>(cbValuesColumn->currentModelIndex().internalPointer());
		} else {
			ui.lValuesColumn->hide();
			cbValuesColumn->hide();

			if (valuesType == XYCurve::ValuesY)
				column = static_cast<const Column*>(m_curve->yColumn());
			else
				column = static_cast<const Column*>(m_curve->xColumn());
		}
		this->showValuesColumnFormat(column);
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesType(valuesType);
}

/*!
  called when the custom column for the values was changed.
*/
void XYCurveDock::valuesColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* column = static_cast<Column*>(index.internalPointer());
	this->showValuesColumnFormat(column);

	for (auto* curve : m_curvesList) {
		//TODO save also the format of the currently selected column for the values (precision etc.)
		curve->setValuesColumn(column);
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
		curve->setValuesDistance( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
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
	valuesFont.setPixelSize( Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Point) );
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
	const auto fillingPosition = XYCurve::FillingPosition(index);

	bool b = (fillingPosition != XYCurve::NoFilling);
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

	if (type == PlotArea::Color) {
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
		if (style == PlotArea::SingleColor) {
			ui.lFillingFirstColor->setText(i18n("Color:"));
			ui.lFillingSecondColor->hide();
			ui.kcbFillingSecondColor->hide();
		} else {
			ui.lFillingFirstColor->setText(i18n("First color:"));
			ui.lFillingSecondColor->show();
			ui.kcbFillingSecondColor->show();
		}
	} else if (type == PlotArea::Image) {
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
	} else if (type == PlotArea::Pattern) {
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

	if (style == PlotArea::SingleColor) {
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
	if (m_initializing)
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
		formats.isEmpty() ? formats += f : formats += ' ' + f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
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
	auto type = XYCurve::ErrorBarsType(index);
	bool b = (type == XYCurve::ErrorBarsWithEnds);
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

	float size = Worksheet::convertToSceneUnits(value, Worksheet::Point);
	for (auto* curve : m_curvesList)
		curve->setErrorBarsCapSize(size);
}

void XYCurveDock::errorBarsStyleChanged(int index) const {
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
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
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
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYCurveDock::curveXColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	this->setModelIndexFromAspect(cbXColumn, column);
	cbXColumn->useCurrentIndexText(true);
	cbXColumn->setInvalid(false);
	m_initializing = false;
}

void XYCurveDock::curveYColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	this->setModelIndexFromAspect(cbYColumn, column);
	cbYColumn->useCurrentIndexText(true);
	cbYColumn->setInvalid(false);
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
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits( pen.widthF(), Worksheet::Point) );
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
	ui.sbDropLineWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Point) );
	m_initializing = false;
}
void XYCurveDock::curveDropLineOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbDropLineOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//Symbol-Tab
void XYCurveDock::curveSymbolsStyleChanged(Symbol::Style style) {
	m_initializing = true;
	ui.cbSymbolStyle->setCurrentIndex((int)style);
	m_initializing = false;
}
void XYCurveDock::curveSymbolsSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
	m_initializing = false;
}
void XYCurveDock::curveSymbolsRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbSymbolRotation->setValue(angle);
	m_initializing = false;
}
void XYCurveDock::curveSymbolsOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbSymbolOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}
void XYCurveDock::curveSymbolsBrushChanged(const QBrush& brush) {
	m_initializing = true;
	ui.cbSymbolFillingStyle->setCurrentIndex((int) brush.style());
	ui.kcbSymbolFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, brush.color());
	m_initializing = false;
}
void XYCurveDock::curveSymbolsPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbSymbolBorderStyle->setCurrentIndex( (int) pen.style());
	ui.kcbSymbolBorderColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, pen.color());
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Point));
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
	this->setModelIndexFromAspect(cbValuesColumn, column);
	m_initializing = false;
}
void XYCurveDock::curveValuesPositionChanged(XYCurve::ValuesPosition position) {
	m_initializing = true;
	ui.cbValuesPosition->setCurrentIndex((int) position);
	m_initializing = false;
}
void XYCurveDock::curveValuesDistanceChanged(qreal distance) {
	m_initializing = true;
	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(distance, Worksheet::Point) );
	m_initializing = false;
}
void XYCurveDock::curveValuesRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbValuesRotation->setValue(angle);
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
	font.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Point)) );
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
	ui.cbFillingType->setCurrentIndex(type);
	m_initializing = false;
}
void XYCurveDock::curveFillingColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbFillingColorStyle->setCurrentIndex(style);
	m_initializing = false;
}
void XYCurveDock::curveFillingImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbFillingImageStyle->setCurrentIndex(style);
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
	this->setModelIndexFromAspect(cbXErrorPlusColumn, column);
	m_initializing = false;
}
void XYCurveDock::curveXErrorMinusColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	this->setModelIndexFromAspect(cbXErrorMinusColumn, column);
	m_initializing = false;
}
void XYCurveDock::curveYErrorTypeChanged(XYCurve::ErrorType type) {
	m_initializing = true;
	ui.cbYErrorType->setCurrentIndex((int) type);
	m_initializing = false;
}
void XYCurveDock::curveYErrorPlusColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	this->setModelIndexFromAspect(cbYErrorPlusColumn, column);
	m_initializing = false;
}

void XYCurveDock::curveYErrorMinusColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	this->setModelIndexFromAspect(cbYErrorMinusColumn, column);
	m_initializing = false;
}
void XYCurveDock::curveErrorBarsCapSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbErrorBarsCapSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
	m_initializing = false;
}
void XYCurveDock::curveErrorBarsTypeChanged(XYCurve::ErrorBarsType type) {
	m_initializing = true;
	ui.cbErrorBarsType->setCurrentIndex( (int) type);
	m_initializing = false;
}
void XYCurveDock::curveErrorBarsPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbErrorBarsStyle->setCurrentIndex( (int) pen.style());
	ui.kcbErrorBarsColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, pen.color());
	ui.sbErrorBarsWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Point) );
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
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->linePen().widthF(), Worksheet::Point) );
	ui.sbLineOpacity->setValue( round(m_curve->lineOpacity()*100.0) );

	//Drop lines
	ui.cbDropLineType->setCurrentIndex( (int) m_curve->dropLineType() );
	ui.cbDropLineStyle->setCurrentIndex( (int) m_curve->dropLinePen().style() );
	ui.kcbDropLineColor->setColor( m_curve->dropLinePen().color() );
	ui.sbDropLineWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->dropLinePen().widthF(),Worksheet::Point) );
	ui.sbDropLineOpacity->setValue( round(m_curve->dropLineOpacity()*100.0) );

	//Symbols
	ui.cbSymbolStyle->setCurrentIndex( (int)m_curve->symbolsStyle() );
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(m_curve->symbolsSize(), Worksheet::Point) );
	ui.sbSymbolRotation->setValue( m_curve->symbolsRotationAngle() );
	ui.sbSymbolOpacity->setValue( round(m_curve->symbolsOpacity()*100.0) );
	ui.cbSymbolFillingStyle->setCurrentIndex( (int) m_curve->symbolsBrush().style() );
	ui.kcbSymbolFillingColor->setColor(  m_curve->symbolsBrush().color() );
	ui.cbSymbolBorderStyle->setCurrentIndex( (int) m_curve->symbolsPen().style() );
	ui.kcbSymbolBorderColor->setColor( m_curve->symbolsPen().color() );
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->symbolsPen().widthF(), Worksheet::Point) );

	//Values
	ui.cbValuesType->setCurrentIndex( (int) m_curve->valuesType() );
	ui.cbValuesPosition->setCurrentIndex( (int) m_curve->valuesPosition() );
	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(m_curve->valuesDistance(), Worksheet::Point) );
	ui.sbValuesRotation->setValue( m_curve->valuesRotationAngle() );
	ui.sbValuesOpacity->setValue( round(m_curve->valuesOpacity()*100.0) );
	ui.leValuesPrefix->setText( m_curve->valuesPrefix() );
	ui.leValuesSuffix->setText( m_curve->valuesSuffix() );
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF( round(Worksheet::convertFromSceneUnits(valuesFont.pixelSize(), Worksheet::Point)) );
	ui.kfrValuesFont->setFont(valuesFont);
	ui.kcbValuesColor->setColor( m_curve->valuesColor() );

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
	ui.sbErrorBarsCapSize->setValue( Worksheet::convertFromSceneUnits(m_curve->errorBarsCapSize(), Worksheet::Point) );
	ui.cbErrorBarsStyle->setCurrentIndex( (int) m_curve->errorBarsPen().style() );
	ui.kcbErrorBarsColor->setColor( m_curve->errorBarsPen().color() );
	ui.sbErrorBarsWidth->setValue( Worksheet::convertFromSceneUnits(m_curve->errorBarsPen().widthF(),Worksheet::Point) );
	ui.sbErrorBarsOpacity->setValue( round(m_curve->errorBarsOpacity()*100.0) );

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, ui.kcbLineColor->color());
	GuiTools::updatePenStyles(ui.cbDropLineStyle, ui.kcbDropLineColor->color());
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, ui.kcbSymbolFillingColor->color());
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, ui.kcbSymbolBorderColor->color());
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, ui.kcbErrorBarsColor->color());
	m_initializing = false;
}

void XYCurveDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
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
	ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LineWidth", m_curve->linePen().widthF()), Worksheet::Point) );
	ui.sbLineOpacity->setValue( round(group.readEntry("LineOpacity", m_curve->lineOpacity())*100.0) );

	//Drop lines
	ui.cbDropLineType->setCurrentIndex( group.readEntry("DropLineType", (int) m_curve->dropLineType()) );
	ui.cbDropLineStyle->setCurrentIndex( group.readEntry("DropLineStyle", (int) m_curve->dropLinePen().style()) );
	ui.kcbDropLineColor->setColor( group.readEntry("DropLineColor", m_curve->dropLinePen().color()) );
	ui.sbDropLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("DropLineWidth", m_curve->dropLinePen().widthF()),Worksheet::Point) );
	ui.sbDropLineOpacity->setValue( round(group.readEntry("DropLineOpacity", m_curve->dropLineOpacity())*100.0) );

	//Symbols
	ui.cbSymbolStyle->setCurrentIndex( group.readEntry("SymbolStyle", (int)m_curve->symbolsStyle()) );
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("SymbolSize", m_curve->symbolsSize()), Worksheet::Point) );
	ui.sbSymbolRotation->setValue( group.readEntry("SymbolRotation", m_curve->symbolsRotationAngle()) );
	ui.sbSymbolOpacity->setValue( round(group.readEntry("SymbolOpacity", m_curve->symbolsOpacity())*100.0) );
	ui.cbSymbolFillingStyle->setCurrentIndex( group.readEntry("SymbolFillingStyle", (int) m_curve->symbolsBrush().style()) );
	ui.kcbSymbolFillingColor->setColor(  group.readEntry("SymbolFillingColor", m_curve->symbolsBrush().color()) );
	ui.cbSymbolBorderStyle->setCurrentIndex( group.readEntry("SymbolBorderStyle", (int) m_curve->symbolsPen().style()) );
	ui.kcbSymbolBorderColor->setColor( group.readEntry("SymbolBorderColor", m_curve->symbolsPen().color()) );
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("SymbolBorderWidth",m_curve->symbolsPen().widthF()), Worksheet::Point) );

	//Values
	ui.cbValuesType->setCurrentIndex( group.readEntry("ValuesType", (int) m_curve->valuesType()) );
	ui.cbValuesPosition->setCurrentIndex( group.readEntry("ValuesPosition", (int) m_curve->valuesPosition()) );
	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ValuesDistance", m_curve->valuesDistance()), Worksheet::Point) );
	ui.sbValuesRotation->setValue( group.readEntry("ValuesRotation", m_curve->valuesRotationAngle()) );
	ui.sbValuesOpacity->setValue( round(group.readEntry("ValuesOpacity",m_curve->valuesOpacity())*100.0) );
	ui.leValuesPrefix->setText( group.readEntry("ValuesPrefix", m_curve->valuesPrefix()) );
	ui.leValuesSuffix->setText( group.readEntry("ValuesSuffix", m_curve->valuesSuffix()) );
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF( round(Worksheet::convertFromSceneUnits(valuesFont.pixelSize(), Worksheet::Point)) );
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
	ui.sbErrorBarsCapSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ErrorBarsCapSize", m_curve->errorBarsCapSize()), Worksheet::Point) );
	ui.cbErrorBarsStyle->setCurrentIndex( group.readEntry("ErrorBarsStyle", (int) m_curve->errorBarsPen().style()) );
	ui.kcbErrorBarsColor->setColor( group.readEntry("ErrorBarsColor", m_curve->errorBarsPen().color()) );
	ui.sbErrorBarsWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ErrorBarsWidth", m_curve->errorBarsPen().widthF()),Worksheet::Point) );
	ui.sbErrorBarsOpacity->setValue( round(group.readEntry("ErrorBarsOpacity", m_curve->errorBarsOpacity())*100.0) );

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, ui.kcbLineColor->color());
	GuiTools::updatePenStyles(ui.cbDropLineStyle, ui.kcbDropLineColor->color());
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, ui.kcbSymbolFillingColor->color());
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, ui.kcbSymbolBorderColor->color());
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, ui.kcbErrorBarsColor->color());
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, ui.kcbFillingFirstColor->color());
	m_initializing = false;
}

void XYCurveDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "XYCurve" );

	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.

	group.writeEntry("LineType", ui.cbLineType->currentIndex());
	group.writeEntry("LineSkipGaps", ui.chkLineSkipGaps->isChecked());
	group.writeEntry("LineInterpolationPointsCount", ui.sbLineInterpolationPointsCount->value() );
	group.writeEntry("LineStyle", ui.cbLineStyle->currentIndex());
	group.writeEntry("LineColor", ui.kcbLineColor->color());
	group.writeEntry("LineWidth", Worksheet::convertToSceneUnits(ui.sbLineWidth->value(),Worksheet::Point) );
	group.writeEntry("LineOpacity", ui.sbLineOpacity->value()/100 );

	//Drop Line
	group.writeEntry("DropLineType", ui.cbDropLineType->currentIndex());
	group.writeEntry("DropLineStyle", ui.cbDropLineStyle->currentIndex());
	group.writeEntry("DropLineColor", ui.kcbDropLineColor->color());
	group.writeEntry("DropLineWidth", Worksheet::convertToSceneUnits(ui.sbDropLineWidth->value(),Worksheet::Point) );
	group.writeEntry("DropLineOpacity", ui.sbDropLineOpacity->value()/100 );

	//Symbol (TODO: character)
	group.writeEntry("SymbolStyle", ui.cbSymbolStyle->currentIndex());
	group.writeEntry("SymbolSize", Worksheet::convertToSceneUnits(ui.sbSymbolSize->value(),Worksheet::Point));
	group.writeEntry("SymbolRotation", ui.sbSymbolRotation->value());
	group.writeEntry("SymbolOpacity", ui.sbSymbolOpacity->value()/100 );
	group.writeEntry("SymbolFillingStyle", ui.cbSymbolFillingStyle->currentIndex());
	group.writeEntry("SymbolFillingColor", ui.kcbSymbolFillingColor->color());
	group.writeEntry("SymbolBorderStyle", ui.cbSymbolBorderStyle->currentIndex());
	group.writeEntry("SymbolBorderColor", ui.kcbSymbolBorderColor->color());
	group.writeEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(ui.sbSymbolBorderWidth->value(),Worksheet::Point));

	//Values
	group.writeEntry("ValuesType", ui.cbValuesType->currentIndex());
	group.writeEntry("ValuesPosition", ui.cbValuesPosition->currentIndex());
	group.writeEntry("ValuesDistance", Worksheet::convertToSceneUnits(ui.sbValuesDistance->value(),Worksheet::Point));
	group.writeEntry("ValuesRotation", ui.sbValuesRotation->value());
	group.writeEntry("ValuesOpacity", ui.sbValuesOpacity->value()/100);
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
	group.writeEntry("ErrorBarsCapSize", Worksheet::convertToSceneUnits(ui.sbErrorBarsCapSize->value(),Worksheet::Point) );
	group.writeEntry("ErrorBarsStyle", ui.cbErrorBarsStyle->currentIndex());
	group.writeEntry("ErrorBarsColor", ui.kcbErrorBarsColor->color());
	group.writeEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(ui.sbErrorBarsWidth->value(),Worksheet::Point) );
	group.writeEntry("ErrorBarsOpacity", ui.sbErrorBarsOpacity->value()/100 );

	config.sync();
}
