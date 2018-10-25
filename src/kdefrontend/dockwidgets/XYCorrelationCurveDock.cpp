/***************************************************************************
    File             : XYCorrelationCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of correlation curves

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

#include "XYCorrelationCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYCorrelationCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

extern "C" {
#include "backend/nsl/nsl_corr.h"
}

/*!
  \class XYCorrelationCurveDock
 \brief  Provides a widget for editing the properties of the XYCorrelationCurves
		(2D-curves defined by a correlation) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYCorrelationCurveDock::XYCorrelationCurveDock(QWidget* parent) : XYCurveDock(parent) {

	//hide the line connection type
	ui.cbLineType->setDisabled(true);

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYCorrelationCurveDock::setupGeneral() {
	DEBUG("XYCorrelationCurveDock::setupGeneral()");
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);

	auto* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
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
	gridLayout->addWidget(cbYDataColumn, 8, 2, 1, 3);
	cbY2DataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbY2DataColumn, 9, 2, 1, 3);

	uiGeneralTab.sbMin->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	uiGeneralTab.sbMax->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	for (int i = 0; i < NSL_CORR_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_corr_type_name[i]));
	// nsl_corr_method_type not exposed to user
	for (int i = 0; i < NSL_CORR_NORM_COUNT; i++)
		uiGeneralTab.cbNorm->addItem(i18n(nsl_corr_norm_name[i]));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	DEBUG("XYCorrelationCurveDock::setupGeneral() DONE");

	//Slots
	connect( uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYCorrelationCurveDock::nameChanged );
	connect( uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYCorrelationCurveDock::commentChanged );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( uiGeneralTab.cbDataSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataSourceTypeChanged(int)) );
	connect( uiGeneralTab.sbSamplingInterval, SIGNAL(valueChanged(double)), this, SLOT(samplingIntervalChanged()) );
	connect( uiGeneralTab.cbAutoRange, SIGNAL(clicked(bool)), this, SLOT(autoRangeChanged()) );
	connect( uiGeneralTab.sbMin, SIGNAL(valueChanged(double)), this, SLOT(xRangeMinChanged()) );
	connect( uiGeneralTab.sbMax, SIGNAL(valueChanged(double)), this, SLOT(xRangeMaxChanged()) );
	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()) );
	connect( uiGeneralTab.cbNorm, SIGNAL(currentIndexChanged(int)), this, SLOT(normChanged()) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );

	connect( cbDataSourceCurve, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(dataSourceCurveChanged(QModelIndex)) );
	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)) );
	connect( cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)) );
	connect( cbY2DataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(y2DataColumnChanged(QModelIndex)) );
}

void XYCorrelationCurveDock::initGeneralTab() {
	DEBUG("XYCorrelationCurveDock::initGeneralTab()");
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
	m_correlationCurve = dynamic_cast<XYCorrelationCurve*>(m_curve);

	// hide x-Range per default
	uiGeneralTab.lXRange->setEnabled(false);
	uiGeneralTab.cbAutoRange->setEnabled(false);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(m_correlationCurve->dataSourceType());
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_correlationCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_correlationCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_correlationCurve->yDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbY2DataColumn, m_correlationCurve->y2DataColumn());
	uiGeneralTab.sbSamplingInterval->setValue(m_correlationData.samplingInterval);
	uiGeneralTab.cbAutoRange->setChecked(m_correlationData.autoRange);
	uiGeneralTab.sbMin->setValue(m_correlationData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_correlationData.xRange.last());
	this->autoRangeChanged();
	y2DataColumnChanged(cbY2DataColumn->currentModelIndex());

	// settings
	uiGeneralTab.cbType->setCurrentIndex(m_correlationData.type);
	//m_correlationData.method not used
	uiGeneralTab.cbNorm->setCurrentIndex(m_correlationData.normalize);

	this->showCorrelationResult();

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_correlationCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_correlationCurve, SIGNAL(dataSourceTypeChanged(XYAnalysisCurve::DataSourceType)), this, SLOT(curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType)));
	connect(m_correlationCurve, SIGNAL(dataSourceCurveChanged(const XYCurve*)), this, SLOT(curveDataSourceCurveChanged(const XYCurve*)));
	connect(m_correlationCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_correlationCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_correlationCurve, SIGNAL(y2DataColumnChanged(const AbstractColumn*)), this, SLOT(curveY2DataColumnChanged(const AbstractColumn*)));
	connect(m_correlationCurve, SIGNAL(correlationDataChanged(XYCorrelationCurve::CorrelationData)), this, SLOT(curveCorrelationDataChanged(XYCorrelationCurve::CorrelationData)));
	connect(m_correlationCurve, SIGNAL(sourceDataChanged()), this, SLOT(enableRecalculate()));
}

void XYCorrelationCurveDock::setModel() {
	DEBUG("XYCorrelationCurveDock::setModel()");
	QList<const char*> list;
	list << "Folder" << "Datapicker" << "Worksheet" << "CartesianPlot" << "XYCurve";
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list.clear();
	list << "Folder" << "Workbook" << "Datapicker" << "DatapickerCurve" << "Spreadsheet"
		<< "FileDataSource" << "Column" << "Worksheet" << "CartesianPlot" << "XYCorrelationCurve";
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);
	cbY2DataColumn->setTopLevelClasses(list);

	cbDataSourceCurve->setModel(m_aspectTreeModel);
	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);
	cbY2DataColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
	DEBUG("XYCorrelationCurveDock::setModel() DONE");
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYCorrelationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_correlationCurve = dynamic_cast<XYCorrelationCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_correlationData = m_correlationCurve->correlationData();
	initGeneralTab();
	initTabs();
	m_initializing = false;

	//hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

//*************************************************************
//**** SLOTs for changes triggered in XYCorrelationCurveDock **
//*************************************************************
void XYCorrelationCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYCorrelationCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYCorrelationCurveDock::dataSourceTypeChanged(int index) {
	auto type = (XYAnalysisCurve::DataSourceType)index;
	if (type == XYAnalysisCurve::DataSourceSpreadsheet) {
		uiGeneralTab.lDataSourceCurve->hide();
		cbDataSourceCurve->hide();
		uiGeneralTab.lXColumn->show();
		cbXDataColumn->show();
		uiGeneralTab.lYColumn->show();
		cbYDataColumn->show();
		uiGeneralTab.lY2Column->show();
		cbY2DataColumn->show();
		uiGeneralTab.lSamplingInterval->show();
		uiGeneralTab.l2SamplingInterval->show();
		uiGeneralTab.sbSamplingInterval->show();
	} else {
		uiGeneralTab.lDataSourceCurve->show();
		cbDataSourceCurve->show();
		uiGeneralTab.lXColumn->hide();
		cbXDataColumn->hide();
		uiGeneralTab.lYColumn->hide();
		cbYDataColumn->hide();
		uiGeneralTab.lY2Column->hide();
		cbY2DataColumn->hide();
		uiGeneralTab.lSamplingInterval->hide();
		uiGeneralTab.l2SamplingInterval->hide();
		uiGeneralTab.sbSamplingInterval->hide();
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYCorrelationCurve*>(curve)->setDataSourceType(type);
}

void XYCorrelationCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYCorrelationCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYCorrelationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	DEBUG("XYCorrelationCurveDock::xDataColumnChanged()");
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYCorrelationCurve*>(curve)->setXDataColumn(column);

	if (column != nullptr) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			uiGeneralTab.sbMin->setValue(column->minimum());
			uiGeneralTab.sbMax->setValue(column->maximum());
		}
	}
}

void XYCorrelationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;
	DEBUG("yDataColumnChanged()");

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYCorrelationCurve*>(curve)->setYDataColumn(column);
}

void XYCorrelationCurveDock::y2DataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;
	DEBUG("y2DataColumnChanged()");

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYCorrelationCurve*>(curve)->setY2DataColumn(column);
}

void XYCorrelationCurveDock::samplingIntervalChanged() {
	double samplingInterval =  uiGeneralTab.sbSamplingInterval->value();
	m_correlationData.samplingInterval = samplingInterval;

	enableRecalculate();
}

void XYCorrelationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_correlationData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_correlationCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
			xDataColumn = m_correlationCurve->xDataColumn();
		else {
			if (m_correlationCurve->dataSourceCurve())
				xDataColumn = m_correlationCurve->dataSourceCurve()->xColumn();
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
void XYCorrelationCurveDock::xRangeMinChanged() {
	double xMin = uiGeneralTab.sbMin->value();

	m_correlationData.xRange.first() = xMin;
	enableRecalculate();
}

void XYCorrelationCurveDock::xRangeMaxChanged() {
	double xMax = uiGeneralTab.sbMax->value();

	m_correlationData.xRange.last() = xMax;
	enableRecalculate();
}

void XYCorrelationCurveDock::typeChanged() {
	auto type = (nsl_corr_type_type) uiGeneralTab.cbType->currentIndex();
	m_correlationData.type = type;

	enableRecalculate();
}

void XYCorrelationCurveDock::normChanged() {
	auto norm = (nsl_corr_norm_type) uiGeneralTab.cbNorm->currentIndex();
	m_correlationData.normalize = norm;

	enableRecalculate();
}

void XYCorrelationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		dynamic_cast<XYCorrelationCurve*>(curve)->setCorrelationData(m_correlationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Correlation status: %1", m_correlationCurve->correlationResult().status));
	QApplication::restoreOverrideCursor();
}

void XYCorrelationCurveDock::enableRecalculate() const {
	DEBUG("XYCorrelationCurveDock::enableRecalculate()");
	if (m_initializing)
		return;

	bool hasSourceData = false;
	//no correlation possible without y-data and y2-data
	if (m_correlationCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet) {
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		AbstractAspect* aspectY2 = static_cast<AbstractAspect*>(cbY2DataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectY != nullptr && aspectY2 != nullptr);
	} else {
		 hasSourceData = (m_correlationCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the correlation
 */
void XYCorrelationCurveDock::showCorrelationResult() {
	const XYCorrelationCurve::CorrelationResult& correlationResult = m_correlationCurve->correlationResult();
	if (!correlationResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", correlationResult.status) + "<br>";

	if (!correlationResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (correlationResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", QString::number(correlationResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", QString::number(correlationResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last correlation
	uiGeneralTab.pbRecalculate->setEnabled(m_correlationCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYCorrelationCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYCorrelationCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(type);
	m_initializing = false;
}

void XYCorrelationCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYCorrelationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYCorrelationCurveDock::curveXDataColumnChanged()");
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	if (column != nullptr) {
		DEBUG("X Column available");
		uiGeneralTab.lXRange->setEnabled(true);
		uiGeneralTab.cbAutoRange->setEnabled(true);
		uiGeneralTab.lSamplingInterval->setEnabled(false);
		uiGeneralTab.l2SamplingInterval->setEnabled(false);
		uiGeneralTab.sbSamplingInterval->setEnabled(false);
	} else {
		DEBUG("X Column not available");
		uiGeneralTab.lXRange->setEnabled(false);
		uiGeneralTab.cbAutoRange->setEnabled(false);
		uiGeneralTab.lSamplingInterval->setEnabled(true);
		uiGeneralTab.l2SamplingInterval->setEnabled(true);
		uiGeneralTab.sbSamplingInterval->setEnabled(true);
	}
	m_initializing = false;
}

void XYCorrelationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYCorrelationCurveDock::curveYDataColumnChanged()");
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYCorrelationCurveDock::curveY2DataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYCorrelationCurveDock::curveY2DataColumnChanged()");
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbY2DataColumn, column);
	m_initializing = false;
}

void XYCorrelationCurveDock::curveCorrelationDataChanged(const XYCorrelationCurve::CorrelationData& correlationData) {
	m_initializing = true;
	m_correlationData = correlationData;

	this->showCorrelationResult();
	m_initializing = false;
}

void XYCorrelationCurveDock::dataChanged() {
	this->enableRecalculate();
}
