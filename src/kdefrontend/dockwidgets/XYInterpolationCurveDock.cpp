/*
	File             : XYInterpolationCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of interpolation curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2016-2017 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYInterpolationCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QStandardItemModel>
#include <QWidgetAction>

extern "C" {
#include <gsl/gsl_interp.h> // gsl_interp types
}
#include <cmath> // isnan

/*!
  \class XYInterpolationCurveDock
 \brief  Provides a widget for editing the properties of the XYInterpolationCurves
		(2D-curves defined by an interpolation) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYInterpolationCurveDock::XYInterpolationCurveDock(QWidget* parent)
	: XYCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYInterpolationCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_teComment = uiGeneralTab.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2, 2, 2, 2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 2);
	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 2);

	for (int i = 0; i < NSL_INTERP_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_interp_type_name[i]));
#if GSL_MAJOR_VERSION < 2
	// disable Steffen spline item
	const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbType->model());
	auto* item = model->item(nsl_interp_type_steffen);
	item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif
	for (int i = 0; i < NSL_INTERP_PCH_VARIANT_COUNT; i++)
		uiGeneralTab.cbVariant->addItem(i18n(nsl_interp_pch_variant_name[i]));
	for (int i = 0; i < NSL_INTERP_EVALUATE_COUNT; i++)
		uiGeneralTab.cbEval->addItem(i18n(nsl_interp_evaluate_name[i]));

	uiGeneralTab.cbPointsMode->addItem(i18n("Auto (5x data points)"));
	uiGeneralTab.cbPointsMode->addItem(i18n("Multiple of data points"));
	uiGeneralTab.cbPointsMode->addItem(i18n("Custom"));

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYInterpolationCurveDock::nameChanged);
	connect(uiGeneralTab.teComment, &QTextEdit::textChanged, this, &XYInterpolationCurveDock::commentChanged);
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYInterpolationCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYInterpolationCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYInterpolationCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYInterpolationCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYInterpolationCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &QDateTimeEdit::dateTimeChanged, this, &XYInterpolationCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &QDateTimeEdit::dateTimeChanged, this, &XYInterpolationCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYInterpolationCurveDock::typeChanged);
	connect(uiGeneralTab.cbVariant, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYInterpolationCurveDock::variantChanged);
	// TODO: use line edits?
	connect(uiGeneralTab.sbTension, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYInterpolationCurveDock::tensionChanged);
	connect(uiGeneralTab.sbContinuity, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYInterpolationCurveDock::continuityChanged);
	connect(uiGeneralTab.sbBias, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYInterpolationCurveDock::biasChanged);
	connect(uiGeneralTab.cbEval, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYInterpolationCurveDock::evaluateChanged);
	// double?
	connect(uiGeneralTab.sbPoints, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYInterpolationCurveDock::numberOfPointsChanged);
	connect(uiGeneralTab.cbPointsMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYInterpolationCurveDock::pointsModeChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYInterpolationCurveDock::recalculateClicked);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYInterpolationCurveDock::plotRangeChanged);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYInterpolationCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYInterpolationCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYInterpolationCurveDock::yDataColumnChanged);
}

void XYInterpolationCurveDock::initGeneralTab() {
	// if there are more then one curve in the list, disable the tab "general"
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

	// show the properties of the first curve
	// data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_interpolationCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_interpolationCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_interpolationCurve->xDataColumn(), m_interpolationCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_interpolationCurve->yDataColumn(), m_interpolationCurve->yDataColumnPath());

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_interpolationCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(m_interpolationData.xRange.first()));
		uiGeneralTab.leMax->setText(numberLocale.toString(m_interpolationData.xRange.last()));
	} else {
		uiGeneralTab.dateTimeEditMin->setDateTime(QDateTime::fromMSecsSinceEpoch(m_interpolationData.xRange.first()));
		uiGeneralTab.dateTimeEditMax->setDateTime(QDateTime::fromMSecsSinceEpoch(m_interpolationData.xRange.last()));
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
	uiGeneralTab.cbAutoRange->setChecked(m_interpolationData.autoRange);
	this->autoRangeChanged();

	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbType->setCurrentIndex(m_interpolationData.type);
	this->typeChanged(m_interpolationData.type);
	uiGeneralTab.cbVariant->setCurrentIndex(m_interpolationData.variant);
	this->variantChanged(m_interpolationData.variant);
	uiGeneralTab.sbTension->setValue(m_interpolationData.tension);
	uiGeneralTab.sbContinuity->setValue(m_interpolationData.continuity);
	uiGeneralTab.sbBias->setValue(m_interpolationData.bias);
	uiGeneralTab.cbEval->setCurrentIndex(m_interpolationData.evaluate);

	if (m_interpolationData.pointsMode == XYInterpolationCurve::PointsMode::Multiple)
		uiGeneralTab.sbPoints->setValue(m_interpolationData.npoints / 5.);
	else
		uiGeneralTab.sbPoints->setValue(m_interpolationData.npoints);
	uiGeneralTab.cbPointsMode->setCurrentIndex(static_cast<int>(m_interpolationData.pointsMode));

	this->showInterpolationResult();

	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_interpolationCurve, &XYInterpolationCurve::aspectDescriptionChanged, this, &XYInterpolationCurveDock::aspectDescriptionChanged);
	connect(m_interpolationCurve, &XYInterpolationCurve::dataSourceTypeChanged, this, &XYInterpolationCurveDock::curveDataSourceTypeChanged);
	connect(m_interpolationCurve, &XYInterpolationCurve::dataSourceCurveChanged, this, &XYInterpolationCurveDock::curveDataSourceCurveChanged);
	connect(m_interpolationCurve, &XYInterpolationCurve::xDataColumnChanged, this, &XYInterpolationCurveDock::curveXDataColumnChanged);
	connect(m_interpolationCurve, &XYInterpolationCurve::yDataColumnChanged, this, &XYInterpolationCurveDock::curveYDataColumnChanged);
	connect(m_interpolationCurve, &XYInterpolationCurve::interpolationDataChanged, this, &XYInterpolationCurveDock::curveInterpolationDataChanged);
	connect(m_interpolationCurve, &XYInterpolationCurve::sourceDataChanged, this, &XYInterpolationCurveDock::enableRecalculate);
	connect(m_interpolationCurve, &WorksheetElement::plotRangeListChanged, this, &XYInterpolationCurveDock::updatePlotRanges);
}

void XYInterpolationCurveDock::setModel() {
	QList<AspectType> list{AspectType::Folder,
						   AspectType::Datapicker,
						   AspectType::Worksheet,
						   AspectType::CartesianPlot,
						   AspectType::XYCurve,
						   AspectType::XYAnalysisCurve};
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list = {AspectType::Folder,
			AspectType::Workbook,
			AspectType::Datapicker,
			AspectType::DatapickerCurve,
			AspectType::Spreadsheet,
			AspectType::LiveDataSource,
			AspectType::Column,
			AspectType::Worksheet,
			AspectType::CartesianPlot,
			AspectType::XYFitCurve,
			AspectType::CantorWorksheet};
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);

	cbDataSourceCurve->setModel(m_aspectTreeModel);
	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYInterpolationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	m_interpolationCurve = static_cast<XYInterpolationCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_interpolationData = m_interpolationCurve->interpolationData();

	const auto numberLocale = QLocale();
	uiGeneralTab.sbTension->setLocale(numberLocale);
	uiGeneralTab.sbContinuity->setLocale(numberLocale);
	uiGeneralTab.sbBias->setLocale(numberLocale);
	uiGeneralTab.sbPoints->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	setSymbols(list);
	m_initializing = false;

	updatePlotRanges();

	// hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

void XYInterpolationCurveDock::updatePlotRanges() {
	updatePlotRangeList(uiGeneralTab.cbPlotRanges);
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYInterpolationCurveDock::dataSourceTypeChanged(int index) {
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
		static_cast<XYInterpolationCurve*>(curve)->setDataSourceType(type);
}

void XYInterpolationCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* dataSourceCurve = static_cast<XYCurve*>(index.internalPointer());

	// disable types that need more data points
	this->updateSettings(dataSourceCurve->xColumn());

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYInterpolationCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYInterpolationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	this->updateSettings(column);

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYInterpolationCurve*>(curve)->setXDataColumn(column);

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYInterpolationCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	// disable types that need more data points
	if (uiGeneralTab.cbAutoRange->isChecked()) {
		const auto numberLocale = QLocale();
		// TODO: this does not check if there are valid data points
		uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
		uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
	}

	unsigned int n = 0;
	for (int row = 0; row < column->rowCount(); row++)
		if (!std::isnan(column->valueAt(row)) && !column->isMasked(row))
			n++;
	dataPoints = n;
	if (m_interpolationData.pointsMode == XYInterpolationCurve::PointsMode::Auto)
		pointsModeChanged(uiGeneralTab.cbPointsMode->currentIndex());

	const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbType->model());
	auto* item = model->item(nsl_interp_type_polynomial);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_polynomial) || dataPoints > 100) { // not good for many points
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_polynomial)
			uiGeneralTab.cbType->setCurrentIndex(0);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	item = model->item(nsl_interp_type_cspline);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_cspline)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_cspline)
			uiGeneralTab.cbType->setCurrentIndex(0);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	item = model->item(nsl_interp_type_cspline_periodic);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_cspline_periodic)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_cspline_periodic)
			uiGeneralTab.cbType->setCurrentIndex(0);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	item = model->item(nsl_interp_type_akima);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_akima)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_akima)
			uiGeneralTab.cbType->setCurrentIndex(0);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	item = model->item(nsl_interp_type_akima_periodic);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_akima_periodic)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_akima_periodic)
			uiGeneralTab.cbType->setCurrentIndex(0);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#if GSL_MAJOR_VERSION >= 2
	item = model->item(nsl_interp_type_steffen);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_steffen)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_steffen)
			uiGeneralTab.cbType->setCurrentIndex(0);
	} else
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif
	// own types work with 2 or more data points
}

void XYInterpolationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());

	for (auto* curve : m_curvesList)
		static_cast<XYInterpolationCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYInterpolationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_interpolationData.autoRange = autoRange;

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
		if (m_interpolationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_interpolationCurve->xDataColumn();
		else {
			if (m_interpolationCurve->dataSourceCurve())
				xDataColumn = m_interpolationCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			if (!m_dateTimeRange) {
				const auto numberLocale = QLocale();
				// TODO: this does not check if there are valid data points
				uiGeneralTab.leMin->setText(numberLocale.toString(xDataColumn->minimum()));
				uiGeneralTab.leMax->setText(numberLocale.toString(xDataColumn->maximum()));
			} else {
				uiGeneralTab.dateTimeEditMin->setDateTime(QDateTime::fromMSecsSinceEpoch(xDataColumn->minimum()));
				uiGeneralTab.dateTimeEditMax->setDateTime(QDateTime::fromMSecsSinceEpoch(xDataColumn->maximum()));
			}
		}
	}
}

void XYInterpolationCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_interpolationData.xRange.first(), uiGeneralTab.leMin);
}

void XYInterpolationCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_interpolationData.xRange.last(), uiGeneralTab.leMax);
}

void XYInterpolationCurveDock::xRangeMinDateTimeChanged(const QDateTime& dateTime) {
	CONDITIONAL_LOCK_RETURN;

	m_interpolationData.xRange.first() = dateTime.toMSecsSinceEpoch();
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::xRangeMaxDateTimeChanged(const QDateTime& dateTime) {
	CONDITIONAL_LOCK_RETURN;

	m_interpolationData.xRange.last() = dateTime.toMSecsSinceEpoch();
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::typeChanged(int index) {
	const auto type = (nsl_interp_type)index;
	m_interpolationData.type = type;

	switch (type) {
	case nsl_interp_type_pch:
		uiGeneralTab.lVariant->show();
		uiGeneralTab.cbVariant->show();
		break;
	case nsl_interp_type_linear:
	case nsl_interp_type_polynomial:
	case nsl_interp_type_cspline:
	case nsl_interp_type_cspline_periodic:
	case nsl_interp_type_akima:
	case nsl_interp_type_akima_periodic:
	case nsl_interp_type_steffen:
	case nsl_interp_type_cosine:
	case nsl_interp_type_exponential:
	case nsl_interp_type_rational:
		uiGeneralTab.lVariant->hide();
		uiGeneralTab.cbVariant->hide();
		uiGeneralTab.cbVariant->setCurrentIndex(nsl_interp_pch_variant_finite_difference);
		uiGeneralTab.lParameter->hide();
		uiGeneralTab.lTension->hide();
		uiGeneralTab.sbTension->hide();
		uiGeneralTab.lContinuity->hide();
		uiGeneralTab.sbContinuity->hide();
		uiGeneralTab.lBias->hide();
		uiGeneralTab.sbBias->hide();
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::variantChanged(int index) {
	const auto variant = (nsl_interp_pch_variant)index;
	m_interpolationData.variant = variant;

	switch (variant) {
	case nsl_interp_pch_variant_finite_difference:
		uiGeneralTab.lParameter->hide();
		uiGeneralTab.lTension->hide();
		uiGeneralTab.sbTension->hide();
		uiGeneralTab.lContinuity->hide();
		uiGeneralTab.sbContinuity->hide();
		uiGeneralTab.lBias->hide();
		uiGeneralTab.sbBias->hide();
		break;
	case nsl_interp_pch_variant_catmull_rom:
		uiGeneralTab.lParameter->show();
		uiGeneralTab.lTension->show();
		uiGeneralTab.sbTension->show();
		uiGeneralTab.sbTension->setEnabled(false);
		uiGeneralTab.sbTension->setValue(0.0);
		uiGeneralTab.lContinuity->hide();
		uiGeneralTab.sbContinuity->hide();
		uiGeneralTab.lBias->hide();
		uiGeneralTab.sbBias->hide();
		break;
	case nsl_interp_pch_variant_cardinal:
		uiGeneralTab.lParameter->show();
		uiGeneralTab.lTension->show();
		uiGeneralTab.sbTension->show();
		uiGeneralTab.sbTension->setEnabled(true);
		uiGeneralTab.lContinuity->hide();
		uiGeneralTab.sbContinuity->hide();
		uiGeneralTab.lBias->hide();
		uiGeneralTab.sbBias->hide();
		break;
	case nsl_interp_pch_variant_kochanek_bartels:
		uiGeneralTab.lParameter->show();
		uiGeneralTab.lTension->show();
		uiGeneralTab.sbTension->show();
		uiGeneralTab.sbTension->setEnabled(true);
		uiGeneralTab.lContinuity->show();
		uiGeneralTab.sbContinuity->show();
		uiGeneralTab.lBias->show();
		uiGeneralTab.sbBias->show();
		break;
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::tensionChanged(double value) {
	m_interpolationData.tension = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::continuityChanged(double value) {
	m_interpolationData.continuity = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::biasChanged(double value) {
	m_interpolationData.bias = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::evaluateChanged(int index) {
	m_interpolationData.evaluate = (nsl_interp_evaluate)index;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::pointsModeChanged(int index) {
	const auto mode = (XYInterpolationCurve::PointsMode)index;

	switch (mode) {
	case XYInterpolationCurve::PointsMode::Auto:
		uiGeneralTab.sbPoints->setEnabled(false);
		uiGeneralTab.sbPoints->setDecimals(0);
		uiGeneralTab.sbPoints->setSingleStep(1.0);
		uiGeneralTab.sbPoints->setValue(5 * dataPoints);
		break;
	case XYInterpolationCurve::PointsMode::Multiple:
		uiGeneralTab.sbPoints->setEnabled(true);
		if (m_interpolationData.pointsMode != XYInterpolationCurve::PointsMode::Multiple && dataPoints > 0) {
			uiGeneralTab.sbPoints->setDecimals(2);
			uiGeneralTab.sbPoints->setValue(uiGeneralTab.sbPoints->value() / (double)dataPoints);
			uiGeneralTab.sbPoints->setSingleStep(0.01);
		}
		break;
	case XYInterpolationCurve::PointsMode::Custom:
		uiGeneralTab.sbPoints->setEnabled(true);
		if (m_interpolationData.pointsMode == XYInterpolationCurve::PointsMode::Multiple) {
			uiGeneralTab.sbPoints->setDecimals(0);
			uiGeneralTab.sbPoints->setSingleStep(1.0);
			uiGeneralTab.sbPoints->setValue(uiGeneralTab.sbPoints->value() * dataPoints);
		}
		break;
	}

	m_interpolationData.pointsMode = mode;
}

void XYInterpolationCurveDock::numberOfPointsChanged() {
	m_interpolationData.npoints = uiGeneralTab.sbPoints->value();
	if (uiGeneralTab.cbPointsMode->currentIndex() == static_cast<int>(XYInterpolationCurve::PointsMode::Multiple))
		m_interpolationData.npoints *= dataPoints;

	// warn if points is smaller than data points
	QPalette palette = uiGeneralTab.sbPoints->palette();
	if (m_interpolationData.npoints < dataPoints)
		palette.setColor(QPalette::Text, Qt::red);
	else
		palette.setColor(QPalette::Text, Qt::black);
	uiGeneralTab.sbPoints->setPalette(palette);

	enableRecalculate();
}

void XYInterpolationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		static_cast<XYInterpolationCurve*>(curve)->setInterpolationData(m_interpolationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Interpolation status: %1", m_interpolationCurve->result().status));
	QApplication::restoreOverrideCursor();
}

void XYInterpolationCurveDock::enableRecalculate() const {
	CONDITIONAL_RETURN_NO_LOCK;

	// no interpolation possible without the x- and y-data
	bool hasSourceData = false;
	if (m_interpolationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectX && aspectY);
		if (aspectX) {
			cbXDataColumn->useCurrentIndexText(true);
			cbXDataColumn->setInvalid(false);
		}
		if (aspectY) {
			cbYDataColumn->useCurrentIndexText(true);
			cbYDataColumn->setInvalid(false);
		}
	} else {
		hasSourceData = (m_interpolationCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the interpolation
 */
void XYInterpolationCurveDock::showInterpolationResult() {
	const auto& interpolationResult = m_interpolationCurve->result();
	if (!interpolationResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", interpolationResult.status) + QStringLiteral("<br>");

	if (!interpolationResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; // result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	const auto numberLocale = QLocale();
	if (interpolationResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", numberLocale.toString(interpolationResult.elapsedTime / 1000)) + QStringLiteral("<br>");
	else
		str += i18n("calculation time: %1 ms", numberLocale.toString(interpolationResult.elapsedTime)) + QStringLiteral("<br>");

	str += QStringLiteral("<br><br>");

	uiGeneralTab.teResult->setText(str);

	// enable the "recalculate"-button if the source data was changed since the last interpolation
	uiGeneralTab.pbRecalculate->setEnabled(m_interpolationCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYInterpolationCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	CONDITIONAL_LOCK_RETURN;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
}

void XYInterpolationCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	CONDITIONAL_LOCK_RETURN;
	cbDataSourceCurve->setAspect(curve);
}

void XYInterpolationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXDataColumn->setColumn(column, m_interpolationCurve->xDataColumnPath());
}

void XYInterpolationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYDataColumn->setColumn(column, m_interpolationCurve->yDataColumnPath());
}

void XYInterpolationCurveDock::curveInterpolationDataChanged(const XYInterpolationCurve::InterpolationData& data) {
	CONDITIONAL_LOCK_RETURN;
	m_interpolationData = data;
	uiGeneralTab.cbType->setCurrentIndex(m_interpolationData.type);
	this->typeChanged(m_interpolationData.type);

	this->showInterpolationResult();
}

void XYInterpolationCurveDock::dataChanged() {
	this->enableRecalculate();
}
