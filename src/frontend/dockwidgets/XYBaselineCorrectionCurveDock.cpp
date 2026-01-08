/*
	File             : XYBaselineCorrectionCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of baseline correction curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYBaselineCorrectionCurveDock.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYBaselineCorrectionCurve.h"
#include "frontend/widgets/TreeViewComboBox.h"

extern "C" {
#include "backend/nsl/nsl_baseline.h"
}

/*!
	\class XYBaselineCorrectionCurveDock
	\brief  Provides a widget for editing the properties of \c XYBaselineCorrectionCurve.

	\ingroup frontend
*/

XYBaselineCorrectionCurveDock::XYBaselineCorrectionCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
}

void XYBaselineCorrectionCurveDock::setupGeneral() {
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
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 3);

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	updateLocale();
	retranslateUi();

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYBaselineCorrectionCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYBaselineCorrectionCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYBaselineCorrectionCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYBaselineCorrectionCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYBaselineCorrectionCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYBaselineCorrectionCurveDock::xRangeMaxDateTimeChanged);

	connect(uiGeneralTab.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYBaselineCorrectionCurveDock::methodChanged);
	connect(uiGeneralTab.sbARPLSSmoothness, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYBaselineCorrectionCurveDock::arPLSSmoothnessChanged);
	connect(uiGeneralTab.sbARPLSTerminationRatio, QOverload<double>::of(&NumberSpinBox::valueChanged), this, QOverload<double>::of(&XYBaselineCorrectionCurveDock::arPLSTerminationRatioChanged));
	connect(uiGeneralTab.sbARPLSIterations, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYBaselineCorrectionCurveDock::arPLSIterationsChanged);

	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYBaselineCorrectionCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYBaselineCorrectionCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYBaselineCorrectionCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYBaselineCorrectionCurveDock::yDataColumnChanged);
}

void XYBaselineCorrectionCurveDock::initGeneralTab() {
	// show the properties of the first curve
	// data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_baselineCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_baselineCurve->dataSourceCurve());
	cbXDataColumn->setAspect(m_baselineCurve->xDataColumn(), m_baselineCurve->xDataColumnPath());
	cbYDataColumn->setAspect(m_baselineCurve->yDataColumn(), m_baselineCurve->yDataColumnPath());

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_baselineCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(m_data.xRange.first()));
		uiGeneralTab.leMax->setText(numberLocale.toString(m_data.xRange.last()));
	} else {
		uiGeneralTab.dateTimeEditMin->setMSecsSinceEpochUTC(m_data.xRange.first());
		uiGeneralTab.dateTimeEditMax->setMSecsSinceEpochUTC(m_data.xRange.last());
	}

	uiGeneralTab.lMin->setVisible(!m_dateTimeRange);
	uiGeneralTab.leMin->setVisible(!m_dateTimeRange);
	uiGeneralTab.lMax->setVisible(!m_dateTimeRange);
	uiGeneralTab.leMax->setVisible(!m_dateTimeRange);
	uiGeneralTab.lMinDateTime->setVisible(m_dateTimeRange);
	uiGeneralTab.dateTimeEditMin->setVisible(m_dateTimeRange);
	uiGeneralTab.lMaxDateTime->setVisible(m_dateTimeRange);
	uiGeneralTab.dateTimeEditMax->setVisible(m_dateTimeRange);

	// auto range
	uiGeneralTab.cbAutoRange->setChecked(m_data.autoRange);
	this->autoRangeChanged();

	// method
	uiGeneralTab.cbMethod->setCurrentIndex(m_data.method);
	this->methodChanged(m_data.method);

	// arPLS parameters
	uiGeneralTab.sbARPLSSmoothness->setValue(m_data.arPLSSmoothness);
	uiGeneralTab.sbARPLSTerminationRatio->setValue(m_data.arPLSTerminationRatio);
	uiGeneralTab.sbARPLSIterations->setValue(m_data.arPLSIterations);

	this->showBaselineCorrectionResult();

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_baselineCurve, &XYBaselineCorrectionCurve::dataSourceTypeChanged, this, &XYBaselineCorrectionCurveDock::curveDataSourceTypeChanged);
	connect(m_baselineCurve, &XYBaselineCorrectionCurve::dataSourceCurveChanged, this, &XYBaselineCorrectionCurveDock::curveDataSourceCurveChanged);
	connect(m_baselineCurve, &XYBaselineCorrectionCurve::xDataColumnChanged, this, &XYBaselineCorrectionCurveDock::curveXDataColumnChanged);
	connect(m_baselineCurve, &XYBaselineCorrectionCurve::yDataColumnChanged, this, &XYBaselineCorrectionCurveDock::curveYDataColumnChanged);
	connect(m_baselineCurve, &XYBaselineCorrectionCurve::baselineDataChanged, this, &XYBaselineCorrectionCurveDock::curveBaselineDataChanged);
	connect(m_baselineCurve, &XYBaselineCorrectionCurve::sourceDataChanged, this, &XYBaselineCorrectionCurveDock::enableRecalculate);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYBaselineCorrectionCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	setAnalysisCurves(list);
	m_baselineCurve = static_cast<XYBaselineCorrectionCurve*>(m_curve);
	m_data = m_baselineCurve->baselineData();

	initGeneralTab();
	initTabs();
	setSymbols(list);

	updatePlotRangeList();
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void XYBaselineCorrectionCurveDock::updateLocale() {
	CONDITIONAL_LOCK_RETURN;
	const auto numberLocale = QLocale();
	uiGeneralTab.sbARPLSSmoothness->setLocale(numberLocale);
	uiGeneralTab.sbARPLSTerminationRatio->setLocale(numberLocale);
	uiGeneralTab.sbARPLSSmoothness->setLocale(numberLocale);
}

void XYBaselineCorrectionCurveDock::retranslateUi() {
	uiGeneralTab.cbMethod->clear();
	// TODO:
	// for (int i = 0; i < NSL_BASELINE_SUBTRACTION_METHOD_COUNT; ++i)
	// 	uiGeneralTab.cbMethod->addItem(i18n(nsl_baseline_correction_method_name[i]));
	uiGeneralTab.cbMethod->addItem(QStringLiteral("arPLS"));
	uiGeneralTab.cbMethod->addItem(i18n("Minimum"));
	uiGeneralTab.cbMethod->addItem(i18n("Maximum"));
	uiGeneralTab.cbMethod->addItem(i18n("Mean"));
	uiGeneralTab.cbMethod->addItem(i18n("Median"));
	uiGeneralTab.cbMethod->addItem(i18n("End Points"));
	uiGeneralTab.cbMethod->addItem(i18n("Linear Regression"));

	// tooltips
	QString info = i18n(
		"Method used to calculate the baseline correction:"
		"<ul>"
		"<li>arPLS - asymmetrically reweighted penalized least square method.</li>"
		"<li>Minimum - subtract the minimum of the data.</li>"
		"<li>Maximum - subtract the maximum of the data.</li>"
		"<li>Mean - subtract the mean of the data.</li>"
		"<li>Median - subtract the median of the data.</li>"
		"<li>End Points - do a linear interpolation using first and last point and subtract it.</li>"
		"<li>Linear Regression - do a linear regression to the data and subtract it.</li>"
		"</ul>");
	uiGeneralTab.lMethod->setToolTip(info);
	uiGeneralTab.cbMethod->setToolTip(info);

	info = i18n("Smoothness parameter - the larger the value the smoother the resulting background.");
	uiGeneralTab.lARPLSSmoothness->setToolTip(info);
	uiGeneralTab.sbARPLSSmoothness->setToolTip(info);

	info = i18n("Weighting termination ratio - value between 0 and 1, smaller values allow less negative values.");
	uiGeneralTab.lARPLSTerminationRatio->setToolTip(info);
	uiGeneralTab.sbARPLSTerminationRatio->setToolTip(info);

	info = i18n("Number of iterations to perform.");
	uiGeneralTab.lARPLSIterations->setToolTip(info);
	uiGeneralTab.sbARPLSIterations->setToolTip(info);
}

//****************************************************************
//* SLOTs for changes triggered in XYBaselineCorrectionCurveDock *
//****************************************************************
void XYBaselineCorrectionCurveDock::dataSourceTypeChanged(int index) {
	const auto type = (XYAnalysisCurve::DataSourceType)index;
	if (type == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		uiGeneralTab.lDataSourceCurve->hide();
		cbDataSourceCurve->hide();
		uiGeneralTab.lXColumn->show();
		cbXDataColumn->show();
		uiGeneralTab.lYColumn->show();
		cbYDataColumn->show();
	} else {
		uiGeneralTab.lDataSourceCurve->show();
		cbDataSourceCurve->show();
		uiGeneralTab.lXColumn->hide();
		cbXDataColumn->hide();
		uiGeneralTab.lYColumn->hide();
		cbYDataColumn->hide();
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYBaselineCorrectionCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

void XYBaselineCorrectionCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_data.autoRange = autoRange;

	uiGeneralTab.lMin->setEnabled(!autoRange);
	uiGeneralTab.leMin->setEnabled(!autoRange);
	uiGeneralTab.lMax->setEnabled(!autoRange);
	uiGeneralTab.leMax->setEnabled(!autoRange);
	uiGeneralTab.lMinDateTime->setEnabled(!autoRange);
	uiGeneralTab.dateTimeEditMin->setEnabled(!autoRange);
	uiGeneralTab.lMaxDateTime->setEnabled(!autoRange);
	uiGeneralTab.dateTimeEditMax->setEnabled(!autoRange);

	if (autoRange) {
		const AbstractColumn* xDataColumn = nullptr;
		if (m_baselineCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_baselineCurve->xDataColumn();
		else {
			if (m_baselineCurve->dataSourceCurve())
				xDataColumn = m_baselineCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			if (!m_dateTimeRange) {
				const auto numberLocale = QLocale();
				uiGeneralTab.leMin->setText(numberLocale.toString(xDataColumn->minimum()));
				uiGeneralTab.leMax->setText(numberLocale.toString(xDataColumn->maximum()));
			} else {
				uiGeneralTab.dateTimeEditMin->setMSecsSinceEpochUTC(xDataColumn->minimum());
				uiGeneralTab.dateTimeEditMax->setMSecsSinceEpochUTC(xDataColumn->maximum());
			}
		}
	}
}

void XYBaselineCorrectionCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_data.xRange.first(), uiGeneralTab.leMin);
}

void XYBaselineCorrectionCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_data.xRange.last(), uiGeneralTab.leMax);
}

void XYBaselineCorrectionCurveDock::xRangeMinDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_data.xRange.first() = value;
	enableRecalculate();
}

void XYBaselineCorrectionCurveDock::xRangeMaxDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_data.xRange.last() = value;
	enableRecalculate();
}

void XYBaselineCorrectionCurveDock::recalculateClicked() {
	for (auto* curve : m_curvesList)
		static_cast<XYBaselineCorrectionCurve*>(curve)->setBaselineData(m_data);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Baseline correction status: %1", m_baselineCurve->baselineResult().status));
}

void XYBaselineCorrectionCurveDock::methodChanged(int index) {
	auto method = static_cast<nsl_baseline_correction_method>(index);
	bool visible = (method == nsl_diff_baseline_correction_arpls);
	uiGeneralTab.lARPLSSmoothness->setVisible(visible);
	uiGeneralTab.sbARPLSSmoothness->setVisible(visible);
	uiGeneralTab.lARPLSTerminationRatio->setVisible(visible);
	uiGeneralTab.sbARPLSTerminationRatio->setVisible(visible);
	uiGeneralTab.lARPLSIterations->setVisible(visible);
	uiGeneralTab.sbARPLSIterations->setVisible(visible);

	m_data.method = method;
	enableRecalculate();
}

void XYBaselineCorrectionCurveDock::arPLSSmoothnessChanged(int value) {
	m_data.arPLSSmoothness = value;
	enableRecalculate();
}

void XYBaselineCorrectionCurveDock::arPLSTerminationRatioChanged(double value) {
	m_data.arPLSTerminationRatio = value;
	enableRecalculate();
}

void XYBaselineCorrectionCurveDock::arPLSIterationsChanged(int value) {
	m_data.arPLSIterations = value;
	enableRecalculate();
}

/*!
 * show the result and details of the differentiation
 */
void XYBaselineCorrectionCurveDock::showBaselineCorrectionResult() {
	showResult(m_baselineCurve, uiGeneralTab.teResult);
}

//*************************************************************
//* SLOTs for changes triggered in XYBaselineCorrectionCurve **
//*************************************************************
// General-Tab
void XYBaselineCorrectionCurveDock::curveBaselineDataChanged(const XYBaselineCorrectionCurve::BaselineData& bdata) {
	CONDITIONAL_LOCK_RETURN;
	m_data = bdata;
	uiGeneralTab.cbMethod->setCurrentIndex(m_data.method);
	methodChanged(m_data.method);
	uiGeneralTab.sbARPLSSmoothness->setValue(m_data.arPLSSmoothness);
	uiGeneralTab.sbARPLSTerminationRatio->setValue(m_data.arPLSTerminationRatio);
	uiGeneralTab.sbARPLSIterations->setValue(m_data.arPLSIterations);

	this->showBaselineCorrectionResult();
}
