/*
	File             : XYDifferentiationCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of differentiation curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYDifferentiationCurveDock.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QStandardItemModel>

extern "C" {
#include "backend/nsl/nsl_diff.h"
}

/*!
  \class XYDifferentiationCurveDock
 \brief  Provides a widget for editing the properties of the XYDifferentiationCurves
		(2D-curves defined by a differentiation) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/

XYDifferentiationCurveDock::XYDifferentiationCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYDifferentiationCurveDock::setupGeneral() {
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

	for (int i = 0; i < NSL_DIFF_DERIV_ORDER_COUNT; ++i)
		uiGeneralTab.cbDerivOrder->addItem(i18n(nsl_diff_deriv_order_name[i]));

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYDifferentiationCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYDifferentiationCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYDifferentiationCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYDifferentiationCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYDifferentiationCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYDifferentiationCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.cbDerivOrder, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYDifferentiationCurveDock::derivOrderChanged);
	connect(uiGeneralTab.sbAccOrder, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYDifferentiationCurveDock::accOrderChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYDifferentiationCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYDifferentiationCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYDifferentiationCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYDifferentiationCurveDock::yDataColumnChanged);
}

void XYDifferentiationCurveDock::initGeneralTab() {
	// show the properties of the first curve
	// data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_differentiationCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_differentiationCurve->dataSourceCurve());
	cbXDataColumn->setAspect(m_differentiationCurve->xDataColumn(), m_differentiationCurve->xDataColumnPath());
	cbYDataColumn->setAspect(m_differentiationCurve->yDataColumn(), m_differentiationCurve->yDataColumnPath());

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_differentiationCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(m_differentiationData.xRange.first()));
		uiGeneralTab.leMax->setText(numberLocale.toString(m_differentiationData.xRange.last()));
	} else {
		uiGeneralTab.dateTimeEditMin->setMSecsSinceEpochUTC(m_differentiationData.xRange.first());
		uiGeneralTab.dateTimeEditMax->setMSecsSinceEpochUTC(m_differentiationData.xRange.last());
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
	uiGeneralTab.cbAutoRange->setChecked(m_differentiationData.autoRange);
	this->autoRangeChanged();

	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbDerivOrder->setCurrentIndex(m_differentiationData.derivOrder);
	this->derivOrderChanged(m_differentiationData.derivOrder);

	uiGeneralTab.sbAccOrder->setValue(m_differentiationData.accOrder);
	this->accOrderChanged(m_differentiationData.accOrder);

	this->showDifferentiationResult();

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_differentiationCurve, &XYDifferentiationCurve::dataSourceTypeChanged, this, &XYDifferentiationCurveDock::curveDataSourceTypeChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::dataSourceCurveChanged, this, &XYDifferentiationCurveDock::curveDataSourceCurveChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::xDataColumnChanged, this, &XYDifferentiationCurveDock::curveXDataColumnChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::yDataColumnChanged, this, &XYDifferentiationCurveDock::curveYDataColumnChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::differentiationDataChanged, this, &XYDifferentiationCurveDock::curveDifferentiationDataChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::sourceDataChanged, this, &XYDifferentiationCurveDock::enableRecalculate);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYDifferentiationCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	setAnalysisCurves(list);
	m_differentiationCurve = static_cast<XYDifferentiationCurve*>(m_curve);
	m_differentiationData = m_differentiationCurve->differentiationData();

	initGeneralTab();
	initTabs();
	setSymbols(list);

	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYDifferentiationCurveDock::dataSourceTypeChanged(int index) {
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
		static_cast<XYDifferentiationCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

/*!
 * disable deriv orders and accuracies that need more data points
 */
void XYDifferentiationCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	const auto& statistics = static_cast<const Column*>(column)->statistics();

	if (uiGeneralTab.cbAutoRange->isChecked()) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(statistics.minimum));
		uiGeneralTab.leMax->setText(numberLocale.toString(statistics.maximum));
	}

	const int n = statistics.size;

	const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbDerivOrder->model());
	auto* item = model->item(nsl_diff_deriv_order_first);
	if (n < 3)
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	else {
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		if (n < 5)
			uiGeneralTab.sbAccOrder->setMinimum(2);
	}

	item = model->item(nsl_diff_deriv_order_second);
	if (n < 3) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_second)
			uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	} else {
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		if (n < 4)
			uiGeneralTab.sbAccOrder->setMinimum(1);
		else if (n < 5)
			uiGeneralTab.sbAccOrder->setMinimum(2);
	}

	item = model->item(nsl_diff_deriv_order_third);
	if (n < 5) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_third)
			uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	item = model->item(nsl_diff_deriv_order_fourth);
	if (n < 5) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_fourth)
			uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	} else {
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		if (n < 7)
			uiGeneralTab.sbAccOrder->setMinimum(1);
	}

	item = model->item(nsl_diff_deriv_order_fifth);
	if (n < 7) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_fifth)
			uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	item = model->item(nsl_diff_deriv_order_sixth);
	if (n < 7) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_sixth)
			uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void XYDifferentiationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_differentiationData.autoRange = autoRange;

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
		if (m_differentiationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_differentiationCurve->xDataColumn();
		else {
			if (m_differentiationCurve->dataSourceCurve())
				xDataColumn = m_differentiationCurve->dataSourceCurve()->xColumn();
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

void XYDifferentiationCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_differentiationData.xRange.first(), uiGeneralTab.leMin);
}

void XYDifferentiationCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_differentiationData.xRange.last(), uiGeneralTab.leMax);
}

void XYDifferentiationCurveDock::xRangeMinDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_differentiationData.xRange.first() = value;
	enableRecalculate();
}

void XYDifferentiationCurveDock::xRangeMaxDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_differentiationData.xRange.last() = value;
	enableRecalculate();
}

void XYDifferentiationCurveDock::derivOrderChanged(int index) {
	const auto derivOrder = (nsl_diff_deriv_order_type)index;
	m_differentiationData.derivOrder = derivOrder;

	// update avail. accuracies
	switch (derivOrder) {
	case nsl_diff_deriv_order_first:
		uiGeneralTab.sbAccOrder->setMinimum(2);
		uiGeneralTab.sbAccOrder->setMaximum(4);
		uiGeneralTab.sbAccOrder->setSingleStep(2);
		if (m_differentiationData.accOrder != 2 && m_differentiationData.accOrder != 4)
			uiGeneralTab.sbAccOrder->setValue(4);
		break;
	case nsl_diff_deriv_order_second:
		uiGeneralTab.sbAccOrder->setMinimum(1);
		uiGeneralTab.sbAccOrder->setMaximum(3);
		uiGeneralTab.sbAccOrder->setSingleStep(1);
		if (m_differentiationData.accOrder != 1 && m_differentiationData.accOrder != 2 && m_differentiationData.accOrder != 3)
			uiGeneralTab.sbAccOrder->setValue(3);
		break;
	case nsl_diff_deriv_order_third:
		uiGeneralTab.sbAccOrder->setMinimum(2);
		uiGeneralTab.sbAccOrder->setMaximum(2);
		if (m_differentiationData.accOrder != 2)
			uiGeneralTab.sbAccOrder->setValue(2);
		break;
	case nsl_diff_deriv_order_fourth:
		uiGeneralTab.sbAccOrder->setMinimum(1);
		uiGeneralTab.sbAccOrder->setMaximum(3);
		uiGeneralTab.sbAccOrder->setSingleStep(2);
		if (m_differentiationData.accOrder != 1 && m_differentiationData.accOrder != 3)
			uiGeneralTab.sbAccOrder->setValue(3);
		break;
	case nsl_diff_deriv_order_fifth:
		uiGeneralTab.sbAccOrder->setMinimum(2);
		uiGeneralTab.sbAccOrder->setMaximum(2);
		if (m_differentiationData.accOrder != 2)
			uiGeneralTab.sbAccOrder->setValue(2);
		break;
	case nsl_diff_deriv_order_sixth:
		uiGeneralTab.sbAccOrder->setMinimum(1);
		uiGeneralTab.sbAccOrder->setMaximum(1);
		if (m_differentiationData.accOrder != 1)
			uiGeneralTab.sbAccOrder->setValue(1);
		break;
	}

	enableRecalculate();
}

void XYDifferentiationCurveDock::accOrderChanged(int value) {
	m_differentiationData.accOrder = value;
	enableRecalculate();
}

void XYDifferentiationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		static_cast<XYDifferentiationCurve*>(curve)->setDifferentiationData(m_differentiationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Differentiation status: %1", m_differentiationCurve->differentiationResult().status));
	QApplication::restoreOverrideCursor();
}

/*!
 * show the result and details of the differentiation
 */
void XYDifferentiationCurveDock::showDifferentiationResult() {
	showResult(m_differentiationCurve, uiGeneralTab.teResult);
}

//*************************************************************
//*** SLOTs for changes triggered in XYDifferentiationCurve ***
//*************************************************************
// General-Tab
void XYDifferentiationCurveDock::curveDifferentiationDataChanged(const XYDifferentiationCurve::DifferentiationData& differentiationData) {
	CONDITIONAL_LOCK_RETURN;
	m_differentiationData = differentiationData;
	uiGeneralTab.cbDerivOrder->setCurrentIndex(m_differentiationData.derivOrder);
	this->derivOrderChanged(m_differentiationData.derivOrder);
	uiGeneralTab.sbAccOrder->setValue(m_differentiationData.accOrder);
	this->accOrderChanged(m_differentiationData.accOrder);

	this->showDifferentiationResult();
}
