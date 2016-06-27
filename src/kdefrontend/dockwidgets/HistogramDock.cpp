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
#include "backend/core/AspectTreeModel.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include <QPainter>
#include <QDir>
#include <QFileDialog>
#include <KUrlCompletion>
#include <KConfigGroup>
#include <QDebug>

/*!
  \class HistogramDock
  \brief  Provides a widget for editing the properties of the Histograms (2D-curves) currently selected in the project explorer.

  If more then one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

HistogramDock::HistogramDock(QWidget *parent): QWidget(parent),
	m_completion(new KUrlCompletion()),
	cbXColumn(0),
	cbYColumn(0),
	m_curve(0),
	m_aspectTreeModel(0) {

	ui.setupUi(this);

	//Tab "Values"
	QGridLayout* gridLayout = qobject_cast<QGridLayout*>(ui.tabValues->layout());
	cbValuesColumn = new TreeViewComboBox(ui.tabValues);
	gridLayout->addWidget(cbValuesColumn, 2, 2, 1, 1);

	//Tab "Filling"
	ui.cbFillingColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.kleFillingFileName->setClearButtonShown(true);
	ui.bFillingOpen->setIcon( QIcon::fromTheme("document-open") );

	ui.kleFillingFileName->setCompletionObject(m_completion);

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
	connect( ui.kleFillingFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
	connect( ui.kleFillingFileName, SIGNAL(clearButtonClicked()), this, SLOT(fileNameChanged()) );
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
HistogramDock::~HistogramDock()
{
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;

	delete m_completion;

}
void HistogramDock::setModelIndexFromColumn(TreeViewComboBox* cb, const AbstractColumn* column){
	if (column)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(column));
	else
		cb->setCurrentModelIndex(QModelIndex());
}
void HistogramDock::retranslateUi(){
	//TODO:
// 	uiGeneralTab.lName->setText(i18n("Name"));
// 	uiGeneralTab.lComment->setText(i18n("Comment"));
// 	uiGeneralTab.chkVisible->setText(i18n("Visible"));
// 	uiGeneralTab.lXColumn->setText(i18n("x-data"));
// 	uiGeneralTab.lYColumn->setText(i18n("y-data"));

	//TODO updatePenStyles, updateBrushStyles for all comboboxes
}
// "General"-tab
void HistogramDock::nameChanged(){
  if (m_initializing)
	return;

  m_curve->setName(uiGeneralTab.leName->text());
}

void HistogramDock::init(){
  	dateStrings<<"yyyy-MM-dd";
	dateStrings<<"yyyy/MM/dd";
	dateStrings<<"dd/MM/yyyy";
	dateStrings<<"dd/MM/yy";
	dateStrings<<"dd.MM.yyyy";
	dateStrings<<"dd.MM.yy";
	dateStrings<<"MM/yyyy";
	dateStrings<<"dd.MM.";
	dateStrings<<"yyyyMMdd";

	timeStrings<<"hh";
	timeStrings<<"hh ap";
	timeStrings<<"hh:mm";
	timeStrings<<"hh:mm ap";
	timeStrings<<"hh:mm:ss";
	timeStrings<<"hh:mm:ss.zzz";
	timeStrings<<"hh:mm:ss:zzz";
	timeStrings<<"mm:ss.zzz";
	timeStrings<<"hhmmss";

	//Values
	ui.cbValuesType->addItem(i18n("no values"));
	ui.cbValuesType->addItem("x");
	ui.cbValuesType->addItem("y");
	ui.cbValuesType->addItem("x, y");
	ui.cbValuesType->addItem("(x, y)");
	ui.cbValuesType->addItem(i18n("custom column"));

	ui.cbValuesPosition->addItem(i18n("above"));
	ui.cbValuesPosition->addItem(i18n("below"));
	ui.cbValuesPosition->addItem(i18n("left"));
	ui.cbValuesPosition->addItem(i18n("right"));

	//Filling
	ui.cbFillingPosition->clear();
	ui.cbFillingPosition->addItem(i18n("none"));
	ui.cbFillingPosition->addItem(i18n("above"));
	ui.cbFillingPosition->addItem(i18n("below"));
	ui.cbFillingPosition->addItem(i18n("zero baseline"));
	ui.cbFillingPosition->addItem(i18n("left"));
	ui.cbFillingPosition->addItem(i18n("right"));

	ui.cbFillingType->clear();
	ui.cbFillingType->addItem(i18n("color"));
	ui.cbFillingType->addItem(i18n("image"));
	ui.cbFillingType->addItem(i18n("pattern"));

	ui.cbFillingColorStyle->clear();
	ui.cbFillingColorStyle->addItem(i18n("single color"));
	ui.cbFillingColorStyle->addItem(i18n("horizontal linear gradient"));
	ui.cbFillingColorStyle->addItem(i18n("vertical linear gradient"));
	ui.cbFillingColorStyle->addItem(i18n("diagonal linear gradient (start from top left)"));
	ui.cbFillingColorStyle->addItem(i18n("diagonal linear gradient (start from bottom left)"));
	ui.cbFillingColorStyle->addItem(i18n("radial gradient"));

	ui.cbFillingImageStyle->clear();
	ui.cbFillingImageStyle->addItem(i18n("scaled and cropped"));
	ui.cbFillingImageStyle->addItem(i18n("scaled"));
	ui.cbFillingImageStyle->addItem(i18n("scaled, keep proportions"));
	ui.cbFillingImageStyle->addItem(i18n("centered"));
	ui.cbFillingImageStyle->addItem(i18n("tiled"));
	ui.cbFillingImageStyle->addItem(i18n("center tiled"));
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, Qt::SolidPattern);

}
void HistogramDock::setModel() {
	QList<const char*>  list;
	list<<"Folder"<<"Workbook"<<"Datapicker"<<"DatapickerCurve"<<"Spreadsheet"
		<<"FileDataSource"<<"Column"<<"Worksheet"<<"CartesianPlot"<< "Histogram"
		<<"XYInterpolationCurve"<<"XYFitCurve"<<"XYFourierFilterCurve";

	if (cbXColumn) {
		cbXColumn->setTopLevelClasses(list);
		cbYColumn->setTopLevelClasses(list);
	}
	cbValuesColumn->setTopLevelClasses(list);

 	list.clear();
	list<<"Column";
	m_aspectTreeModel->setSelectableAspects(list);
	if (cbXColumn) {
		cbXColumn->setSelectableClasses(list);
		cbYColumn->setSelectableClasses(list);
	}
	cbValuesColumn->setSelectableClasses(list);
	if (cbXColumn) {
		cbXColumn->setModel(m_aspectTreeModel);
		cbYColumn->setModel(m_aspectTreeModel);
	}
	cbValuesColumn->setModel(m_aspectTreeModel);
}
void HistogramDock::setCurves(QList<Histogram*> list){
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	Q_ASSERT(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	setModel();
	initGeneralTab();
	initTabs();
	m_initializing=false;
}
void HistogramDock::initGeneralTab(){
	//if there are more then one curve in the list, disable the content in the tab "general"
	if (m_curvesList.size()==1){
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.leComment->setEnabled(true);

		uiGeneralTab.lXColumn->setEnabled(true);
		cbXColumn->setEnabled(true);
		uiGeneralTab.lYColumn->setEnabled(true);
		cbYColumn->setEnabled(true);

		this->setModelIndexFromColumn(cbXColumn, m_curve->xColumn());
		this->setModelIndexFromColumn(cbYColumn, m_curve->yColumn());

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.leComment->setText(m_curve->comment());
	}else {
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

		uiGeneralTab.leName->setText("");
		uiGeneralTab.leComment->setText("");
	}

	//show the properties of the first curve
	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );
	
	connect( uiGeneralTab.cbHistogramType, SIGNAL(currentIndexChanged(int)), this, SLOT(histogramTypeChanged(int)) );
	

	//Slots
	connect(m_curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	
	connect(m_curve, SIGNAL(xColumnChanged(const AbstractColumn*)), this, SLOT(curveXColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(yColumnChanged(const AbstractColumn*)), this, SLOT(curveYColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(visibilityChanged(bool)), this, SLOT(curveVisibilityChanged(bool)));
	//types options
	uiGeneralTab.cbHistogramType->addItem(i18n("Ordinary Histogram"));
	uiGeneralTab.cbHistogramType->addItem(i18n("Cummulative Histogram"));
	uiGeneralTab.cbHistogramType->addItem(i18n("AvgShifted Histogram"));
	
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
void HistogramDock::curveValuesPrefixChanged(QString prefix) {
	m_initializing = true;
  	ui.leValuesPrefix->setText(prefix);
	m_initializing = false;
}
void HistogramDock::curveValuesSuffixChanged(QString suffix) {
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
	ui.kleFillingFileName->setText(filename);
	m_initializing = false;
}
void HistogramDock::curveFillingOpacityChanged(float opacity){
	m_initializing = true;
	ui.sbFillingOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}
void HistogramDock::initTabs() {
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size()==1){
		/*this->setModelIndexFromColumn(cbValuesColumn, m_curve->valuesColumn());
	*/}else {
		cbValuesColumn->setCurrentModelIndex(QModelIndex());
	}

	//show the properties of the first curve
	KConfig config("", KConfig::SimpleConfig);
	loadConfig(config);

	//Slots
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
	ui.kleFillingFileName->setEnabled(b);
	ui.bFillingOpen->setEnabled(b);
	ui.sbFillingOpacity->setEnabled(b);

	if (m_initializing)
		return;

	foreach(Histogram* curve, m_curvesList)
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
		ui.kleFillingFileName->hide();
		ui.bFillingOpen->hide();

		ui.lFillingFirstColor->show();
		ui.kcbFillingFirstColor->show();

		PlotArea::BackgroundColorStyle style =
			(PlotArea::BackgroundColorStyle) ui.cbFillingColorStyle->currentIndex();
		if (style == PlotArea::SingleColor){
			ui.lFillingFirstColor->setText(i18n("Color"));
			ui.lFillingSecondColor->hide();
			ui.kcbFillingSecondColor->hide();
		}else{
			ui.lFillingFirstColor->setText(i18n("First Color"));
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
		ui.kleFillingFileName->show();
		ui.bFillingOpen->show();

		ui.lFillingFirstColor->hide();
		ui.kcbFillingFirstColor->hide();
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();
	}else if(type == PlotArea::Pattern) {
		ui.lFillingFirstColor->setText(i18n("Color"));
		ui.lFillingColorStyle->hide();
		ui.cbFillingColorStyle->hide();
		ui.lFillingImageStyle->hide();
		ui.cbFillingImageStyle->hide();
		ui.lFillingBrushStyle->show();
		ui.cbFillingBrushStyle->show();
		ui.lFillingFileName->hide();
		ui.kleFillingFileName->hide();
		ui.bFillingOpen->hide();

		ui.lFillingFirstColor->show();
		ui.kcbFillingFirstColor->show();
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();
	}

	if (m_initializing)
		return;

	foreach(Histogram* curve, m_curvesList)
		curve->setFillingType(type);
}

void HistogramDock::fillingColorStyleChanged(int index){
	PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor){
		ui.lFillingFirstColor->setText(i18n("Color"));
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();
	}else{
		ui.lFillingFirstColor->setText(i18n("First Color"));
		ui.lFillingSecondColor->show();
		ui.kcbFillingSecondColor->show();
		ui.lFillingBrushStyle->hide();
		ui.cbFillingBrushStyle->hide();
	}

	if (m_initializing)
		return;

	foreach(Histogram* curve, m_curvesList)
		curve->setFillingColorStyle(style);
}

void HistogramDock::fillingImageStyleChanged(int index){
	if (m_initializing)
		return;

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	foreach(Histogram* curve, m_curvesList)
		curve->setFillingImageStyle(style);
}

void HistogramDock::fillingBrushStyleChanged(int index){
	if (m_initializing)
		return;

	Qt::BrushStyle style = (Qt::BrushStyle)index;
	foreach(Histogram* curve, m_curvesList)
		curve->setFillingBrushStyle(style);
}

void HistogramDock::fillingFirstColorChanged(const QColor& c){
	if (m_initializing)
		return;

	foreach(Histogram* curve, m_curvesList)
		curve->setFillingFirstColor(c);
}

void HistogramDock::fillingSecondColorChanged(const QColor& c){
	if (m_initializing)
		return;

	foreach(Histogram* curve, m_curvesList)
		curve->setFillingSecondColor(c);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void HistogramDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "HistogramDock");
	QString dir = conf.readEntry("LastImageDir", "");
    QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir);
    if (path.isEmpty())
        return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos!=-1) {
		QString newDir = path.left(pos);
		if (newDir!=dir)
			conf.writeEntry("LastImageDir", newDir);
	}

    ui.kleFillingFileName->setText( path );

	foreach(Histogram* curve, m_curvesList)
		curve->setFillingFileName(path);
}

void HistogramDock::fileNameChanged(){
	if (m_initializing)
		return;

	QString fileName = ui.kleFillingFileName->text();
	foreach(Histogram* curve, m_curvesList)
		curve->setFillingFileName(fileName);
}

void HistogramDock::fillingOpacityChanged(int value){
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	foreach(Histogram* curve, m_curvesList)
		curve->setFillingOpacity(opacity);
}
void HistogramDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "Histogram" );

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
	ui.kleFillingFileName->setText( group.readEntry("FillingFileName", m_curve->fillingFileName()) );
	ui.kcbFillingFirstColor->setColor( group.readEntry("FillingFirstColor", m_curve->fillingFirstColor()) );
	ui.kcbFillingSecondColor->setColor( group.readEntry("FillingSecondColor", m_curve->fillingSecondColor()) );
	ui.sbFillingOpacity->setValue( round(group.readEntry("FillingOpacity", m_curve->fillingOpacity())*100.0) );

	m_initializing=true;
	m_initializing=false;
}
void HistogramDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	// Tab "General"
	QGridLayout* gridLayout = qobject_cast<QGridLayout*>(generalTab->layout());

	cbXColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXColumn, 2, 2, 1, 1);

	cbYColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYColumn, 3, 2, 1, 1);

	//General
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	qDebug() << "colo";
	connect( cbXColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xColumnChanged(QModelIndex)) );
	qDebug() << "colo1";
	connect( cbYColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yColumnChanged(QModelIndex)) );

}

void HistogramDock::xColumnChanged(const QModelIndex& index) {
	
	qDebug() << "here";
	if (m_initializing)
		return;
	
	qDebug() << "here";

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		qDebug() << "here as[ect";
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(Histogram* curve, m_curvesList) {
		qDebug() << "here as well";
		curve->setXColumn(column);
	}
}

void HistogramDock::yColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(Histogram* curve, m_curvesList)
		curve->setYColumn(column);
}