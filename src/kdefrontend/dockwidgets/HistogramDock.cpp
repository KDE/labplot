/***************************************************************************
    File                 : HistogramDock.cpp
    Project              : LabPlot
    Description          : widget for Histogram properties
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Anu Mittal (anu22mittal@gmail.com)

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
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include <QCompleter>
#include <QDir>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>
#include <QPainter>

#include <KConfigGroup>
#include <KConfig>
#include <KLocalizedString>
#include <KSharedConfig>

/*!
  \class HistogramDock
  \brief  Provides a widget for editing the properties of the Histograms (2D-curves) currently selected in the project explorer.

  If more than one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/
HistogramDock::HistogramDock(QWidget* parent) : QWidget(parent),
	cbXColumn(nullptr),
	m_curve(nullptr),
	m_aspectTreeModel(nullptr),
	m_initializing(false) {

	ui.setupUi(this);

	// Tab "General"
	QGridLayout* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	cbXColumn = new TreeViewComboBox(ui.tabGeneral);
	gridLayout->addWidget(cbXColumn, 3, 2, 1, 1);

	//Tab "Values"
	gridLayout = qobject_cast<QGridLayout*>(ui.tabValues->layout());
	cbValuesColumn = new TreeViewComboBox(ui.tabValues);
	gridLayout->addWidget(cbValuesColumn, 2, 2, 1, 1);

	//Tab "Filling"
	ui.cbFillingColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.bFillingOpen->setIcon( QIcon::fromTheme("document-open") );

	QCompleter* completer = new QCompleter(this);
	completer->setModel(new QDirModel);
	ui.leFillingFileName->setCompleter(completer);

	//adjust layouts in the tabs
	for (int i=0; i<ui.tabWidget->count(); ++i){
		QGridLayout* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	//Slots
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &HistogramDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &HistogramDock::commentChanged);
	connect( ui.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( cbXColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xColumnChanged(QModelIndex)) );
	connect( ui.cbHistogramType, SIGNAL(currentIndexChanged(int)), this, SLOT(histogramTypeChanged(int)) );
	connect( ui.cbHistogramOrientation, SIGNAL(currentIndexChanged(int)), this, SLOT(histogramOrientationChanged(int)));
	connect( ui.cbBins, SIGNAL(currentIndexChanged(int)), this, SLOT(binsOptionChanged(int)) );
	connect( ui.sbBins, SIGNAL(valueChanged(int)), this, SLOT(binValueChanged(int)) );

	//Line
	connect( ui.kcbLineColor, SIGNAL(changed(QColor)), this, SLOT(lineColorChanged(QColor)) );

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
	connect( ui.bFillingOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect( ui.leFillingFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
	connect( ui.leFillingFileName, SIGNAL(textChanged(QString)), this, SLOT(fileNameChanged()) );
	connect( ui.kcbFillingFirstColor, SIGNAL(changed(QColor)), this, SLOT(fillingFirstColorChanged(QColor)) );
	connect( ui.kcbFillingSecondColor, SIGNAL(changed(QColor)), this, SLOT(fillingSecondColorChanged(QColor)) );
	connect( ui.sbFillingOpacity, SIGNAL(valueChanged(int)), this, SLOT(fillingOpacityChanged(int)) );


	//template handler
	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Histogram);
	ui.verticalLayout->addWidget(templateHandler);
	templateHandler->show();
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));

	retranslateUi();
	init();
}

HistogramDock::~HistogramDock() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;
}

//TODO: very similiar to ColumnDock
void HistogramDock::updateValuesFormatWidgets(const AbstractColumn::ColumnMode columnMode) {
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
	case AbstractColumn::DateTime:
		for (const auto& s: AbstractColumn::dateFormats())
			ui.cbValuesFormat->addItem(s, QVariant(s));

		for (const auto& s: AbstractColumn::timeFormats())
			ui.cbValuesFormat->addItem(s, QVariant(s));

		for (const auto& s1: AbstractColumn::dateFormats())
			for (const auto& s2: AbstractColumn::timeFormats())
				ui.cbValuesFormat->addItem(s1 + ' ' + s2, QVariant(s1 + ' ' + s2));
		break;
	}

	ui.cbValuesFormat->setCurrentIndex(0);

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
		ui.cbValuesFormat->setCurrentIndex(0);
	}

	if (columnMode == AbstractColumn::DateTime) {
		ui.cbValuesFormat->setEditable(true);
	} else {
		ui.cbValuesFormat->setEditable(false);
	}
}

//TODO: very similiar to ColumnDock
void HistogramDock::showValuesColumnFormat(const Column* column){
  if (!column){
	// no valid column is available
	// -> hide all the format properties widgets (equivalent to showing the properties of the column mode "Text")
	this->updateValuesFormatWidgets(AbstractColumn::Text);
  }else{
	AbstractColumn::ColumnMode columnMode = column->columnMode();

	//update the format widgets for the new column mode
	this->updateValuesFormatWidgets(columnMode);

	 //show the actuall formating properties
	switch(columnMode) {
		case AbstractColumn::Numeric:{
		  Double2StringFilter * filter = static_cast<Double2StringFilter*>(column->outputFilter());
		  ui.cbValuesFormat->setCurrentIndex(ui.cbValuesFormat->findData(filter->numericFormat()));
		  ui.sbValuesPrecision->setValue(filter->numDigits());
		  break;
		}
		case AbstractColumn::Text:
		case AbstractColumn::Integer:
			break;
		case AbstractColumn::Month:
		case AbstractColumn::Day:
		case AbstractColumn::DateTime: {
				DateTime2StringFilter * filter = static_cast<DateTime2StringFilter*>(column->outputFilter());
				ui.cbValuesFormat->setCurrentIndex(ui.cbValuesFormat->findData(filter->format()));
				break;
			}
	}
  }
}
void HistogramDock::setModelIndexFromColumn(TreeViewComboBox* cb, const AbstractColumn* column) {
	if (column)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(column));
	else
		cb->setCurrentModelIndex(QModelIndex());
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
// "General"-tab
void HistogramDock::nameChanged() {
  if (m_initializing)
	return;

  m_curve->setName(ui.leName->text());
}
void HistogramDock::commentChanged() {
  if (m_initializing)
	return;

  m_curve->setComment(ui.leComment->text());
}

void HistogramDock::visibilityChanged(bool state){
	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		curve->setVisible(state);
}
void HistogramDock::valuesColorChanged(const QColor& color){
	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		curve->setValuesColor(color);
}
void HistogramDock::init(){
	//General
	//bins option
	ui.cbBins->addItem(i18n("By Number"));
	ui.cbBins->addItem(i18n("By width"));
	ui.cbBins->addItem(i18n("Square-root rule"));
	ui.cbBins->addItem(i18n("Rice rule"));
	ui.cbBins->addItem(i18n("Sturgis rule"));

	//histogram type
	ui.cbHistogramType->addItem(i18n("Ordinary Histogram"));
	ui.cbHistogramType->addItem(i18n("Cumulative Histogram"));
	ui.cbHistogramType->addItem(i18n("AvgShifted Histogram"));

	//Orientation
	ui.cbHistogramOrientation->addItem(i18n("Vertical"));
	ui.cbHistogramOrientation->addItem(i18n("Horizontal"));

	//Line
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

	QPainter pa;
	//TODO size of the icon depending on the actuall height of the combobox?
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	ui.cbSymbolStyle->setIconSize(QSize(iconSize, iconSize));
	QTransform trafo;
	trafo.scale(15, 15);

	QPen pen(Qt::SolidPattern, 0);
	const QColor& color = (palette().color(QPalette::Base).lightness() < 128) ? Qt::white : Qt::black;
	pen.setColor(color);
	pa.setPen( pen );

	ui.cbSymbolStyle->addItem(i18n("None"));
	for (int i = 1; i < 19; ++i) {	//TODO: use enum count
		Symbol::Style style = (Symbol::Style)i;
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
	ui.cbValuesType->addItem("y");
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

	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, Qt::black);
}

void HistogramDock::setModel() {
	QList<const char*>  list;
	list<<"Folder"<<"Workbook"<<"Datapicker"<<"DatapickerCurve"<<"Spreadsheet"
		<<"FileDataSource"<<"Column"<<"Worksheet"<<"CartesianPlot"<< "Histogram"
		<<"XYInterpolationCurve"<<"XYFitCurve"<<"XYFourierFilterCurve";

	if (cbXColumn) {
		cbXColumn->setTopLevelClasses(list);
	}
	cbValuesColumn->setTopLevelClasses(list);

 	list.clear();
	list<<"Column";
	m_aspectTreeModel->setSelectableAspects(list);
	if (cbXColumn)
		cbXColumn->setModel(m_aspectTreeModel);

	cbValuesColumn->setModel(m_aspectTreeModel);
}
void HistogramDock::setCurves(QList<Histogram*> list){
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	Q_ASSERT(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	setModel();

	//if there are more then one curve in the list, disable the content in the tab "general"
	if (m_curvesList.size()==1){
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.lXColumn->setEnabled(true);
		cbXColumn->setEnabled(true);

		this->setModelIndexFromColumn(cbXColumn, m_curve->xColumn());
		this->setModelIndexFromColumn(cbValuesColumn, m_curve->valuesColumn());

		ui.leName->setText(m_curve->name());
		ui.leComment->setText(m_curve->comment());
	}else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.lXColumn->setEnabled(false);
		cbXColumn->setEnabled(false);

		cbXColumn->setCurrentModelIndex(QModelIndex());
		cbValuesColumn->setCurrentModelIndex(QModelIndex());

		ui.leName->setText("");
		ui.leComment->setText("");
	}

	//show the properties of the first curve
	const Histogram::HistogramData& data = m_curve->histogramData();
	ui.cbHistogramType->setCurrentIndex(data.type);
	ui.cbBins->setCurrentIndex(data.binsOption);
	ui.chkVisible->setChecked( m_curve->isVisible() );

	KConfig config("", KConfig::SimpleConfig);
	loadConfig(config);

	//Slots
	//General-tab
	connect(m_curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_curve, SIGNAL(histogramDataChanged(Histogram::HistogramData)), this, SLOT(curveHistogramDataChanged(Histogram::HistogramData)));
	connect(m_curve, SIGNAL(visibilityChanged(bool)), this, SLOT(curveVisibilityChanged(bool)));

	//Line-tab
	connect(m_curve, SIGNAL(linePenChanged(QPen)), this, SLOT(curveLinePenChanged(QPen)));

	//Values-Tab
	connect(m_curve, SIGNAL(valuesTypeChanged(Histogram::ValuesType)), this, SLOT(curveValuesTypeChanged(Histogram::ValuesType)));
	connect(m_curve, SIGNAL(valuesColumnChanged(const AbstractColumn*)), this, SLOT(curveValuesColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(valuesPositionChanged(Histogram::ValuesPosition)), this, SLOT(curveValuesPositionChanged(Histogram::ValuesPosition)));
	connect(m_curve, SIGNAL(valuesDistanceChanged(qreal)), this, SLOT(curveValuesDistanceChanged(qreal)));
	connect(m_curve, SIGNAL(valuesOpacityChanged(qreal)), this, SLOT(curveValuesOpacityChanged(qreal)));
	connect(m_curve, SIGNAL(valuesRotationAngleChanged(qreal)), this, SLOT(curveValuesRotationAngleChanged(qreal)));
	connect(m_curve, SIGNAL(valuesPrefixChanged(QString)), this, SLOT(curveValuesPrefixChanged(QString)));
	connect(m_curve, SIGNAL(valuesSuffixChanged(QString)), this, SLOT(curveValuesSuffixChanged(QString)));
	connect(m_curve, SIGNAL(valuesFontChanged(QFont)), this, SLOT(curveValuesFontChanged(QFont)));
	connect(m_curve, SIGNAL(valuesColorChanged(QColor)), this, SLOT(curveValuesColorChanged(QColor)));

	//Filling-Tab
	connect( m_curve, SIGNAL(fillingPositionChanged(Histogram::FillingPosition)), this, SLOT(curveFillingPositionChanged(Histogram::FillingPosition)) );
	connect( m_curve, SIGNAL(fillingTypeChanged(PlotArea::BackgroundType)), this, SLOT(curveFillingTypeChanged(PlotArea::BackgroundType)) );
	connect( m_curve, SIGNAL(fillingColorStyleChanged(PlotArea::BackgroundColorStyle)), this, SLOT(curveFillingColorStyleChanged(PlotArea::BackgroundColorStyle)) );
	connect( m_curve, SIGNAL(fillingImageStyleChanged(PlotArea::BackgroundImageStyle)), this, SLOT(curveFillingImageStyleChanged(PlotArea::BackgroundImageStyle)) );
	connect( m_curve, SIGNAL(fillingBrushStyleChanged(Qt::BrushStyle)), this, SLOT(curveFillingBrushStyleChanged(Qt::BrushStyle)) );
	connect( m_curve, SIGNAL(fillingFirstColorChanged(QColor&)), this, SLOT(curveFillingFirstColorChanged(QColor&)) );
	connect( m_curve, SIGNAL(fillingSecondColorChanged(QColor&)), this, SLOT(curveFillingSecondColorChanged(QColor&)) );
	connect( m_curve, SIGNAL(fillingFileNameChanged(QString&)), this, SLOT(curveFillingFileNameChanged(QString&)) );
	connect( m_curve, SIGNAL(fillingOpacityChanged(float)), this, SLOT(curveFillingOpacityChanged(float)) );

	m_initializing=false;
}

//*************************************************************
//**** SLOTs for changes triggered in HistogramDock *****
//*************************************************************

void HistogramDock::curveLinePenChanged(const QPen& pen) {
	m_initializing = true;
	ui.kcbLineColor->setColor( pen.color());
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
	this->setModelIndexFromColumn(cbValuesColumn, column);
	m_initializing = false;
}
void HistogramDock::curveValuesPositionChanged(Histogram::ValuesPosition position) {
	m_initializing = true;
  	ui.cbValuesPosition->setCurrentIndex((int) position);
	m_initializing = false;
}
void HistogramDock::curveValuesDistanceChanged(qreal distance) {
	m_initializing = true;
  	ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(distance, Worksheet::Point) );
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
	font.setPointSizeF( round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Point)) );
  	ui.kfrValuesFont->setFont(font);
	m_initializing = false;
}
void HistogramDock::curveValuesColorChanged(QColor color) {
	m_initializing = true;
  	ui.kcbValuesColor->setColor(color);
	m_initializing = false;
}
/*!
  called when the type of the values (none, x, y, (x,y) etc.) was changed.
*/
void HistogramDock::valuesTypeChanged(int index) {
	Histogram::ValuesType valuesType = Histogram::ValuesType(index);

	if (valuesType == Histogram::NoValues){
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
		if (valuesType == Histogram::ValuesCustomColumn) {
			ui.lValuesColumn->show();
			cbValuesColumn->show();

			column= static_cast<Column*>(cbValuesColumn->currentModelIndex().internalPointer());
		} else {
			ui.lValuesColumn->hide();
			cbValuesColumn->hide();
			column = static_cast<const Column*>(m_curve->xColumn());
		}
		this->showValuesColumnFormat(column);
	}

	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		curve->setValuesType(valuesType);
}

/*!
  called when the custom column for the values was changed.
*/
void HistogramDock::valuesColumnChanged(const QModelIndex& index){
	if (m_initializing)
		return;

	Column* column = static_cast<Column*>(index.internalPointer());
	this->showValuesColumnFormat(column);

	for (auto* curve: m_curvesList) {
	//TODO save also the format of the currently selected column for the values (precision etc.)
		curve->setValuesColumn(column);
	}
}
void HistogramDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}


void HistogramDock::valuesPositionChanged(int index){
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesPosition(Histogram::ValuesPosition(index));
}

void HistogramDock::valuesDistanceChanged(double  value){
	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		curve->setValuesDistance( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void HistogramDock::valuesRotationChanged(int value){
	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		curve->setValuesRotationAngle(value);
}

void HistogramDock::valuesOpacityChanged(int value){
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve: m_curvesList)
		curve->setValuesOpacity(opacity);
}

void HistogramDock::valuesPrefixChanged(){
	if (m_initializing)
		return;

	QString prefix = ui.leValuesPrefix->text();
	for (auto* curve: m_curvesList)
		curve->setValuesPrefix(prefix);
}

void HistogramDock::valuesSuffixChanged(){
	if (m_initializing)
		return;

	QString suffix = ui.leValuesSuffix->text();
	for (auto* curve: m_curvesList)
		curve->setValuesSuffix(suffix);
}

void HistogramDock::valuesFontChanged(const QFont& font){
	if (m_initializing)
		return;

	QFont valuesFont = font;
	valuesFont.setPixelSize( Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Point) );
	for (auto* curve: m_curvesList)
		curve->setValuesFont(valuesFont);
}

//Filling
void HistogramDock::curveFillingPositionChanged(Histogram::FillingPosition position) {
	m_initializing = true;
	ui.cbFillingPosition->setCurrentIndex((int)position);
	m_initializing = false;
}
void HistogramDock::curveFillingTypeChanged(PlotArea::BackgroundType type){
	m_initializing = true;
	ui.cbFillingType->setCurrentIndex(type);
	m_initializing = false;
}
void HistogramDock::curveFillingColorStyleChanged(PlotArea::BackgroundColorStyle style){
	m_initializing = true;
	ui.cbFillingColorStyle->setCurrentIndex(style);
	m_initializing = false;
}
void HistogramDock::curveFillingImageStyleChanged(PlotArea::BackgroundImageStyle style){
	m_initializing = true;
	ui.cbFillingImageStyle->setCurrentIndex(style);
	m_initializing = false;
}
void HistogramDock::curveFillingBrushStyleChanged(Qt::BrushStyle style){
	m_initializing = true;
	ui.cbFillingBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}
void HistogramDock::curveFillingFirstColorChanged(QColor& color){
	m_initializing = true;
	ui.kcbFillingFirstColor->setColor(color);
	m_initializing = false;
}
void HistogramDock::curveFillingSecondColorChanged(QColor& color){
	m_initializing = true;
	ui.kcbFillingSecondColor->setColor(color);
	m_initializing = false;
}
void HistogramDock::curveFillingFileNameChanged(QString& filename){
	m_initializing = true;
	ui.leFillingFileName->setText(filename);
	m_initializing = false;
}
void HistogramDock::curveFillingOpacityChanged(float opacity){
	m_initializing = true;
	ui.sbFillingOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

//Filling-tab
void HistogramDock::fillingPositionChanged(int index){
	Histogram::FillingPosition fillingPosition = Histogram::FillingPosition(index);

	bool b = (fillingPosition != Histogram::NoFilling);
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

	for (auto* curve: m_curvesList)
		curve->setFillingPosition(fillingPosition);
}

void HistogramDock::fillingTypeChanged(int index){
	PlotArea::BackgroundType type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::Color){
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

		PlotArea::BackgroundColorStyle style =
			(PlotArea::BackgroundColorStyle) ui.cbFillingColorStyle->currentIndex();
		if (style == PlotArea::SingleColor){
			ui.lFillingFirstColor->setText(i18n("Color:"));
			ui.lFillingSecondColor->hide();
			ui.kcbFillingSecondColor->hide();
		}else{
			ui.lFillingFirstColor->setText(i18n("First color:"));
			ui.lFillingSecondColor->show();
			ui.kcbFillingSecondColor->show();
		}
	}else if(type == PlotArea::Image){
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
	}else if(type == PlotArea::Pattern) {
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

	for (auto* curve: m_curvesList)
		curve->setFillingType(type);
}

void HistogramDock::fillingColorStyleChanged(int index){
	PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor){
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

	for (auto* curve: m_curvesList)
		curve->setFillingColorStyle(style);
}

void HistogramDock::fillingImageStyleChanged(int index){
	if (m_initializing)
		return;

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	for (auto* curve: m_curvesList)
		curve->setFillingImageStyle(style);
}

void HistogramDock::fillingBrushStyleChanged(int index){
	if (m_initializing)
		return;

	Qt::BrushStyle style = (Qt::BrushStyle)index;
	for (auto* curve: m_curvesList)
		curve->setFillingBrushStyle(style);
}

void HistogramDock::fillingFirstColorChanged(const QColor& c){
	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		curve->setFillingFirstColor(c);
}

void HistogramDock::fillingSecondColorChanged(const QColor& c){
	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		curve->setFillingSecondColor(c);
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
		formats.isEmpty() ? formats+=f : formats+=' '+f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
        	return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir!=dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leFillingFileName->setText( path );

	for (auto* curve: m_curvesList)
		curve->setFillingFileName(path);
}

void HistogramDock::fileNameChanged(){
	if (m_initializing)
		return;

	QString fileName = ui.leFillingFileName->text();
	for (auto* curve: m_curvesList)
		curve->setFillingFileName(fileName);
}

void HistogramDock::fillingOpacityChanged(int value){
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* curve: m_curvesList)
		curve->setFillingOpacity(opacity);
}

void HistogramDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("Histogram"));

  	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.
	//This data is read in HistogramDock::setCurves().

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
}

void HistogramDock::histogramTypeChanged(int index) {
	if (m_initializing)
		return;

	Histogram::HistogramType histogramType = Histogram::HistogramType(index);
	for (auto* curve : m_curvesList)
		curve->setHistogramType(histogramType);
}

void HistogramDock::histogramOrientationChanged(int index) {
	if (m_initializing)
		return;

	Histogram::HistogramOrientation histogramOrientation = Histogram::HistogramOrientation(index);
	for (auto* curve : m_curvesList)
		curve->setHistogramOrientation(histogramOrientation);
}

void HistogramDock::binValueChanged(int value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setBinValue(value);
}

void HistogramDock::binsOptionChanged(int index) {
	if (m_initializing)
		return;

	Histogram::BinsOption binsOption = Histogram::BinsOption(index);
	for (auto* curve : m_curvesList)
		curve->setbinsOption(binsOption);
}

void HistogramDock::lineColorChanged(const QColor& color){
	if (m_initializing)
		return;

	QPen pen;
	for (auto* curve: m_curvesList) {
		pen = curve->linePen();
		pen.setColor(color);
		curve->setLinePen(pen);
  	}
}
void HistogramDock::xColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_curvesList)
		curve->setXColumn(column);
}

void HistogramDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index!=-1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_curvesList.size();
	if (size>1)
		m_curve->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_curve->beginMacro(i18n("%1: template \"%2\" loaded", m_curve->name(), name));

	this->loadConfig(config);

	m_curve->endMacro();
}
void HistogramDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "Histogram" );
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

	config.sync();
}

//*************************************************************
//*********** SLOTs for changes triggered in Histogram **********
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

void HistogramDock::curveHistogramDataChanged(Histogram::HistogramData data) {
	m_initializing = true;
	ui.cbHistogramType->setCurrentIndex(data.type);
	ui.cbBins->setCurrentIndex(data.binsOption);
	ui.sbBins->setValue(data.binValue);
	m_initializing = false;
}
