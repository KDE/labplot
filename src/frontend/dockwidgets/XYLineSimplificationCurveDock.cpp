/*
	File             : XYLineSimplificationCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of line simplification curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYLineSimplificationCurveDock.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QProgressBar>
#include <QStatusBar>

/*!
  \class XYLineSimplificationCurveDock
 \brief  Provides a widget for editing the properties of the XYLineSimplificationCurves
		(2D-curves defined by an line simplification) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/

XYLineSimplificationCurveDock::XYLineSimplificationCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYLineSimplificationCurveDock::setupGeneral() {
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
	uiGeneralTab.sbTolerance->setRange(0.0, std::numeric_limits<double>::max());
	uiGeneralTab.sbTolerance2->setRange(0.0, std::numeric_limits<double>::max());

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	updateLocale();
	retranslateUi();

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYLineSimplificationCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYLineSimplificationCurveDock::methodChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYLineSimplificationCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYLineSimplificationCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYLineSimplificationCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYLineSimplificationCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYLineSimplificationCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.chkAuto, &QCheckBox::clicked, this, &XYLineSimplificationCurveDock::autoToleranceChanged);
	connect(uiGeneralTab.sbTolerance, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYLineSimplificationCurveDock::toleranceChanged);
	connect(uiGeneralTab.chkAuto2, &QCheckBox::clicked, this, &XYLineSimplificationCurveDock::autoTolerance2Changed);
	connect(uiGeneralTab.sbTolerance2, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYLineSimplificationCurveDock::tolerance2Changed);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYLineSimplificationCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYLineSimplificationCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYLineSimplificationCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYLineSimplificationCurveDock::yDataColumnChanged);
}

void XYLineSimplificationCurveDock::initGeneralTab() {
	// show the properties of the first curve
	// data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_lineSimplificationCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_lineSimplificationCurve->dataSourceCurve());
	cbXDataColumn->setAspect(m_lineSimplificationCurve->xDataColumn(), m_lineSimplificationCurve->xDataColumnPath());
	cbYDataColumn->setAspect(m_lineSimplificationCurve->yDataColumn(), m_lineSimplificationCurve->yDataColumnPath());

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_lineSimplificationCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(m_lineSimplificationData.xRange.first()));
		uiGeneralTab.leMax->setText(numberLocale.toString(m_lineSimplificationData.xRange.last()));
	} else {
		uiGeneralTab.dateTimeEditMin->setMSecsSinceEpochUTC(m_lineSimplificationData.xRange.first());
		uiGeneralTab.dateTimeEditMax->setMSecsSinceEpochUTC(m_lineSimplificationData.xRange.last());
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
	uiGeneralTab.cbAutoRange->setChecked(m_lineSimplificationData.autoRange);
	this->autoRangeChanged();

	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbMethod->setCurrentIndex(m_lineSimplificationData.type);
	this->methodChanged(m_lineSimplificationData.type);
	uiGeneralTab.chkAuto->setChecked(m_lineSimplificationData.autoTolerance);
	this->autoToleranceChanged();
	uiGeneralTab.sbTolerance->setValue(m_lineSimplificationData.tolerance);
	this->toleranceChanged(m_lineSimplificationData.tolerance);
	uiGeneralTab.chkAuto2->setChecked(m_lineSimplificationData.autoTolerance2);
	this->autoTolerance2Changed();
	uiGeneralTab.sbTolerance2->setValue(m_lineSimplificationData.tolerance2);
	this->tolerance2Changed(m_lineSimplificationData.tolerance2);

	this->showLineSimplificationResult();

	// enable the "recalculate"-button if the source data was changed since the last lineSimplification
	uiGeneralTab.pbRecalculate->setEnabled(m_lineSimplificationCurve->isSourceDataChangedSinceLastRecalc());

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_lineSimplificationCurve, &XYLineSimplificationCurve::dataSourceTypeChanged, this, &XYLineSimplificationCurveDock::curveDataSourceTypeChanged);
	connect(m_lineSimplificationCurve, &XYLineSimplificationCurve::dataSourceCurveChanged, this, &XYLineSimplificationCurveDock::curveDataSourceCurveChanged);
	connect(m_lineSimplificationCurve, &XYLineSimplificationCurve::xDataColumnChanged, this, &XYLineSimplificationCurveDock::curveXDataColumnChanged);
	connect(m_lineSimplificationCurve, &XYLineSimplificationCurve::yDataColumnChanged, this, &XYLineSimplificationCurveDock::curveYDataColumnChanged);
	connect(m_lineSimplificationCurve, &XYLineSimplificationCurve::lineSimplificationDataChanged, this, &XYLineSimplificationCurveDock::curveLineSimplificationDataChanged);
	connect(m_lineSimplificationCurve, &XYLineSimplificationCurve::sourceDataChanged, this, &XYLineSimplificationCurveDock::enableRecalculate);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYLineSimplificationCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	setAnalysisCurves(list);
	m_lineSimplificationCurve = static_cast<XYLineSimplificationCurve*>(m_curve);
	m_lineSimplificationData = m_lineSimplificationCurve->lineSimplificationData();

	const auto numberLocale = QLocale();
	uiGeneralTab.sbTolerance->setLocale(numberLocale);
	uiGeneralTab.sbTolerance2->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	setSymbols(list);

	updatePlotRangeList();
}

void XYLineSimplificationCurveDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;
	ui.retranslateUi(this);

	uiGeneralTab.cbMethod->clear();
	for (int i = 0; i < NSL_GEOM_LINESIM_TYPE_COUNT; ++i)
		uiGeneralTab.cbMethod->addItem(i18n(nsl_geom_linesim_type_name[i]));
	uiGeneralTab.cbMethod->setItemData(nsl_geom_linesim_type_visvalingam_whyatt, i18n("This method is much slower than any other"), Qt::ToolTipRole);

	// tooltip texts
	// note for the i18n-team: some algorithm are named after their creators like Douglas and Peucker, Visvalingam and Whyatt, Opheim, Lang
	QString info = i18n("Method to simplify the line:"
		"<ul>"
		"<li>Douglas-Peucker - recursively divides the line and retains only those points that deviate more than a specified threshold from the line segment between the start and end points.</li>"
		"<li>Visvalingam-Whyatt - iteratively removes the point that contributes the least effective area to the polygon's shape until a desired number of points or area threshold is reached.</li>"
		"<li>Perpendicular Distance - measures the perpendicular distance of each point from a reference line and retains only those exceeding a threshold.</li>"
		"<li>n-th Point - keeps every n-th point only.</li>"
		"<li>Radial Distance - measures the distance of each point from the previous retained point and retains only those exceeding a threshold.</li>"
		"<li>Interpolation.</li>"
		"<li>Opheim - uses both distance and angular tolerance to simplify lines. It retains points only if they fall outside a specified distance and angular threshold from the previous retained point.</li>"
		"<li>Lang - retaines points that are at least a specified distance apart and ensuring no point deviates more than a set tolerance from the simplified line.</li>"
		"</ul>"
	);

	uiGeneralTab.lMethod->setToolTip(info);
	uiGeneralTab.cbMethod->setToolTip(info);

	updateOptionsTexts();
}

/*!
 * updates the names and the tooltip texts for the widgets used for the parameters/options
 * of the currently selected line simplification method.
 */
void XYLineSimplificationCurveDock::updateOptionsTexts() {
	const auto type = (nsl_geom_linesim_type)uiGeneralTab.cbMethod->currentIndex();

	switch (type) {
	case nsl_geom_linesim_type_douglas_peucker:
		uiGeneralTab.lOption->setText(i18n("Tolerance (distance):"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Maximum perpendicular distance from the connecting line segment"));
		break;
	case nsl_geom_linesim_type_raddist:
		uiGeneralTab.lOption->setText(i18n("Tolerance (distance):"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Maximum radial distance from the previous retained point"));
		break;
	case nsl_geom_linesim_type_interp:
	case nsl_geom_linesim_type_reumann_witkam:
		uiGeneralTab.lOption->setText(i18n("Tolerance (distance):"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Maximum perpendicular distance from the line segment"));
		break;
	case nsl_geom_linesim_type_douglas_peucker_variant:
		uiGeneralTab.lOption->setText(i18n("Number of points:"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Target number of points in the simplified line"));
		break;
	case nsl_geom_linesim_type_nthpoint:
		uiGeneralTab.lOption->setText(i18n("Step size:"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Keep every n-th point (n is the step size)"));
		break;
	case nsl_geom_linesim_type_perpdist:
		uiGeneralTab.lOption->setText(i18n("Tolerance (distance):"));
		uiGeneralTab.lOption2->setText(i18n("Repeats:"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Maximum perpendicular distance from the reference line"));
		uiGeneralTab.sbTolerance2->setToolTip(i18n("Number of iterations to apply the simplification"));
		break;
	case nsl_geom_linesim_type_visvalingam_whyatt:
		uiGeneralTab.lOption->setText(i18n("Tolerance (area):"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Minimum area contribution threshold for retaining points"));
		break;
	case nsl_geom_linesim_type_opheim:
		uiGeneralTab.lOption->setText(i18n("Minimum tolerance:"));
		uiGeneralTab.lOption2->setText(i18n("Maximum tolerance:"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Minimum distance threshold from retained point"));
		uiGeneralTab.sbTolerance2->setToolTip(i18n("Maximum distance threshold from retained point"));
		break;
	case nsl_geom_linesim_type_lang:
		uiGeneralTab.lOption->setText(i18n("Tolerance (distance):"));
		uiGeneralTab.lOption2->setText(i18n("Search region:"));
		uiGeneralTab.sbTolerance->setToolTip(i18n("Minimum point spacing distance"));
		uiGeneralTab.sbTolerance2->setToolTip(i18n("Search region size in units of points"));
		break;
	}
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void XYLineSimplificationCurveDock::updateLocale() {
	const auto numberLocale = QLocale();
	uiGeneralTab.sbTolerance->setLocale(numberLocale);
	uiGeneralTab.sbTolerance2->setLocale(numberLocale);
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYLineSimplificationCurveDock::dataSourceTypeChanged(int index) {
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
		static_cast<XYLineSimplificationCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

void XYLineSimplificationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	for (auto* curve : m_curvesList)
		static_cast<XYLineSimplificationCurve*>(curve)->setXDataColumn(column);

	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
		uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
	}

	updateSettings(column);
	enableRecalculate();
}

void XYLineSimplificationCurveDock::updateSettings(const AbstractColumn*) {
	updateTolerance();
}

void XYLineSimplificationCurveDock::updateTolerance() {
	const AbstractColumn* xDataColumn = nullptr;
	const AbstractColumn* yDataColumn = nullptr;
	if (m_lineSimplificationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		xDataColumn = m_lineSimplificationCurve->xDataColumn();
		yDataColumn = m_lineSimplificationCurve->yDataColumn();
	} else {
		if (m_lineSimplificationCurve->dataSourceCurve()) {
			xDataColumn = m_lineSimplificationCurve->dataSourceCurve()->xColumn();
			yDataColumn = m_lineSimplificationCurve->dataSourceCurve()->yColumn();
		}
	}

	if (xDataColumn == nullptr || yDataColumn == nullptr)
		return;

	// copy all valid data points for calculating tolerance to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	const double xmin = m_lineSimplificationData.xRange.first();
	const double xmax = m_lineSimplificationData.xRange.last();
	XYAnalysisCurve::copyData(xdataVector, ydataVector, xDataColumn, yDataColumn, xmin, xmax);

	if (xdataVector.size() > 1)
		uiGeneralTab.cbMethod->setEnabled(true);
	else {
		uiGeneralTab.cbMethod->setEnabled(false);
		return;
	}
	DEBUG(Q_FUNC_INFO << ", set automatic tolerance:");
	DEBUG("clip_diag_perpoint =" << nsl_geom_linesim_clip_diag_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size()));
	DEBUG("clip_area_perpoint =" << nsl_geom_linesim_clip_area_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size()));
	DEBUG("avg_dist_perpoint =" << nsl_geom_linesim_avg_dist_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size()));

	const auto type = (nsl_geom_linesim_type)uiGeneralTab.cbMethod->currentIndex();
	if (type == nsl_geom_linesim_type_raddist || type == nsl_geom_linesim_type_opheim)
		m_lineSimplificationData.tolerance = 10. * nsl_geom_linesim_clip_diag_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size());
	else if (type == nsl_geom_linesim_type_visvalingam_whyatt)
		m_lineSimplificationData.tolerance = 0.1 * nsl_geom_linesim_clip_area_perpoint(xdataVector.data(), ydataVector.data(), (size_t)xdataVector.size());
	else if (type == nsl_geom_linesim_type_douglas_peucker_variant)
		m_lineSimplificationData.tolerance = xdataVector.size() / 10.; // reduction to 10%
	else
		m_lineSimplificationData.tolerance = 2. * nsl_geom_linesim_avg_dist_perpoint(xdataVector.data(), ydataVector.data(), xdataVector.size());
	// m_lineSimplificationData.tolerance = nsl_geom_linesim_clip_diag_perpoint(xdataVector.data(), ydataVector.data(), xdataVector.size());
	uiGeneralTab.sbTolerance->setValue(m_lineSimplificationData.tolerance);
	DEBUG(Q_FUNC_INFO << ", tolerance value = " << m_lineSimplificationData.tolerance)

	CONDITIONAL_LOCK_RETURN;

	// update data of curves
	for (auto* curve : m_curvesList)
		static_cast<XYLineSimplificationCurve*>(curve)->setLineSimplificationData(m_lineSimplificationData);
}

void XYLineSimplificationCurveDock::updateTolerance2() {
	const auto type = (nsl_geom_linesim_type)uiGeneralTab.cbMethod->currentIndex();

	if (type == nsl_geom_linesim_type_perpdist)
		uiGeneralTab.sbTolerance2->setValue(10);
	else if (type == nsl_geom_linesim_type_opheim)
		uiGeneralTab.sbTolerance2->setValue(5 * uiGeneralTab.sbTolerance->value());
	else if (type == nsl_geom_linesim_type_lang)
		uiGeneralTab.sbTolerance2->setValue(10);
}

void XYLineSimplificationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_lineSimplificationData.autoRange = autoRange;

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
		if (m_lineSimplificationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_lineSimplificationCurve->xDataColumn();
		else {
			if (m_lineSimplificationCurve->dataSourceCurve())
				xDataColumn = m_lineSimplificationCurve->dataSourceCurve()->xColumn();
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

void XYLineSimplificationCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_lineSimplificationData.xRange.first(), uiGeneralTab.leMin);
}

void XYLineSimplificationCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_lineSimplificationData.xRange.last(), uiGeneralTab.leMax);
}

void XYLineSimplificationCurveDock::xRangeMinDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_lineSimplificationData.xRange.first() = value;
	enableRecalculate();
}

void XYLineSimplificationCurveDock::xRangeMaxDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_lineSimplificationData.xRange.last() = value;
	enableRecalculate();
}

void XYLineSimplificationCurveDock::methodChanged(int index) {
	const auto type = (nsl_geom_linesim_type)index;
	m_lineSimplificationData.type = type;

	switch (type) {
	case nsl_geom_linesim_type_douglas_peucker:
	case nsl_geom_linesim_type_raddist:
	case nsl_geom_linesim_type_interp:
	case nsl_geom_linesim_type_reumann_witkam:
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
		uiGeneralTab.sbTolerance->setValue(10);
		uiGeneralTab.sbTolerance->setDecimals(0);
		uiGeneralTab.sbTolerance->setMinimum(1);
		uiGeneralTab.sbTolerance->setSingleStep(1);
		uiGeneralTab.lOption2->hide();
		uiGeneralTab.chkAuto2->hide();
		uiGeneralTab.sbTolerance2->hide();
		break;
	case nsl_geom_linesim_type_perpdist: // repeat option
		uiGeneralTab.sbTolerance->setDecimals(6);
		uiGeneralTab.sbTolerance->setMinimum(0);
		uiGeneralTab.sbTolerance->setSingleStep(0.01);
		uiGeneralTab.sbTolerance2->show();
		uiGeneralTab.lOption2->show();
		uiGeneralTab.chkAuto2->show();
		uiGeneralTab.sbTolerance2->setDecimals(0);
		uiGeneralTab.sbTolerance2->setMinimum(1);
		uiGeneralTab.sbTolerance2->setSingleStep(1);
		if (uiGeneralTab.chkAuto->isChecked())
			updateTolerance();
		if (uiGeneralTab.chkAuto2->isChecked())
			updateTolerance2();
		break;
	case nsl_geom_linesim_type_visvalingam_whyatt:
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
		uiGeneralTab.sbTolerance->setDecimals(6);
		uiGeneralTab.sbTolerance->setMinimum(0);
		uiGeneralTab.sbTolerance->setSingleStep(0.01);
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
		uiGeneralTab.sbTolerance->setDecimals(6);
		uiGeneralTab.sbTolerance->setMinimum(0);
		uiGeneralTab.sbTolerance->setSingleStep(0.01);
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

	updateOptionsTexts();
	enableRecalculate();
}

void XYLineSimplificationCurveDock::autoToleranceChanged() {
	const auto autoTolerance = (bool)uiGeneralTab.chkAuto->isChecked();
	m_lineSimplificationData.autoTolerance = autoTolerance;

	if (autoTolerance) {
		uiGeneralTab.sbTolerance->setEnabled(false);
		updateTolerance();
	} else
		uiGeneralTab.sbTolerance->setEnabled(true);
}

void XYLineSimplificationCurveDock::toleranceChanged(double value) {
	m_lineSimplificationData.tolerance = value;
	enableRecalculate();
}

void XYLineSimplificationCurveDock::autoTolerance2Changed() {
	const auto autoTolerance2 = (bool)uiGeneralTab.chkAuto2->isChecked();
	m_lineSimplificationData.autoTolerance2 = autoTolerance2;

	if (autoTolerance2) {
		uiGeneralTab.sbTolerance2->setEnabled(false);
		updateTolerance2();
	} else
		uiGeneralTab.sbTolerance2->setEnabled(true);
}

void XYLineSimplificationCurveDock::tolerance2Changed(double value) {
	m_lineSimplificationData.tolerance2 = value;
	enableRecalculate();
}

void XYLineSimplificationCurveDock::recalculateClicked() {
	for (auto* curve : m_curvesList)
		static_cast<XYLineSimplificationCurve*>(curve)->setLineSimplificationData(m_lineSimplificationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Status: %1", m_lineSimplificationCurve->lineSimplificationResult().status));
}

/*!
 * show the result and details of the lineSimplification
 */
void XYLineSimplificationCurveDock::showLineSimplificationResult() {
	showResult(m_lineSimplificationCurve, uiGeneralTab.teResult);
}

QString XYLineSimplificationCurveDock::customText() const {
	const auto& result = m_lineSimplificationCurve->lineSimplificationResult();
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
void XYLineSimplificationCurveDock::curveLineSimplificationDataChanged(const XYLineSimplificationCurve::LineSimplificationData& lineSimplificationData) {
	CONDITIONAL_LOCK_RETURN;
	m_lineSimplificationData = lineSimplificationData;
	// uiGeneralTab.cbMethod->setCurrentIndex(m_lineSimplificationData.type);
	// this->typeChanged();

	this->showLineSimplificationResult();
}
