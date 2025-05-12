/*
	File             : XYCorrelationCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of correlation curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYCorrelationCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYCorrelationCurve.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <KConfigGroup>

#include <QMenu>
#include <QWidgetAction>

extern "C" {
#include "backend/nsl/nsl_corr.h"
}

/*!
  \class XYCorrelationCurveDock
 \brief  Provides a widget for editing the properties of the XYCorrelationCurves
		(2D-curves defined by a correlation) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/

XYCorrelationCurveDock::XYCorrelationCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent, XYAnalysisCurveDock::RequiredDataSource::YY2) {
}

/*!
 * 	// Tab "General"
 */
void XYCorrelationCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment, uiGeneralTab.pbRecalculate, uiGeneralTab.cbDataSourceType);
	setVisibilityWidgets(uiGeneralTab.chkVisible, uiGeneralTab.chkLegendVisible);

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2, 2, 2, 2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 3);
	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 3);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 8, 2, 1, 3);
	cbY2DataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbY2DataColumn, 9, 2, 1, 3);

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	for (int i = 0; i < NSL_CORR_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_corr_type_name[i]));
	// nsl_corr_method_type not exposed to user
	for (int i = 0; i < NSL_CORR_NORM_COUNT; i++)
		uiGeneralTab.cbNorm->addItem(i18n(nsl_corr_norm_name[i]));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCorrelationCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.sbSamplingInterval, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYCorrelationCurveDock::samplingIntervalChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYCorrelationCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYCorrelationCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYCorrelationCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCorrelationCurveDock::typeChanged);
	connect(uiGeneralTab.cbNorm, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCorrelationCurveDock::normChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYCorrelationCurveDock::recalculateClicked);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCorrelationCurveDock::plotRangeChanged);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYCorrelationCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCorrelationCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCorrelationCurveDock::yDataColumnChanged);
	connect(cbY2DataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCorrelationCurveDock::y2DataColumnChanged);
}

void XYCorrelationCurveDock::initGeneralTab() {
	// show the properties of the first curve
	//  hide x-Range per default
	uiGeneralTab.lXRange->setEnabled(false);
	uiGeneralTab.cbAutoRange->setEnabled(false);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_correlationCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_correlationCurve->dataSourceCurve());
	cbXDataColumn->setAspect(m_correlationCurve->xDataColumn(), m_correlationCurve->xDataColumnPath());
	cbYDataColumn->setAspect(m_correlationCurve->yDataColumn(), m_correlationCurve->yDataColumnPath());
	cbY2DataColumn->setAspect(m_correlationCurve->y2DataColumn(), m_correlationCurve->y2DataColumnPath());
	uiGeneralTab.sbSamplingInterval->setValue(m_correlationData.samplingInterval);
	uiGeneralTab.cbAutoRange->setChecked(m_correlationData.autoRange);

	const auto numberLocale = QLocale();
	uiGeneralTab.leMin->setText(numberLocale.toString(m_correlationData.xRange.first()));
	uiGeneralTab.leMax->setText(numberLocale.toString(m_correlationData.xRange.last()));
	this->autoRangeChanged();
	y2DataColumnChanged(cbY2DataColumn->currentModelIndex());

	// settings
	uiGeneralTab.cbType->setCurrentIndex(m_correlationData.type);
	// m_correlationData.method not used
	uiGeneralTab.cbNorm->setCurrentIndex(m_correlationData.normalize);

	this->showCorrelationResult();

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_correlationCurve, &XYCorrelationCurve::dataSourceTypeChanged, this, &XYCorrelationCurveDock::curveDataSourceTypeChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::dataSourceCurveChanged, this, &XYCorrelationCurveDock::curveDataSourceCurveChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::xDataColumnChanged, this, &XYCorrelationCurveDock::curveXDataColumnChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::yDataColumnChanged, this, &XYCorrelationCurveDock::curveYDataColumnChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::y2DataColumnChanged, this, &XYCorrelationCurveDock::curveY2DataColumnChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::correlationDataChanged, this, &XYCorrelationCurveDock::curveCorrelationDataChanged);
	connect(m_correlationCurve, &XYCorrelationCurve::sourceDataChanged, this, &XYCorrelationCurveDock::enableRecalculate);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYCorrelationCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	setAnalysisCurves(list);
	m_correlationCurve = static_cast<XYCorrelationCurve*>(m_curve);
	m_correlationData = m_correlationCurve->correlationData();

	const auto numberLocale = QLocale();
	uiGeneralTab.sbSamplingInterval->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	setSymbols(list);

	updatePlotRangeList();
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

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYCorrelationCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

void XYCorrelationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	for (auto* curve : m_curvesList)
		static_cast<XYCorrelationCurve*>(curve)->setXDataColumn(column);

	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
		uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
	}

	enableRecalculate();
}

void XYCorrelationCurveDock::samplingIntervalChanged() {
	double samplingInterval = uiGeneralTab.sbSamplingInterval->value();
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
			const auto numberLocale = QLocale();
			uiGeneralTab.leMin->setText(numberLocale.toString(xDataColumn->minimum()));
			uiGeneralTab.leMax->setText(numberLocale.toString(xDataColumn->maximum()));
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
	auto type = (nsl_corr_type_type)uiGeneralTab.cbType->currentIndex();
	m_correlationData.type = type;

	enableRecalculate();
}

void XYCorrelationCurveDock::normChanged() {
	auto norm = (nsl_corr_norm_type)uiGeneralTab.cbNorm->currentIndex();
	m_correlationData.normalize = norm;

	enableRecalculate();
}

void XYCorrelationCurveDock::recalculateClicked() {
	for (auto* curve : m_curvesList)
		static_cast<XYCorrelationCurve*>(curve)->setCorrelationData(m_correlationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Correlation status: %1", m_correlationCurve->correlationResult().status));
}

/*!
 * show the result and details of the correlation
 */
void XYCorrelationCurveDock::showCorrelationResult() {
	showResult(m_correlationCurve, uiGeneralTab.teResult);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYCorrelationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYCorrelationCurveDock::curveXDataColumnChanged()");
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
	CONDITIONAL_LOCK_RETURN;
	cbXDataColumn->setAspect(column, m_correlationCurve->xDataColumnPath());
	enableRecalculate();
}

void XYCorrelationCurveDock::curveY2DataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbY2DataColumn->setAspect(column, m_correlationCurve->y2DataColumnPath());
	enableRecalculate();
}

void XYCorrelationCurveDock::curveCorrelationDataChanged(const XYCorrelationCurve::CorrelationData& correlationData) {
	CONDITIONAL_LOCK_RETURN;
	m_correlationData = correlationData;

	this->showCorrelationResult();
}
