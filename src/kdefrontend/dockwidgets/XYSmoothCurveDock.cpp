/*
	File             : XYSmoothCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of smooth curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017-2020 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYSmoothCurveDock.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QStandardItemModel>

/*!
  \class XYSmoothCurveDock
 \brief  Provides a widget for editing the properties of the XYSmoothCurves
		(2D-curves defined by an smooth) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYSmoothCurveDock::XYSmoothCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYSmoothCurveDock::setupGeneral() {
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

	for (int i = 0; i < NSL_SMOOTH_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_smooth_type_name[i]));

	for (int i = 0; i < NSL_SMOOTH_WEIGHT_TYPE_COUNT; i++)
		uiGeneralTab.cbWeight->addItem(i18n(nsl_smooth_weight_type_name[i]));

	for (int i = 0; i < NSL_SMOOTH_PAD_MODE_COUNT; i++)
		uiGeneralTab.cbMode->addItem(i18n(nsl_smooth_pad_mode_name[i]));

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYSmoothCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYSmoothCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYSmoothCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYSmoothCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYSmoothCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYSmoothCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYSmoothCurveDock::typeChanged);
	// TODO: line edits?
	connect(uiGeneralTab.sbPoints, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYSmoothCurveDock::pointsChanged);
	connect(uiGeneralTab.cbWeight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYSmoothCurveDock::weightChanged);
	connect(uiGeneralTab.sbPercentile, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYSmoothCurveDock::percentileChanged);
	connect(uiGeneralTab.sbOrder, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYSmoothCurveDock::orderChanged);
	connect(uiGeneralTab.cbMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYSmoothCurveDock::modeChanged);
	connect(uiGeneralTab.sbLeftValue, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYSmoothCurveDock::valueChanged);
	connect(uiGeneralTab.sbRightValue, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYSmoothCurveDock::valueChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYSmoothCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYSmoothCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYSmoothCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYSmoothCurveDock::yDataColumnChanged);
}

void XYSmoothCurveDock::initGeneralTab() {
	// show the properties of the first curve
	// data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_smoothCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_smoothCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_smoothCurve->xDataColumn(), m_smoothCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_smoothCurve->yDataColumn(), m_smoothCurve->yDataColumnPath());

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_smoothCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(m_smoothData.xRange.first()));
		uiGeneralTab.leMax->setText(numberLocale.toString(m_smoothData.xRange.last()));
	} else {
		uiGeneralTab.dateTimeEditMin->setMSecsSinceEpochUTC(m_smoothData.xRange.first());
		uiGeneralTab.dateTimeEditMax->setMSecsSinceEpochUTC(m_smoothData.xRange.last());
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
	uiGeneralTab.cbAutoRange->setChecked(m_smoothData.autoRange);
	this->autoRangeChanged();

	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbType->setCurrentIndex(m_smoothData.type);
	typeChanged(uiGeneralTab.cbType->currentIndex()); // needed, when type does not change
	uiGeneralTab.sbPoints->setValue((int)m_smoothData.points);
	uiGeneralTab.cbWeight->setCurrentIndex(m_smoothData.weight);
	uiGeneralTab.sbPercentile->setValue(m_smoothData.percentile);
	uiGeneralTab.sbOrder->setValue((int)m_smoothData.order);
	uiGeneralTab.cbMode->setCurrentIndex(m_smoothData.mode);
	modeChanged(uiGeneralTab.cbMode->currentIndex()); // needed, when mode does not change
	uiGeneralTab.sbLeftValue->setValue(m_smoothData.lvalue);
	uiGeneralTab.sbRightValue->setValue(m_smoothData.rvalue);
	valueChanged();
	this->showSmoothResult();

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_smoothCurve, &XYSmoothCurve::dataSourceTypeChanged, this, &XYSmoothCurveDock::curveDataSourceTypeChanged);
	connect(m_smoothCurve, &XYSmoothCurve::dataSourceCurveChanged, this, &XYSmoothCurveDock::curveDataSourceCurveChanged);
	connect(m_smoothCurve, &XYSmoothCurve::xDataColumnChanged, this, &XYSmoothCurveDock::curveXDataColumnChanged);
	connect(m_smoothCurve, &XYSmoothCurve::yDataColumnChanged, this, &XYSmoothCurveDock::curveYDataColumnChanged);
	connect(m_smoothCurve, &XYSmoothCurve::smoothDataChanged, this, &XYSmoothCurveDock::curveSmoothDataChanged);
	connect(m_smoothCurve, &XYSmoothCurve::sourceDataChanged, this, &XYSmoothCurveDock::enableRecalculate);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYSmoothCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	setAnalysisCurves(list);
	m_smoothCurve = static_cast<XYSmoothCurve*>(m_curve);
	m_smoothData = m_smoothCurve->smoothData();

	const auto numberLocale = QLocale();
	uiGeneralTab.sbPercentile->setLocale(numberLocale);
	uiGeneralTab.sbLeftValue->setLocale(numberLocale);
	uiGeneralTab.sbRightValue->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	setSymbols(list);

	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYSmoothCurveDock::dataSourceTypeChanged(int index) {
	auto type = (XYAnalysisCurve::DataSourceType)index;
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
		static_cast<XYSmoothCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

void XYSmoothCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	for (auto* curve : m_curvesList)
		static_cast<XYSmoothCurve*>(curve)->setXDataColumn(column);

	// disable types that need more data points
	if (column) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			const auto numberLocale = QLocale();
			uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
			uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
		}

		updateSettings(column);
	}

	enableRecalculate();
}

void XYSmoothCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	// set maximum of sbPoints to the number of valid rows in the data column for x
	const auto& statistics = static_cast<const Column*>(column)->statistics();
	uiGeneralTab.sbPoints->setMaximum(statistics.size);
}

void XYSmoothCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_smoothData.autoRange = autoRange;

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
		if (m_smoothCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_smoothCurve->xDataColumn();
		else {
			if (m_smoothCurve->dataSourceCurve())
				xDataColumn = m_smoothCurve->dataSourceCurve()->xColumn();
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

void XYSmoothCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_smoothData.xRange.first(), uiGeneralTab.leMin);
}

void XYSmoothCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_smoothData.xRange.last(), uiGeneralTab.leMax);
}

void XYSmoothCurveDock::xRangeMinDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_smoothData.xRange.first() = value;
	enableRecalculate();
}

void XYSmoothCurveDock::xRangeMaxDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_smoothData.xRange.last() = value;
	enableRecalculate();
}

void XYSmoothCurveDock::typeChanged(int index) {
	auto type = (nsl_smooth_type)index;
	m_smoothData.type = type;

	const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbMode->model());
	auto* pad_interp_item = model->item(nsl_smooth_pad_interp);
	if (type == nsl_smooth_type_moving_average || type == nsl_smooth_type_moving_average_lagged) {
		uiGeneralTab.lWeight->show();
		uiGeneralTab.cbWeight->show();
		// disable interp pad model for MA and MAL
		pad_interp_item->setFlags(pad_interp_item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	} else {
		uiGeneralTab.lWeight->hide();
		uiGeneralTab.cbWeight->hide();
		pad_interp_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}

	if (type == nsl_smooth_type_moving_average_lagged) {
		uiGeneralTab.sbPoints->setSingleStep(1);
		uiGeneralTab.sbPoints->setMinimum(2);
		uiGeneralTab.lRightValue->hide();
		uiGeneralTab.sbRightValue->hide();
	} else {
		uiGeneralTab.sbPoints->setSingleStep(2);
		uiGeneralTab.sbPoints->setMinimum(3);
		if (m_smoothData.mode == nsl_smooth_pad_constant) {
			uiGeneralTab.lRightValue->show();
			uiGeneralTab.sbRightValue->show();
		}
	}

	if (type == nsl_smooth_type_percentile) {
		uiGeneralTab.lPercentile->show();
		uiGeneralTab.sbPercentile->show();
		// disable interp pad model for MA and MAL
		pad_interp_item->setFlags(pad_interp_item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	} else {
		uiGeneralTab.lPercentile->hide();
		uiGeneralTab.sbPercentile->hide();
	}

	if (type == nsl_smooth_type_savitzky_golay) {
		uiGeneralTab.lOrder->show();
		uiGeneralTab.sbOrder->show();
	} else {
		uiGeneralTab.lOrder->hide();
		uiGeneralTab.sbOrder->hide();
	}

	enableRecalculate();
}

void XYSmoothCurveDock::pointsChanged(int value) {
	m_smoothData.points = (unsigned int)value;
	uiGeneralTab.sbOrder->setMaximum(value - 1); // set maximum order
	enableRecalculate();
}

void XYSmoothCurveDock::weightChanged(int index) {
	m_smoothData.weight = (nsl_smooth_weight_type)index;
	enableRecalculate();
}

void XYSmoothCurveDock::percentileChanged(double value) {
	m_smoothData.percentile = value;
	enableRecalculate();
}

void XYSmoothCurveDock::orderChanged(int value) {
	m_smoothData.order = (unsigned int)value;
	enableRecalculate();
}

void XYSmoothCurveDock::modeChanged(int index) {
	m_smoothData.mode = (nsl_smooth_pad_mode)index;

	if (m_smoothData.mode == nsl_smooth_pad_constant) {
		uiGeneralTab.lLeftValue->show();
		uiGeneralTab.sbLeftValue->show();
		if (m_smoothData.type == nsl_smooth_type_moving_average_lagged) {
			uiGeneralTab.lRightValue->hide();
			uiGeneralTab.sbRightValue->hide();
		} else {
			uiGeneralTab.lRightValue->show();
			uiGeneralTab.sbRightValue->show();
		}
	} else {
		uiGeneralTab.lLeftValue->hide();
		uiGeneralTab.sbLeftValue->hide();
		uiGeneralTab.lRightValue->hide();
		uiGeneralTab.sbRightValue->hide();
	}

	enableRecalculate();
}

void XYSmoothCurveDock::valueChanged() {
	m_smoothData.lvalue = uiGeneralTab.sbLeftValue->value();
	m_smoothData.rvalue = uiGeneralTab.sbRightValue->value();

	enableRecalculate();
}

void XYSmoothCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		static_cast<XYSmoothCurve*>(curve)->setSmoothData(m_smoothData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Smoothing status: %1", m_smoothCurve->result().status));
	QApplication::restoreOverrideCursor();
}

/*!
 * show the result and details of the smooth
 */
void XYSmoothCurveDock::showSmoothResult() {
	showResult(m_smoothCurve, uiGeneralTab.teResult);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYSmoothCurveDock::curveSmoothDataChanged(const XYSmoothCurve::SmoothData& smoothData) {
	CONDITIONAL_LOCK_RETURN;
	m_smoothData = smoothData;
	uiGeneralTab.cbType->setCurrentIndex(m_smoothData.type);

	this->showSmoothResult();
}
