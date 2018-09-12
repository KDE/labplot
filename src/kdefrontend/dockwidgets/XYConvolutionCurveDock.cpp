/***************************************************************************
    File             : XYConvolutionCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of convolution curves

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

#include "XYConvolutionCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

extern "C" {
#include "backend/nsl/nsl_conv.h"
}

/*!
  \class XYConvolutionCurveDock
 \brief  Provides a widget for editing the properties of the XYConvolutionCurves
		(2D-curves defined by a convolution) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYConvolutionCurveDock::XYConvolutionCurveDock(QWidget* parent) : XYCurveDock(parent),
	cbDataSourceCurve(nullptr),
	cbXDataColumn(nullptr),
	cbYDataColumn(nullptr),
	m_convolutionCurve(nullptr) {

	//hide the line connection type
	ui.cbLineType->setDisabled(true);

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYConvolutionCurveDock::setupGeneral() {
	DEBUG("XYConvolutionCurveDock::setupGeneral()");
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);

	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2,2,2,2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 3);
	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 3);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 3);
	cbY2DataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbY2DataColumn, 8, 2, 1, 3);

	uiGeneralTab.sbMin->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	uiGeneralTab.sbMax->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	for (int i = 0; i < NSL_CONV_DIRECTION_COUNT; i++)
		uiGeneralTab.cbDirection->addItem(i18n(nsl_conv_direction_name[i]));
	for (int i = 0; i < NSL_CONV_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_conv_type_name[i]));
	// nsl_conv_method_type not exposed to user
	for (int i = 0; i < NSL_CONV_WRAP_COUNT; i++)
		uiGeneralTab.cbWrap->addItem(i18n(nsl_conv_wrap_name[i]));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYConvolutionCurveDock::nameChanged );
	connect( uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYConvolutionCurveDock::commentChanged );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( uiGeneralTab.cbDataSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataSourceTypeChanged(int)) );
	connect( uiGeneralTab.cbAutoRange, SIGNAL(clicked(bool)), this, SLOT(autoRangeChanged()) );
	connect( uiGeneralTab.sbMin, SIGNAL(valueChanged(double)), this, SLOT(xRangeMinChanged()) );
	connect( uiGeneralTab.sbMax, SIGNAL(valueChanged(double)), this, SLOT(xRangeMaxChanged()) );
	connect( uiGeneralTab.cbDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(directionChanged()) );
	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()) );
	connect( uiGeneralTab.cbNorm, SIGNAL(clicked(bool)), this, SLOT(normChanged()) );
	connect( uiGeneralTab.cbWrap, SIGNAL(currentIndexChanged(int)), this, SLOT(wrapChanged()) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );

	connect( cbDataSourceCurve, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(dataSourceCurveChanged(QModelIndex)) );
	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)) );
	connect( cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)) );
	connect( cbY2DataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(y2DataColumnChanged(QModelIndex)) );
}

void XYConvolutionCurveDock::initGeneralTab() {
	DEBUG("XYConvolutionCurveDock::initGeneralTab()");
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1) {
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.leComment->setEnabled(true);

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.leComment->setText(m_curve->comment());
	}else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.leComment->setEnabled(false);

		uiGeneralTab.leName->setText("");
		uiGeneralTab.leComment->setText("");
	}

	//show the properties of the first curve
	m_convolutionCurve = dynamic_cast<XYConvolutionCurve*>(m_curve);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(m_convolutionCurve->dataSourceType());
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_convolutionCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_convolutionCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_convolutionCurve->yDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbY2DataColumn, m_convolutionCurve->y2DataColumn());
	uiGeneralTab.cbAutoRange->setChecked(m_convolutionData.autoRange);
	uiGeneralTab.sbMin->setValue(m_convolutionData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_convolutionData.xRange.last());
	this->autoRangeChanged();
	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbDirection->setCurrentIndex(m_convolutionData.direction);
	uiGeneralTab.cbType->setCurrentIndex(m_convolutionData.type);
	//m_convolutionData.method not used
	if (m_convolutionData.normalize == nsl_conv_norm_euclidean)
		uiGeneralTab.cbNorm->setChecked(true);
	else
		uiGeneralTab.cbNorm->setChecked(false);
	uiGeneralTab.cbWrap->setCurrentIndex(m_convolutionData.wrap);

	this->directionChanged();

	this->showConvolutionResult();

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_convolutionCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_convolutionCurve, SIGNAL(dataSourceTypeChanged(XYAnalysisCurve::DataSourceType)), this, SLOT(curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType)));
	connect(m_convolutionCurve, SIGNAL(dataSourceCurveChanged(const XYCurve*)), this, SLOT(curveDataSourceCurveChanged(const XYCurve*)));
	connect(m_convolutionCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_convolutionCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_convolutionCurve, SIGNAL(y2DataColumnChanged(const AbstractColumn*)), this, SLOT(curveY2DataColumnChanged(const AbstractColumn*)));
	connect(m_convolutionCurve, SIGNAL(convolutionDataChanged(XYConvolutionCurve::ConvolutionData)), this, SLOT(curveConvolutionDataChanged(XYConvolutionCurve::ConvolutionData)));
	connect(m_convolutionCurve, SIGNAL(sourceDataChanged()), this, SLOT(enableRecalculate()));
}

void XYConvolutionCurveDock::setModel() {
	DEBUG("XYConvolutionCurveDock::setModel()");
	QList<const char*> list;
	list << "Folder" << "Datapicker" << "Worksheet" << "CartesianPlot" << "XYCurve";
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list.clear();
	list << "Folder" << "Workbook" << "Datapicker" << "DatapickerCurve" << "Spreadsheet"
		<< "FileDataSource" << "Column" << "Worksheet" << "CartesianPlot" << "XYFitCurve";
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);
	cbY2DataColumn->setTopLevelClasses(list);

	cbDataSourceCurve->setModel(m_aspectTreeModel);
	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);
	cbY2DataColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYConvolutionCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_convolutionCurve = dynamic_cast<XYConvolutionCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_convolutionData = m_convolutionCurve->convolutionData();
	initGeneralTab();
	initTabs();
	m_initializing=false;

	//hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYConvolutionCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYConvolutionCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYConvolutionCurveDock::dataSourceTypeChanged(int index) {
	XYAnalysisCurve::DataSourceType type = (XYAnalysisCurve::DataSourceType)index;
	if (type == XYAnalysisCurve::DataSourceSpreadsheet) {
		uiGeneralTab.lDataSourceCurve->hide();
		cbDataSourceCurve->hide();
		uiGeneralTab.lXColumn->show();
		cbXDataColumn->show();
		uiGeneralTab.lYColumn->show();
		cbYDataColumn->show();
		uiGeneralTab.lY2Column->show();
		cbY2DataColumn->show();
	} else {
		uiGeneralTab.lDataSourceCurve->show();
		cbDataSourceCurve->show();
		uiGeneralTab.lXColumn->hide();
		cbXDataColumn->hide();
		uiGeneralTab.lYColumn->hide();
		cbYDataColumn->hide();
		uiGeneralTab.lY2Column->hide();
		cbY2DataColumn->hide();
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setDataSourceType(type);
}

void XYConvolutionCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	XYCurve* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	// disable convolution orders and accuracies that need more data points
	this->updateSettings(dataSourceCurve->xColumn());

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYConvolutionCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setXDataColumn(column);

	if (column != nullptr) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			uiGeneralTab.sbMin->setValue(column->minimum());
			uiGeneralTab.sbMax->setValue(column->maximum());
		}

		// disable convolution methods that need more data points
		this->updateSettings(column);
	}
}

void XYConvolutionCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	//TODO
// 	size_t n=0;
// 	for (int row=0; row < column->rowCount(); row++)
// 		if (!std::isnan(column->valueAt(row)) && !column->isMasked(row))
// 			n++;
}
void XYConvolutionCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setYDataColumn(column);
}

void XYConvolutionCurveDock::y2DataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setY2DataColumn(column);
}


void XYConvolutionCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_convolutionData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_convolutionCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
			xDataColumn = m_convolutionCurve->xDataColumn();
		else {
			if (m_convolutionCurve->dataSourceCurve())
				xDataColumn = m_convolutionCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			uiGeneralTab.sbMin->setValue(xDataColumn->minimum());
			uiGeneralTab.sbMax->setValue(xDataColumn->maximum());
		}
	} else {
		uiGeneralTab.lMin->setEnabled(true);
		uiGeneralTab.sbMin->setEnabled(true);
		uiGeneralTab.lMax->setEnabled(true);
		uiGeneralTab.sbMax->setEnabled(true);
	}

}
void XYConvolutionCurveDock::xRangeMinChanged() {
	double xMin = uiGeneralTab.sbMin->value();

	m_convolutionData.xRange.first() = xMin;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYConvolutionCurveDock::xRangeMaxChanged() {
	double xMax = uiGeneralTab.sbMax->value();

	m_convolutionData.xRange.last() = xMax;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYConvolutionCurveDock::directionChanged() {
	nsl_conv_direction_type direction = (nsl_conv_direction_type) uiGeneralTab.cbDirection->currentIndex();
	m_convolutionData.direction = direction;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYConvolutionCurveDock::typeChanged() {
	nsl_conv_type_type type = (nsl_conv_type_type) uiGeneralTab.cbType->currentIndex();
	m_convolutionData.type = type;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYConvolutionCurveDock::normChanged() {
	bool norm = uiGeneralTab.cbNorm->isChecked();
	if (norm)
		m_convolutionData.normalize = nsl_conv_norm_euclidean;
	else
		m_convolutionData.normalize = nsl_conv_norm_none;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYConvolutionCurveDock::wrapChanged() {
	nsl_conv_wrap_type wrap = (nsl_conv_wrap_type) uiGeneralTab.cbWrap->currentIndex();
	m_convolutionData.wrap = wrap;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYConvolutionCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setConvolutionData(m_convolutionData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	if (m_convolutionData.direction == nsl_conv_direction_forward)
		emit info(i18n("Convolution status: %1", m_convolutionCurve->convolutionResult().status));
	else
		emit info(i18n("Deconvolution status: %1", m_convolutionCurve->convolutionResult().status));
	QApplication::restoreOverrideCursor();
}

void XYConvolutionCurveDock::enableRecalculate() const {
	DEBUG("XYConvolutionCurveDock::enableRecalculate()");
	if (m_initializing)
		return;

	//no convolution possible without the y- and y2-data
	bool hasSourceData = false;
	if (m_convolutionCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet) {
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		AbstractAspect* aspectY2 = static_cast<AbstractAspect*>(cbY2DataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectY!=nullptr && aspectY2!=nullptr);
	} else {
		 hasSourceData = (m_convolutionCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the convolution
 */
void XYConvolutionCurveDock::showConvolutionResult() {
	const XYConvolutionCurve::ConvolutionResult& convolutionResult = m_convolutionCurve->convolutionResult();
	if (!convolutionResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", convolutionResult.status) + "<br>";

	if (!convolutionResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (convolutionResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s", QString::number(convolutionResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", QString::number(convolutionResult.elapsedTime)) + "<br>";

	str += i18n("value: %1", QString::number(convolutionResult.value)) + "<br>";
 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last convolution
	uiGeneralTab.pbRecalculate->setEnabled(m_convolutionCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYConvolutionCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYConvolutionCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(type);
	m_initializing = false;
}

void XYConvolutionCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYConvolutionCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	m_initializing = false;
}

void XYConvolutionCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYConvolutionCurveDock::curveY2DataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbY2DataColumn, column);
	m_initializing = false;
}

void XYConvolutionCurveDock::curveConvolutionDataChanged(const XYConvolutionCurve::ConvolutionData& convolutionData) {
	m_initializing = true;
	m_convolutionData = convolutionData;
	this->directionChanged();

	this->showConvolutionResult();
	m_initializing = false;
}

void XYConvolutionCurveDock::dataChanged() {
	this->enableRecalculate();
}
