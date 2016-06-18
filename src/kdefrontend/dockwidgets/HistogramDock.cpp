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

HistogramDock::HistogramDock(QWidget *parent): QWidget(parent) {

	ui.setupUi(this);
}

HistogramDock::~HistogramDock()
{

}
void HistogramDock::setCurves(QList<Histogram*> list){
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	Q_ASSERT(m_curve);
	//m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	//setModel();
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

		//this->setModelIndexFromColumn(cbXColumn, m_curve->xColumn());
		//this->setModelIndexFromColumn(cbYColumn, m_curve->yColumn());

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

	//Slots
	connect(m_curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_curve, SIGNAL(xColumnChanged(const AbstractColumn*)), this, SLOT(curveXColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(yColumnChanged(const AbstractColumn*)), this, SLOT(curveYColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(visibilityChanged(bool)), this, SLOT(curveVisibilityChanged(bool)));
}

void HistogramDock::initTabs() {
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size()==1){
		/*this->setModelIndexFromColumn(cbValuesColumn, m_curve->valuesColumn());
		this->setModelIndexFromColumn(cbXErrorPlusColumn, m_curve->xErrorPlusColumn());
		this->setModelIndexFromColumn(cbXErrorMinusColumn, m_curve->xErrorMinusColumn());
		this->setModelIndexFromColumn(cbYErrorPlusColumn, m_curve->yErrorPlusColumn());
		this->setModelIndexFromColumn(cbYErrorMinusColumn, m_curve->yErrorMinusColumn());
	*/}else {
		cbValuesColumn->setCurrentModelIndex(QModelIndex());
		cbXErrorPlusColumn->setCurrentModelIndex(QModelIndex());
		cbXErrorMinusColumn->setCurrentModelIndex(QModelIndex());
		cbYErrorPlusColumn->setCurrentModelIndex(QModelIndex());
		cbYErrorMinusColumn->setCurrentModelIndex(QModelIndex());
	}

	//show the properties of the first curve
	KConfig config("", KConfig::SimpleConfig);
	loadConfig(config);

	//Slots

	//Line-Tab
	connect(m_curve, SIGNAL(lineTypeChanged(Histogram::LineType)), this, SLOT(curveLineTypeChanged(Histogram::LineType)));
	connect(m_curve, SIGNAL(lineSkipGapsChanged(bool)), this, SLOT(curveLineSkipGapsChanged(bool)));
	connect(m_curve, SIGNAL(lineInterpolationPointsCountChanged(int)), this, SLOT(curveLineInterpolationPointsCountChanged(int)));
	connect(m_curve, SIGNAL(linePenChanged(QPen)), this, SLOT(curveLinePenChanged(QPen)));
	connect(m_curve, SIGNAL(lineOpacityChanged(qreal)), this, SLOT(curveLineOpacityChanged(qreal)));
	connect(m_curve, SIGNAL(dropLineTypeChanged(Histogram::DropLineType)), this, SLOT(curveDropLineTypeChanged(Histogram::DropLineType)));
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

	//"Error bars"-Tab
	connect(m_curve, SIGNAL(xErrorTypeChanged(Histogram::ErrorType)), this, SLOT(curveXErrorTypeChanged(Histogram::ErrorType)));
	connect(m_curve, SIGNAL(xErrorPlusColumnChanged(const AbstractColumn*)), this, SLOT(curveXErrorPlusColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(xErrorMinusColumnChanged(const AbstractColumn*)), this, SLOT(curveXErrorMinusColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(yErrorTypeChanged(Histogram::ErrorType)), this, SLOT(curveYErrorTypeChanged(Histogram::ErrorType)));
	connect(m_curve, SIGNAL(yErrorPlusColumnChanged(const AbstractColumn*)), this, SLOT(curveYErrorPlusColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(yErrorMinusColumnChanged(const AbstractColumn*)), this, SLOT(curveYErrorMinusColumnChanged(const AbstractColumn*)));
	connect(m_curve, SIGNAL(errorBarsCapSizeChanged(qreal)), this, SLOT(curveErrorBarsCapSizeChanged(qreal)));
	connect(m_curve, SIGNAL(errorBarsTypeChanged(Histogram::ErrorBarsType)), this, SLOT(curveErrorBarsTypeChanged(Histogram::ErrorBarsType)));
	connect(m_curve, SIGNAL(errorBarsPenChanged(QPen)), this, SLOT(curveErrorBarsPenChanged(QPen)));
	connect(m_curve, SIGNAL(errorBarsOpacityChanged(qreal)), this, SLOT(curveErrorBarsOpacityChanged(qreal)));
}

void HistogramDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "Histogram" );

  	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.
	//This data is read in HistogramDock::setCurves().

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
	//TODO: character
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
	ui.kleFillingFileName->setText( group.readEntry("FillingFileName", m_curve->fillingFileName()) );
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

	m_initializing=true;
	GuiTools::updatePenStyles(ui.cbLineStyle, ui.kcbLineColor->color());
	GuiTools::updatePenStyles(ui.cbDropLineStyle, ui.kcbDropLineColor->color());
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, ui.kcbSymbolFillingColor->color());
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, ui.kcbSymbolBorderColor->color());
	GuiTools::updatePenStyles(ui.cbErrorBarsStyle, ui.kcbErrorBarsColor->color());
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
	connect( cbXColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xColumnChanged(QModelIndex)) );
	connect( cbYColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yColumnChanged(QModelIndex)) );
}
