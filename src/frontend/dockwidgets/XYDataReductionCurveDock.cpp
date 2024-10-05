/*
	File             : XYDataReductionCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of data reduction curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYDataReductionCurveDock.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QProgressBar>
#include <QStatusBar>
#include <QWidgetAction>

/*!
  \class XYDataReductionCurveDock
 \brief  Provides a widget for editing the properties of the XYDataReductionCurves
		(2D-curves defined by an data reduction) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/

XYDataReductionCurveDock::XYDataReductionCurveDock(QWidget* parent, QStatusBar* sb)
	: XYAnalysisCurveDock(parent)
	, statusBar(sb) {
}

/*!
 * 	// Tab "General"
 */
void XYDataReductionCurveDock::setupGeneral() {
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

	for (int i = 0; i < NSL_GEOM_LINESIM_TYPE_COUNT; ++i)
		uiGeneralTab.cbType->addItem(i18n(nsl_geom_linesim_type_name[i]));
	uiGeneralTab.cbType->setItemData(nsl_geom_linesim_type_visvalingam_whyatt, i18n("This method is much slower than any other"), Qt::ToolTipRole);

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));
	uiGeneralTab.sbTolerance->setRange(0.0, std::numeric_limits<double>::max());
	uiGeneralTab.sbTolerance2->setRange(0.0, std::numeric_limits<double>::max());

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYDataReductionCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYDataReductionCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYDataReductionCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYDataReductionCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYDataReductionCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYDataReductionCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.chkAuto, &QCheckBox::clicked, this, &XYDataReductionCurveDock::autoToleranceChanged);
	connect(uiGeneralTab.sbTolerance, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYDataReductionCurveDock::toleranceChanged);
	connect(uiGeneralTab.chkAuto2, &QCheckBox::clicked, this, &XYDataReductionCurveDock::autoTolerance2Changed);
	connect(uiGeneralTab.sbTolerance2, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYDataReductionCurveDock::tolerance2Changed);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYDataReductionCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYDataReductionCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYDataReductionCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYDataReductionCurveDock::yDataColumnChanged);
}

void XYDataReductionCurveDock::initGeneralTab() {
	// show the properties of the first curve
	// data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_dataReductionCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_dataReductionCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_dataReductionCurve->xDataColumn(), m_dataReductionCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_dataReductionCurve->yDataColumn(), m_dataReductionCurve->yDataColumnPath());

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_dataReductionCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(m_dataReductionData.xRange.first()));
		uiGeneralTab.leMax->setText(numberLocale.toString(m_dataReductionData.xRange.last()));
	} else {
		uiGeneralTab.dateTimeEditMin->setMSecsSinceEpochUTC(m_dataReductionData.xRange.first());
		uiGeneralTab.dateTimeEditMax->setMSecsSinceEpochUTC(m_dataReductionData.xRange.last());
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
	uiGeneralTab.cbAutoRange->setChecked(m_dataReductionData.autoRange);
	this->autoRangeChanged();

	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbType->setCurrentIndex(m_dataReductionData.type);
	this->typeChanged(m_dataReductionData.type);
	uiGeneralTab.chkAuto->setChecked(m_dataReductionData.autoTolerance);
	this->autoToleranceChanged();
	uiGeneralTab.sbTolerance->setValue(m_dataReductionData.tolerance);
	this->toleranceChanged(m_dataReductionData.tolerance);
	uiGeneralTab.chkAuto2->setChecked(m_dataReductionData.autoTolerance2);
	this->autoTolerance2Changed();
	uiGeneralTab.sbTolerance2->setValue(m_dataReductionData.tolerance2);
	this->tolerance2Changed(m_dataReductionData.tolerance2);

	this->showDataReductionResult();

	// enable the "recalculate"-button if the source data was changed since the last dataReduction
	uiGeneralTab.pbRecalculate->setEnabled(m_dataReductionCurve->isSourceDataChangedSinceLastRecalc());

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_dataReductionCurve, &XYDataReductionCurve::dataSourceTypeChanged, this, &XYDataReductionCurveDock::curveDataSourceTypeChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::dataSourceCurveChanged, this, &XYDataReductionCurveDock::curveDataSourceCurveChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::xDataColumnChanged, this, &XYDataReductionCurveDock::curveXDataColumnChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::yDataColumnChanged, this, &XYDataReductionCurveDock::curveYDataColumnChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::dataReductionDataChanged, this, &XYDataReductionCurveDock::curveDataReductionDataChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::sourceDataChanged, this, &XYDataReductionCurveDock::enableRecalculate);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYDataReductionCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	setAnalysisCurves(list);
	m_dataReductionCurve = static_cast<XYDataReductionCurve*>(m_curve);
	m_dataReductionData = m_dataReductionCurve->dataReductionData();

	const auto numberLocale = QLocale();
	uiGeneralTab.sbTolerance->setLocale(numberLocale);
	uiGeneralTab.sbTolerance2->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	setSymbols(list);

	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYDataReductionCurveDock::dataSourceTypeChanged(int index) {
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
		static_cast<XYDataReductionCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

void XYDataReductionCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	for (auto* curve : m_curvesList)
		static_cast<XYDataReductionCurve*>(curve)->setXDataColumn(column);

	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
		uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
	}

	updateSettings(column);
	enableRecalculate();
}

void XYDataReductionCurveDock::updateSettings(const AbstractColumn*) {
	updateTolerance();
}

void XYDataReductionCurveDock::updateTolerance() {
	const AbstractColumn* xDataColumn = nullptr;
	const AbstractColumn* yDataColumn = nullptr;
	if (m_dataReductionCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		xDataColumn = m_dataReductionCurve->xDataColumn();
		yDataColumn = m_dataReductionCurve->yDataColumn();
	} else {
		if (m_dataReductionCurve->dataSourceCurve()) {
			xDataColumn = m_dataReductionCurve->dataSourceCurve()->xColumn();
			yDataColumn = m_dataReductionCurve->dataSourceCurve()->yColumn();
		}
	}

	if (xDataColumn == nullptr || yDataColumn == nullptr)
		return;

	// copy all valid data points for calculating tolerance to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	const double xmin = m_dataReductionData.xRange.first();
	const double xmax = m_dataReductionData.xRange.last();
	XYAnalysisCurve::copyData(xdataVector, ydataVector, xDataColumn, yDataColumn, xmin, xmax);

	if (xdataVector.size() > 1)
		uiGeneralTab.cbType->setEnabled(true);
	else {
		uiGeneralTab.cbType->setEnabled(false);
		return;
	}
	DEBUG("automatic tolerance:");
	DEBUG("clip_diag_perpoint =" << nsl_geom_linesim_clip_diag_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size()));
	DEBUG("clip_area_perpoint =" << nsl_geom_linesim_clip_area_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size()));
	DEBUG("avg_dist_perpoint =" << nsl_geom_linesim_avg_dist_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size()));

	const auto type = (nsl_geom_linesim_type)uiGeneralTab.cbType->currentIndex();
	if (type == nsl_geom_linesim_type_raddist || type == nsl_geom_linesim_type_opheim)
		m_dataReductionData.tolerance = 10. * nsl_geom_linesim_clip_diag_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size());
	else if (type == nsl_geom_linesim_type_visvalingam_whyatt)
		m_dataReductionData.tolerance = 0.1 * nsl_geom_linesim_clip_area_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size());
	else if (type == nsl_geom_linesim_type_douglas_peucker_variant)
		m_dataReductionData.tolerance = xdataVector.size() / 10.; // reduction to 10%
	else
		m_dataReductionData.tolerance = 2. * nsl_geom_linesim_avg_dist_perpoint(xdataVector.data(), ydataVector.data(), xdataVector.size());
	// m_dataReductionData.tolerance = nsl_geom_linesim_clip_diag_perpoint(xdataVector.data(), ydataVector.data(), xdataVector.size());
	uiGeneralTab.sbTolerance->setValue(m_dataReductionData.tolerance);
}

void XYDataReductionCurveDock::updateTolerance2() {
	const auto type = (nsl_geom_linesim_type)uiGeneralTab.cbType->currentIndex();

	if (type == nsl_geom_linesim_type_perpdist)
		uiGeneralTab.sbTolerance2->setValue(10);
	else if (type == nsl_geom_linesim_type_opheim)
		uiGeneralTab.sbTolerance2->setValue(5 * uiGeneralTab.sbTolerance->value());
	else if (type == nsl_geom_linesim_type_lang)
		uiGeneralTab.sbTolerance2->setValue(10);
}

void XYDataReductionCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_dataReductionData.autoRange = autoRange;

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
		if (m_dataReductionCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_dataReductionCurve->xDataColumn();
		else {
			if (m_dataReductionCurve->dataSourceCurve())
				xDataColumn = m_dataReductionCurve->dataSourceCurve()->xColumn();
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

void XYDataReductionCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_dataReductionData.xRange.first(), uiGeneralTab.leMin);
}

void XYDataReductionCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_dataReductionData.xRange.last(), uiGeneralTab.leMax);
}

void XYDataReductionCurveDock::xRangeMinDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_dataReductionData.xRange.first() = value;
	enableRecalculate();
}

void XYDataReductionCurveDock::xRangeMaxDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_dataReductionData.xRange.last() = value;
	enableRecalculate();
}

void XYDataReductionCurveDock::typeChanged(int index) {
	const auto type = (nsl_geom_linesim_type)index;
	m_dataReductionData.type = type;

	switch (type) {
	case nsl_geom_linesim_type_douglas_peucker:
	case nsl_geom_linesim_type_raddist:
	case nsl_geom_linesim_type_interp:
	case nsl_geom_linesim_type_reumann_witkam:
		uiGeneralTab.lOption->setText(i18n("Tolerance (distance):"));
		uiGeneralTab.sbTolerance->setDecimals(6);
		uiGeneralTab.sbTolerance->setMinimum(0);
		uiGeneralTab.sbTolerance->setSingleStep(0.01);
		uiGeneralTab.lOption2->hide();
		uiGeneralTab.chkAuto2->hide();
		uiGeneralTab.sbTolerance2->hide();
		if (uiGeneralTab.chkAuto->isChecked())
			updateTolerance();
		break;
	case nsl_geom_linesim_type_douglas_peucker_variant:
		uiGeneralTab.lOption->setText(i18n("Number of points:"));
		uiGeneralTab.sbTolerance->setDecimals(0);
		uiGeneralTab.sbTolerance->setMinimum(2);
		uiGeneralTab.sbTolerance->setSingleStep(1);
		uiGeneralTab.lOption2->hide();
		uiGeneralTab.chkAuto2->hide();
		uiGeneralTab.sbTolerance2->hide();
		if (uiGeneralTab.chkAuto->isChecked())
			updateTolerance();
		break;
	case nsl_geom_linesim_type_nthpoint:
		uiGeneralTab.lOption->setText(i18n("Step size:"));
		uiGeneralTab.sbTolerance->setValue(10);
		uiGeneralTab.sbTolerance->setDecimals(0);
		uiGeneralTab.sbTolerance->setMinimum(1);
		uiGeneralTab.sbTolerance->setSingleStep(1);
		uiGeneralTab.lOption2->hide();
		uiGeneralTab.chkAuto2->hide();
		uiGeneralTab.sbTolerance2->hide();
		break;
	case nsl_geom_linesim_type_perpdist: // repeat option
		uiGeneralTab.lOption->setText(i18n("Tolerance (distance):"));
		uiGeneralTab.sbTolerance->setDecimals(6);
		uiGeneralTab.sbTolerance->setMinimum(0);
		uiGeneralTab.sbTolerance->setSingleStep(0.01);
		uiGeneralTab.sbTolerance2->show();
		uiGeneralTab.lOption2->show();
		uiGeneralTab.chkAuto2->show();
		uiGeneralTab.lOption2->setText(i18n("Repeats:"));
		uiGeneralTab.sbTolerance2->setDecimals(0);
		uiGeneralTab.sbTolerance2->setMinimum(1);
		uiGeneralTab.sbTolerance2->setSingleStep(1);
		if (uiGeneralTab.chkAuto->isChecked())
			updateTolerance();
		if (uiGeneralTab.chkAuto2->isChecked())
			updateTolerance2();
		break;
	case nsl_geom_linesim_type_visvalingam_whyatt:
		uiGeneralTab.lOption->setText(i18n("Tolerance (area):"));
		uiGeneralTab.sbTolerance->setDecimals(6);
		uiGeneralTab.sbTolerance->setMinimum(0);
		uiGeneralTab.sbTolerance->setSingleStep(0.01);
		uiGeneralTab.lOption2->hide();
		uiGeneralTab.chkAuto2->hide();
		uiGeneralTab.sbTolerance2->hide();
		if (uiGeneralTab.chkAuto->isChecked())
			updateTolerance();
		break;
	case nsl_geom_linesim_type_opheim: // min/max tol options
		uiGeneralTab.lOption->setText(i18n("Minimum tolerance:"));
		uiGeneralTab.sbTolerance->setDecimals(6);
		uiGeneralTab.sbTolerance->setMinimum(0);
		uiGeneralTab.sbTolerance->setSingleStep(0.01);
		uiGeneralTab.lOption2->setText(i18n("Maximum tolerance:"));
		uiGeneralTab.lOption2->show();
		uiGeneralTab.chkAuto2->show();
		uiGeneralTab.sbTolerance2->show();
		uiGeneralTab.sbTolerance2->setDecimals(6);
		uiGeneralTab.sbTolerance2->setMinimum(0);
		uiGeneralTab.sbTolerance2->setSingleStep(0.01);
		if (uiGeneralTab.chkAuto->isChecked())
			updateTolerance();
		if (uiGeneralTab.chkAuto2->isChecked())
			updateTolerance2();
		break;
	case nsl_geom_linesim_type_lang: // distance/region
		uiGeneralTab.lOption->setText(i18n("Tolerance (distance):"));
		uiGeneralTab.sbTolerance->setDecimals(6);
		uiGeneralTab.sbTolerance->setMinimum(0);
		uiGeneralTab.sbTolerance->setSingleStep(0.01);
		uiGeneralTab.lOption2->setText(i18n("Search region:"));
		uiGeneralTab.lOption2->show();
		uiGeneralTab.chkAuto2->show();
		uiGeneralTab.sbTolerance2->show();
		uiGeneralTab.sbTolerance2->setDecimals(0);
		uiGeneralTab.sbTolerance2->setMinimum(1);
		uiGeneralTab.sbTolerance2->setSingleStep(1);
		if (uiGeneralTab.chkAuto->isChecked())
			updateTolerance();
		if (uiGeneralTab.chkAuto2->isChecked())
			updateTolerance2();
		break;
	}

	enableRecalculate();
}

void XYDataReductionCurveDock::autoToleranceChanged() {
	const auto autoTolerance = (bool)uiGeneralTab.chkAuto->isChecked();
	m_dataReductionData.autoTolerance = autoTolerance;

	if (autoTolerance) {
		uiGeneralTab.sbTolerance->setEnabled(false);
		updateTolerance();
	} else
		uiGeneralTab.sbTolerance->setEnabled(true);
}

void XYDataReductionCurveDock::toleranceChanged(double value) {
	m_dataReductionData.tolerance = value;
	enableRecalculate();
}

void XYDataReductionCurveDock::autoTolerance2Changed() {
	const auto autoTolerance2 = (bool)uiGeneralTab.chkAuto2->isChecked();
	m_dataReductionData.autoTolerance2 = autoTolerance2;

	if (autoTolerance2) {
		uiGeneralTab.sbTolerance2->setEnabled(false);
		updateTolerance2();
	} else
		uiGeneralTab.sbTolerance2->setEnabled(true);
}

void XYDataReductionCurveDock::tolerance2Changed(double value) {
	m_dataReductionData.tolerance2 = value;
	enableRecalculate();
}

void XYDataReductionCurveDock::recalculateClicked() {
	// show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	connect(m_curve, SIGNAL(completed(int)), progressBar, SLOT(setValue(int)));
	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		static_cast<XYDataReductionCurve*>(curve)->setDataReductionData(m_dataReductionData);

	QApplication::restoreOverrideCursor();
	statusBar->removeWidget(progressBar);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Data reduction status: %1", m_dataReductionCurve->dataReductionResult().status));
}

/*!
 * show the result and details of the dataReduction
 */
void XYDataReductionCurveDock::showDataReductionResult() {
	showResult(m_dataReductionCurve, uiGeneralTab.teResult);
}

QString XYDataReductionCurveDock::customText() const {
	const auto& result = m_dataReductionCurve->dataReductionResult();
	const auto numberLocale = QLocale();
	QString str = QStringLiteral("<br>");

	str += i18n("number of points: %1", numberLocale.toString(static_cast<qulonglong>(result.npoints))) + QStringLiteral("<br>");
	str += i18n("positional squared error: %1", numberLocale.toString(result.posError)) + QStringLiteral("<br>");
	str += i18n("area error: %1", numberLocale.toString(result.areaError)) + QStringLiteral("<br>");

	return str;
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYDataReductionCurveDock::curveDataReductionDataChanged(const XYDataReductionCurve::DataReductionData& dataReductionData) {
	CONDITIONAL_LOCK_RETURN;
	m_dataReductionData = dataReductionData;
	// uiGeneralTab.cbType->setCurrentIndex(m_dataReductionData.type);
	// this->typeChanged();

	this->showDataReductionResult();
}
