/*
	File             : XYIntegrationCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of integration curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYIntegrationCurveDock.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>

extern "C" {
#include "backend/nsl/nsl_int.h"
}

/*!
  \class XYIntegrationCurveDock
 \brief  Provides a widget for editing the properties of the XYIntegrationCurves
		(2D-curves defined by a integration) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYIntegrationCurveDock::XYIntegrationCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYIntegrationCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment, 1.2);

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2, 2, 2, 2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 3);
	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 3);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 3);

	for (int i = 0; i < NSL_INT_NETHOD_COUNT; i++)
		uiGeneralTab.cbMethod->addItem(i18n(nsl_int_method_name[i]));

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYIntegrationCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYIntegrationCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYIntegrationCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYIntegrationCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYIntegrationCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYIntegrationCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &XYIntegrationCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYIntegrationCurveDock::methodChanged);
	connect(uiGeneralTab.cbAbsolute, &QCheckBox::clicked, this, &XYIntegrationCurveDock::absoluteChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYIntegrationCurveDock::recalculateClicked);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYIntegrationCurveDock::plotRangeChanged);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYIntegrationCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYIntegrationCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYIntegrationCurveDock::yDataColumnChanged);
}

void XYIntegrationCurveDock::initGeneralTab() {
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
	// data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_integrationCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_integrationCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_integrationCurve->xDataColumn(), m_integrationCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_integrationCurve->yDataColumn(), m_integrationCurve->yDataColumnPath());

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_integrationCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(m_integrationData.xRange.first()));
		uiGeneralTab.leMax->setText(numberLocale.toString(m_integrationData.xRange.last()));
	} else {
		uiGeneralTab.dateTimeEditMin->setMSecsSinceEpochUTC(m_integrationData.xRange.first());
		uiGeneralTab.dateTimeEditMax->setMSecsSinceEpochUTC(m_integrationData.xRange.last());
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
	uiGeneralTab.cbAutoRange->setChecked(m_integrationData.autoRange);
	this->autoRangeChanged();

	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbMethod->setCurrentIndex(m_integrationData.method);
	this->methodChanged(m_integrationData.method);
	uiGeneralTab.cbAbsolute->setChecked(m_integrationData.absolute);
	this->absoluteChanged();

	this->showIntegrationResult();

	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_integrationCurve, &XYIntegrationCurve::dataSourceTypeChanged, this, &XYIntegrationCurveDock::curveDataSourceTypeChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::dataSourceCurveChanged, this, &XYIntegrationCurveDock::curveDataSourceCurveChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::xDataColumnChanged, this, &XYIntegrationCurveDock::curveXDataColumnChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::yDataColumnChanged, this, &XYIntegrationCurveDock::curveYDataColumnChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::integrationDataChanged, this, &XYIntegrationCurveDock::curveIntegrationDataChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::sourceDataChanged, this, &XYIntegrationCurveDock::enableRecalculate);
	connect(m_integrationCurve, &WorksheetElement::plotRangeListChanged, this, &XYIntegrationCurveDock::updatePlotRanges);
	connect(m_integrationCurve, &WorksheetElement::visibleChanged, this, &XYIntegrationCurveDock::curveVisibilityChanged);
}

void XYIntegrationCurveDock::setModel() {
	auto list = defaultColumnTopLevelClasses();
	list.append(AspectType::XYFitCurve);

	XYAnalysisCurveDock::setModel(list);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYIntegrationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	m_integrationCurve = static_cast<XYIntegrationCurve*>(m_curve);
	this->setModel();
	m_integrationData = m_integrationCurve->integrationData();

	initGeneralTab();
	initTabs();
	setSymbols(list);
	m_initializing = false;

	updatePlotRanges();

	// hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

void XYIntegrationCurveDock::updatePlotRanges() {
	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************

void XYIntegrationCurveDock::dataSourceTypeChanged(int index) {
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
		static_cast<XYIntegrationCurve*>(curve)->setDataSourceType(type);
}

void XYIntegrationCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* dataSourceCurve = static_cast<XYCurve*>(index.internalPointer());

	// disable integration orders and accuracies that need more data points
	this->updateSettings(dataSourceCurve->xColumn());

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYIntegrationCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYIntegrationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());

	for (auto* curve : m_curvesList)
		static_cast<XYIntegrationCurve*>(curve)->setXDataColumn(column);

	if (column) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			const auto numberLocale = QLocale();
			uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
			uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
		}

		// disable integration methods that need more data points
		this->updateSettings(column);
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

/*!
 * disable deriv orders and accuracies that need more data points
 */
void XYIntegrationCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	// TODO
	// 	size_t n = 0;
	// 	for (int row = 0; row < column->rowCount(); row++)
	// 		if (!std::isnan(column->valueAt(row)) && !column->isMasked(row))
	// 			n++;
}
void XYIntegrationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	cbYDataColumn->hidePopup();

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());

	for (auto* curve : m_curvesList)
		static_cast<XYIntegrationCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYIntegrationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_integrationData.autoRange = autoRange;

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
		if (m_integrationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_integrationCurve->xDataColumn();
		else {
			if (m_integrationCurve->dataSourceCurve())
				xDataColumn = m_integrationCurve->dataSourceCurve()->xColumn();
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

void XYIntegrationCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_integrationData.xRange.first(), uiGeneralTab.leMin);
}

void XYIntegrationCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_integrationData.xRange.last(), uiGeneralTab.leMax);
}

void XYIntegrationCurveDock::xRangeMinDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_integrationData.xRange.first() = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::xRangeMaxDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	m_integrationData.xRange.last() = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::methodChanged(int index) {
	const auto method = (nsl_int_method_type)index;
	m_integrationData.method = method;

	// update absolute option
	switch (method) {
	case nsl_int_method_rectangle:
	case nsl_int_method_trapezoid:
		uiGeneralTab.cbAbsolute->setEnabled(true);
		break;
	case nsl_int_method_simpson:
	case nsl_int_method_simpson_3_8:
		uiGeneralTab.cbAbsolute->setChecked(false);
		uiGeneralTab.cbAbsolute->setEnabled(false);
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::absoluteChanged() {
	bool absolute = uiGeneralTab.cbAbsolute->isChecked();
	m_integrationData.absolute = absolute;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		static_cast<XYIntegrationCurve*>(curve)->setIntegrationData(m_integrationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Integration status: %1", m_integrationCurve->integrationResult().status));
	QApplication::restoreOverrideCursor();
}

void XYIntegrationCurveDock::enableRecalculate() const {
	CONDITIONAL_RETURN_NO_LOCK;

	// no integration possible without the x- and y-data
	bool hasSourceData = false;
	if (m_integrationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
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
		hasSourceData = (m_integrationCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the integration
 */
void XYIntegrationCurveDock::showIntegrationResult() {
	showResult(m_integrationCurve, uiGeneralTab.teResult, uiGeneralTab.pbRecalculate);
}

QString XYIntegrationCurveDock::customText() const {
	const auto numberLocale = QLocale();
	return i18n("value: %1", numberLocale.toString(m_integrationCurve->integrationResult().value)) + QStringLiteral("<br>");
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYIntegrationCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	CONDITIONAL_LOCK_RETURN;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
}

void XYIntegrationCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	CONDITIONAL_LOCK_RETURN;
	cbDataSourceCurve->setAspect(curve);
}

void XYIntegrationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXDataColumn->setColumn(column, m_integrationCurve->xDataColumnPath());
}

void XYIntegrationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYDataColumn->setColumn(column, m_integrationCurve->yDataColumnPath());
}

void XYIntegrationCurveDock::curveIntegrationDataChanged(const XYIntegrationCurve::IntegrationData& integrationData) {
	CONDITIONAL_LOCK_RETURN;
	m_integrationData = integrationData;
	uiGeneralTab.cbMethod->setCurrentIndex(m_integrationData.method);
	this->methodChanged(m_integrationData.method);
	uiGeneralTab.cbAbsolute->setChecked(m_integrationData.absolute);
	this->absoluteChanged();

	this->showIntegrationResult();
}

void XYIntegrationCurveDock::dataChanged() {
	this->enableRecalculate();
}

void XYIntegrationCurveDock::curveVisibilityChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	uiGeneralTab.chkVisible->setChecked(on);
}
