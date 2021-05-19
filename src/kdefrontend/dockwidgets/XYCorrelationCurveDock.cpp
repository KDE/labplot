/***************************************************************************
    File             : XYCorrelationCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2018-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include <KConfigGroup>
#include <KSharedConfig>

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
}

/*!
 * 	// Tab "General"
 */
void XYCorrelationCurveDock::setupGeneral() {
	DEBUG("XYCorrelationCurveDock::setupGeneral()");
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_teComment = uiGeneralTab.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2,2,2,2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

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

	uiGeneralTab.leMin->setValidator( new QDoubleValidator(uiGeneralTab.leMin) );
	uiGeneralTab.leMax->setValidator( new QDoubleValidator(uiGeneralTab.leMax) );

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
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYCorrelationCurveDock::nameChanged );
	connect(uiGeneralTab.teComment, &QTextEdit::textChanged, this, &XYCorrelationCurveDock::commentChanged );
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYCorrelationCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCorrelationCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.sbSamplingInterval, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYCorrelationCurveDock::samplingIntervalChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYCorrelationCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYCorrelationCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYCorrelationCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCorrelationCurveDock::typeChanged);
	connect(uiGeneralTab.cbNorm, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCorrelationCurveDock::normChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYCorrelationCurveDock::recalculateClicked);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCorrelationCurveDock::plotRangeChanged );

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYCorrelationCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCorrelationCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCorrelationCurveDock::yDataColumnChanged);
	connect(cbY2DataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCorrelationCurveDock::y2DataColumnChanged);
}

void XYCorrelationCurveDock::initGeneralTab() {
	DEBUG("XYCorrelationCurveDock::initGeneralTab()");
	//if there are more then one curve in the list, disable the tab "general"
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
	// hide x-Range per default
	uiGeneralTab.lXRange->setEnabled(false);
	uiGeneralTab.cbAutoRange->setEnabled(false);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_correlationCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_correlationCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_correlationCurve->xDataColumn(), m_correlationCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_correlationCurve->yDataColumn(), m_correlationCurve->yDataColumnPath());
	cbY2DataColumn->setColumn(m_correlationCurve->y2DataColumn(), m_correlationCurve->y2DataColumnPath());
	uiGeneralTab.sbSamplingInterval->setValue(m_correlationData.samplingInterval);
	uiGeneralTab.cbAutoRange->setChecked(m_correlationData.autoRange);

	SET_NUMBER_LOCALE
	uiGeneralTab.leMin->setText( numberLocale.toString(m_correlationData.xRange.first()) );
	uiGeneralTab.leMax->setText( numberLocale.toString(m_correlationData.xRange.last()) );
	this->autoRangeChanged();
	y2DataColumnChanged(cbY2DataColumn->currentModelIndex());

	// settings
	uiGeneralTab.cbType->setCurrentIndex(m_correlationData.type);
	//m_correlationData.method not used
	uiGeneralTab.cbNorm->setCurrentIndex(m_correlationData.normalize);

	this->showCorrelationResult();

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_correlationCurve, &XYCorrelationCurve::aspectDescriptionChanged, this, &XYCorrelationCurveDock::aspectDescriptionChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::dataSourceTypeChanged, this, &XYCorrelationCurveDock::curveDataSourceTypeChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::dataSourceCurveChanged, this, &XYCorrelationCurveDock::curveDataSourceCurveChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::xDataColumnChanged, this, &XYCorrelationCurveDock::curveXDataColumnChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::yDataColumnChanged, this, &XYCorrelationCurveDock::curveYDataColumnChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::y2DataColumnChanged, this, &XYCorrelationCurveDock::curveY2DataColumnChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::correlationDataChanged, this, &XYCorrelationCurveDock::curveCorrelationDataChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::sourceDataChanged, this, &XYCorrelationCurveDock::enableRecalculate);
	connect(m_correlationCurve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &XYCorrelationCurveDock::curveVisibilityChanged);
	connect(m_correlationCurve, &WorksheetElement::plotRangeListChanged, this, &XYCorrelationCurveDock::updatePlotRanges);
}

void XYCorrelationCurveDock::setModel() {
	DEBUG("XYCorrelationCurveDock::setModel()");
	QList<AspectType> list{AspectType::Folder, AspectType::Datapicker, AspectType::Worksheet,
	                       AspectType::CartesianPlot, AspectType::XYCurve, AspectType::XYAnalysisCurve};
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list = {AspectType::Folder, AspectType::Workbook, AspectType::Datapicker, AspectType::DatapickerCurve,
	        AspectType::Spreadsheet, AspectType::LiveDataSource, AspectType::Column,
	        AspectType::Worksheet, AspectType::CartesianPlot, AspectType::XYCorrelationCurve
	       };
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
	m_aspect = m_curve;
	m_correlationCurve = static_cast<XYCorrelationCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_correlationData = m_correlationCurve->correlationData();

	SET_NUMBER_LOCALE
	uiGeneralTab.sbSamplingInterval->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	m_initializing = false;

	updatePlotRanges();

	//hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

void XYCorrelationCurveDock::updatePlotRanges() const {
	updatePlotRangeList(uiGeneralTab.cbPlotRanges);
}

//*************************************************************
//**** SLOTs for changes triggered in XYCorrelationCurveDock **
//*************************************************************
void XYCorrelationCurveDock::dataSourceTypeChanged(int index) {
	auto type = (XYAnalysisCurve::DataSourceType)index;
	if (type == XYAnalysisCurve::DataSourceType::Spreadsheet) {
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

	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		SET_NUMBER_LOCALE
		uiGeneralTab.leMin->setText( numberLocale.toString(column->minimum()) );
		uiGeneralTab.leMax->setText( numberLocale.toString(column->maximum()) );
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYCorrelationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYCorrelationCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYCorrelationCurveDock::y2DataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYCorrelationCurve*>(curve)->setY2DataColumn(column);

	cbY2DataColumn->useCurrentIndexText(true);
	cbY2DataColumn->setInvalid(false);
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
		uiGeneralTab.leMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.leMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_correlationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_correlationCurve->xDataColumn();
		else {
			if (m_correlationCurve->dataSourceCurve())
				xDataColumn = m_correlationCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			SET_NUMBER_LOCALE
			uiGeneralTab.leMin->setText( numberLocale.toString(xDataColumn->minimum()) );
			uiGeneralTab.leMax->setText( numberLocale.toString(xDataColumn->maximum()) );
		}
	} else {
		uiGeneralTab.lMin->setEnabled(true);
		uiGeneralTab.leMin->setEnabled(true);
		uiGeneralTab.lMax->setEnabled(true);
		uiGeneralTab.leMax->setEnabled(true);
	}

}
void XYCorrelationCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_correlationData.xRange.first(), uiGeneralTab.leMin);
}

void XYCorrelationCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_correlationData.xRange.last(), uiGeneralTab.leMax);
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
	if (m_correlationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		AbstractAspect* aspectY2 = static_cast<AbstractAspect*>(cbY2DataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectY != nullptr && aspectY2 != nullptr);
		if (aspectY) {
			cbYDataColumn->useCurrentIndexText(true);
			cbYDataColumn->setInvalid(false);
		}
		if (aspectY2) {
			cbY2DataColumn->useCurrentIndexText(true);
			cbY2DataColumn->setInvalid(false);
		}
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

	SET_NUMBER_LOCALE
	if (correlationResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", numberLocale.toString(correlationResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", numberLocale.toString(correlationResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last correlation
	uiGeneralTab.pbRecalculate->setEnabled(m_correlationCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYCorrelationCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void XYCorrelationCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	cbDataSourceCurve->setAspect(curve);
	m_initializing = false;
}

void XYCorrelationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYCorrelationCurveDock::curveXDataColumnChanged()");
	m_initializing = true;
	cbXDataColumn->setColumn(column, m_correlationCurve->xDataColumnPath());
	if (column) {
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
	cbYDataColumn->setColumn(column, m_correlationCurve->yDataColumnPath());
	m_initializing = false;
}

void XYCorrelationCurveDock::curveY2DataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYCorrelationCurveDock::curveY2DataColumnChanged()");
	m_initializing = true;
	cbY2DataColumn->setColumn(column, m_correlationCurve->y2DataColumnPath());
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

void XYCorrelationCurveDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	uiGeneralTab.chkVisible->setChecked(on);
	m_initializing = false;
}
