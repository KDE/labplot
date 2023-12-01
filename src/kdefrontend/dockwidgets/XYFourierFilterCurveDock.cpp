/*
	File             : XYFourierFilterCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of Fourier filter curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYFourierFilterCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <KMessageBox>

#include <QMenu>
#include <QWidgetAction>

/*!
  \class XYFourierFilterCurveDock
 \brief  Provides a widget for editing the properties of the XYFourierFilterCurves
		(2D-curves defined by a Fourier filter) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYFourierFilterCurveDock::XYFourierFilterCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYFourierFilterCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment);

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2, 2, 2, 2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 3);
	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 2);

	for (int i = 0; i < NSL_FILTER_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_filter_type_name[i]));

	for (int i = 0; i < NSL_FILTER_FORM_COUNT; i++)
		uiGeneralTab.cbForm->addItem(i18n(nsl_filter_form_name[i]));

	for (int i = 0; i < NSL_FILTER_CUTOFF_UNIT_COUNT; i++) {
		uiGeneralTab.cbUnit->addItem(i18n(nsl_filter_cutoff_unit_name[i]));
		uiGeneralTab.cbUnit2->addItem(i18n(nsl_filter_cutoff_unit_name[i]));
	}

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYFourierFilterCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYFourierFilterCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYFourierFilterCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYFourierFilterCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYFourierFilterCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYFourierFilterCurveDock::xRangeMaxDateTimeChanged);

	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::typeChanged);
	connect(uiGeneralTab.cbForm, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::formChanged);
	connect(uiGeneralTab.sbOrder, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYFourierFilterCurveDock::orderChanged);
	connect(uiGeneralTab.sbCutoff, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYFourierFilterCurveDock::enableRecalculate);
	connect(uiGeneralTab.sbCutoff2, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYFourierFilterCurveDock::enableRecalculate);
	connect(uiGeneralTab.cbUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::unitChanged);
	connect(uiGeneralTab.cbUnit2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::unit2Changed);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYFourierFilterCurveDock::recalculateClicked);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::plotRangeChanged);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYFourierFilterCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFourierFilterCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFourierFilterCurveDock::yDataColumnChanged);
}

void XYFourierFilterCurveDock::initGeneralTab() {
	// if there are more than one curve in the list, disable the tab "general"
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
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_filterCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_filterCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_filterCurve->xDataColumn(), m_filterCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_filterCurve->yDataColumn(), m_filterCurve->xDataColumnPath());
	uiGeneralTab.cbAutoRange->setChecked(m_filterData.autoRange);

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_filterCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(m_filterData.xRange.first()));
		uiGeneralTab.leMax->setText(numberLocale.toString(m_filterData.xRange.last()));
	} else {
		uiGeneralTab.dateTimeEditMin->setMSecsSinceEpochUTC(m_filterData.xRange.first());
		uiGeneralTab.dateTimeEditMax->setMSecsSinceEpochUTC(m_filterData.xRange.last());
	}

	uiGeneralTab.lMin->setVisible(!m_dateTimeRange);
	uiGeneralTab.leMin->setVisible(!m_dateTimeRange);
	uiGeneralTab.lMax->setVisible(!m_dateTimeRange);
	uiGeneralTab.leMax->setVisible(!m_dateTimeRange);
	uiGeneralTab.lMinDateTime->setVisible(m_dateTimeRange);
	uiGeneralTab.dateTimeEditMin->setVisible(m_dateTimeRange);
	uiGeneralTab.lMaxDateTime->setVisible(m_dateTimeRange);
	uiGeneralTab.dateTimeEditMax->setVisible(m_dateTimeRange);

	this->autoRangeChanged();

	uiGeneralTab.cbType->setCurrentIndex(m_filterData.type);
	this->typeChanged();
	uiGeneralTab.cbForm->setCurrentIndex(m_filterData.form);
	this->formChanged();
	uiGeneralTab.sbOrder->setValue((int)m_filterData.order);
	uiGeneralTab.cbUnit->setCurrentIndex(m_filterData.unit);
	this->unitChanged();
	// after unit has set
	uiGeneralTab.sbCutoff->setValue(m_filterData.cutoff);
	uiGeneralTab.cbUnit2->setCurrentIndex(m_filterData.unit2);
	this->unit2Changed();
	// after unit has set
	uiGeneralTab.sbCutoff2->setValue(m_filterData.cutoff2);
	this->showFilterResult();

	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_filterCurve, &XYFourierFilterCurve::dataSourceTypeChanged, this, &XYFourierFilterCurveDock::curveDataSourceTypeChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::dataSourceCurveChanged, this, &XYFourierFilterCurveDock::curveDataSourceCurveChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::xDataColumnChanged, this, &XYFourierFilterCurveDock::curveXDataColumnChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::yDataColumnChanged, this, &XYFourierFilterCurveDock::curveYDataColumnChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::filterDataChanged, this, &XYFourierFilterCurveDock::curveFilterDataChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::sourceDataChanged, this, &XYFourierFilterCurveDock::enableRecalculate);
	connect(m_filterCurve, &XYCurve::visibleChanged, this, &XYFourierFilterCurveDock::curveVisibilityChanged);
	connect(m_filterCurve, &WorksheetElement::plotRangeListChanged, this, &XYFourierFilterCurveDock::updatePlotRanges);
}

void XYFourierFilterCurveDock::setModel() {
	auto list = defaultColumnTopLevelClasses();
	list.append(AspectType::XYFitCurve);

	XYAnalysisCurveDock::setModel(list);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYFourierFilterCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	m_filterCurve = static_cast<XYFourierFilterCurve*>(m_curve);
	this->setModel();
	m_filterData = m_filterCurve->filterData();

	initGeneralTab();
	initTabs();
	setSymbols(list);
	m_initializing = false;

	updatePlotRanges();
}

void XYFourierFilterCurveDock::updatePlotRanges() {
	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYFourierFilterCurveDock::dataSourceTypeChanged(int index) {
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
		static_cast<XYFourierFilterCurve*>(curve)->setDataSourceType(type);
}

void XYFourierFilterCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* dataSourceCurve = static_cast<XYCurve*>(index.internalPointer());

	// update range of cutoff spin boxes (like a unit change)
	unitChanged();
	unit2Changed();

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYFourierFilterCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYFourierFilterCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());

	for (auto* curve : m_curvesList)
		static_cast<XYFourierFilterCurve*>(curve)->setXDataColumn(column);

	// update range of cutoff spin boxes (like a unit change)
	unitChanged();
	unit2Changed();

	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
		uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYFourierFilterCurveDock::yDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());

	for (auto* curve : m_curvesList)
		static_cast<XYFourierFilterCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYFourierFilterCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_filterData.autoRange = autoRange;

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
		if (m_filterCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_filterCurve->xDataColumn();
		else {
			if (m_filterCurve->dataSourceCurve())
				xDataColumn = m_filterCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			const auto numberLocale = QLocale();
			uiGeneralTab.leMin->setText(numberLocale.toString(xDataColumn->minimum()));
			uiGeneralTab.leMax->setText(numberLocale.toString(xDataColumn->maximum()));
		}
	}
}
void XYFourierFilterCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_filterData.xRange.first(), uiGeneralTab.leMin);
}

void XYFourierFilterCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_filterData.xRange.last(), uiGeneralTab.leMax);
}

void XYFourierFilterCurveDock::xRangeMinDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_filterData.xRange.first() = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYFourierFilterCurveDock::xRangeMaxDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_filterData.xRange.last() = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYFourierFilterCurveDock::typeChanged() {
	auto type = (nsl_filter_type)uiGeneralTab.cbType->currentIndex();
	m_filterData.type = type;

	switch (type) {
	case nsl_filter_type_low_pass:
	case nsl_filter_type_high_pass:
		uiGeneralTab.lCutoff->setText(i18n("Cutoff:"));
		uiGeneralTab.lCutoff2->setVisible(false);
		uiGeneralTab.sbCutoff2->setVisible(false);
		uiGeneralTab.cbUnit2->setVisible(false);
		break;
	case nsl_filter_type_band_pass:
	case nsl_filter_type_band_reject:
		uiGeneralTab.lCutoff2->setVisible(true);
		uiGeneralTab.lCutoff->setText(i18n("Lower cutoff:"));
		uiGeneralTab.lCutoff2->setText(i18n("Upper cutoff:"));
		uiGeneralTab.sbCutoff2->setVisible(true);
		uiGeneralTab.cbUnit2->setVisible(true);
		break;
		// TODO
		/*	case nsl_filter_type_threshold:
				uiGeneralTab.lCutoff->setText(i18n("Value:"));
				uiGeneralTab.lCutoff2->setVisible(false);
				uiGeneralTab.sbCutoff2->setVisible(false);
				uiGeneralTab.cbUnit2->setVisible(false);
		*/
	}

	enableRecalculate();
}

void XYFourierFilterCurveDock::formChanged() {
	auto form = (nsl_filter_form)uiGeneralTab.cbForm->currentIndex();
	m_filterData.form = form;

	switch (form) {
	case nsl_filter_form_ideal:
		uiGeneralTab.sbOrder->setVisible(false);
		uiGeneralTab.lOrder->setVisible(false);
		break;
	case nsl_filter_form_butterworth:
	case nsl_filter_form_chebyshev_i:
	case nsl_filter_form_chebyshev_ii:
	case nsl_filter_form_legendre:
	case nsl_filter_form_bessel:
		uiGeneralTab.sbOrder->setVisible(true);
		uiGeneralTab.lOrder->setVisible(true);
		break;
	}

	enableRecalculate();
}

void XYFourierFilterCurveDock::orderChanged() {
	m_filterData.order = (unsigned int)uiGeneralTab.sbOrder->value();

	enableRecalculate();
}

void XYFourierFilterCurveDock::updateCutoffSpinBoxes(NumberSpinBox* sb, nsl_filter_cutoff_unit newUnit, nsl_filter_cutoff_unit oldUnit, double oldValue) {
	int n = 100;
	double f = 1.0; // sample frequency
	const AbstractColumn* xDataColumn = nullptr;
	if (m_filterCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
		xDataColumn = m_filterCurve->xDataColumn();
	else {
		if (m_filterCurve->dataSourceCurve())
			xDataColumn = m_filterCurve->dataSourceCurve()->xColumn();
	}

	if (xDataColumn) {
		n = xDataColumn->rowCount();
		double range = xDataColumn->maximum() - xDataColumn->minimum();
		if (xDataColumn->columnMode() == AbstractColumn::ColumnMode::DateTime) {
			// data is in ms therefore they have to be converted to s
			range /= 1000;
		}
		f = (n - 1) / range / 2.;
		DEBUG(" n =" << n << " sample frequency =" << f);
	}

	switch (newUnit) {
	case nsl_filter_cutoff_unit_frequency:
		sb->setMaximum(f);
		sb->setSuffix(QStringLiteral(" Hz"));
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			break;
		case nsl_filter_cutoff_unit_fraction:
			sb->setValue(oldValue * f);
			break;
		case nsl_filter_cutoff_unit_index:
			sb->setValue(oldValue * f / n);
			break;
		}
		break;
	case nsl_filter_cutoff_unit_fraction:
		sb->setMaximum(1.0);
		sb->setSuffix(QString());
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			sb->setValue(oldValue / f);
			break;
		case nsl_filter_cutoff_unit_fraction:
			break;
		case nsl_filter_cutoff_unit_index:
			sb->setValue(oldValue / n);
			break;
		}
		break;
	case nsl_filter_cutoff_unit_index:
		sb->setMaximum(n);
		sb->setSuffix(QString());
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			sb->setValue(oldValue * n / f);
			break;
		case nsl_filter_cutoff_unit_fraction:
			sb->setValue(oldValue * n);
			break;
		case nsl_filter_cutoff_unit_index:
			break;
		}
		break;
	}
}

void XYFourierFilterCurveDock::unitChanged() {
	auto unit = (nsl_filter_cutoff_unit)uiGeneralTab.cbUnit->currentIndex();
	m_filterData.unit = unit;

	updateCutoffSpinBoxes(uiGeneralTab.sbCutoff, unit, m_filterData.unit, uiGeneralTab.sbCutoff->value());
	enableRecalculate();
}

void XYFourierFilterCurveDock::unit2Changed() {
	auto unit = (nsl_filter_cutoff_unit)uiGeneralTab.cbUnit2->currentIndex();
	m_filterData.unit2 = unit;

	updateCutoffSpinBoxes(uiGeneralTab.sbCutoff2, unit, m_filterData.unit2, uiGeneralTab.sbCutoff2->value());
	enableRecalculate();
}

void XYFourierFilterCurveDock::recalculateClicked() {
	m_filterData.cutoff = uiGeneralTab.sbCutoff->value();
	m_filterData.cutoff2 = uiGeneralTab.sbCutoff2->value();

	if ((m_filterData.type == nsl_filter_type_band_pass || m_filterData.type == nsl_filter_type_band_reject) && m_filterData.cutoff2 <= m_filterData.cutoff) {
		KMessageBox::error(this,
						   i18n("The band width is <= 0 since lower cutoff value is not smaller than upper cutoff value. Please fix this."),
						   i18n("band width <= 0"));
		return;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (auto* curve : m_curvesList)
		static_cast<XYFourierFilterCurve*>(curve)->setFilterData(m_filterData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Fourier-Filter status: %1", m_filterCurve->filterResult().status));
	QApplication::restoreOverrideCursor();
}

void XYFourierFilterCurveDock::enableRecalculate() const {
	CONDITIONAL_RETURN_NO_LOCK;

	// no filtering possible without the x- and y-data
	bool hasSourceData = false;
	if (m_filterCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
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
		hasSourceData = (m_filterCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the filter
 */
void XYFourierFilterCurveDock::showFilterResult() {
	showResult(m_filterCurve, uiGeneralTab.teResult, uiGeneralTab.pbRecalculate);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYFourierFilterCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	CONDITIONAL_LOCK_RETURN;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
}

void XYFourierFilterCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	CONDITIONAL_LOCK_RETURN;
	cbDataSourceCurve->setAspect(curve);
}

void XYFourierFilterCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXDataColumn->setColumn(column, m_filterCurve->xDataColumnPath());
}

void XYFourierFilterCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYDataColumn->setColumn(column, m_filterCurve->yDataColumnPath());
}

void XYFourierFilterCurveDock::curveFilterDataChanged(const XYFourierFilterCurve::FilterData& filterData) {
	CONDITIONAL_LOCK_RETURN;
	m_filterData = filterData;
	uiGeneralTab.cbType->setCurrentIndex(m_filterData.type);
	this->typeChanged();

	this->showFilterResult();
}

void XYFourierFilterCurveDock::dataChanged() {
	this->enableRecalculate();
}

void XYFourierFilterCurveDock::curveVisibilityChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	uiGeneralTab.chkVisible->setChecked(on);
}
