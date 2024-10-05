/*
	File             : XYConvolutionCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of convolution curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYConvolutionCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>

extern "C" {
#include "backend/nsl/nsl_conv.h"
}

/*!
  \class XYConvolutionCurveDock
 \brief  Provides a widget for editing the properties of the XYConvolutionCurves
		(2D-curves defined by a convolution) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYConvolutionCurveDock::XYConvolutionCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent, XYAnalysisCurveDock::RequiredDataSource::YY2) {
}

/*!
 * 	// Tab "General"
 */
void XYConvolutionCurveDock::setupGeneral() {
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

	for (int i = 0; i < NSL_CONV_KERNEL_COUNT; i++)
		uiGeneralTab.cbKernel->addItem(i18n(nsl_conv_kernel_name[i]));

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	for (int i = 0; i < NSL_CONV_DIRECTION_COUNT; i++)
		uiGeneralTab.cbDirection->addItem(i18n(nsl_conv_direction_name[i]));
	for (int i = 0; i < NSL_CONV_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_conv_type_name[i]));
	// nsl_conv_method_type not exposed to user
	for (int i = 0; i < NSL_CONV_NORM_COUNT; i++)
		uiGeneralTab.cbNorm->addItem(i18n(nsl_conv_norm_name[i]));
	for (int i = 0; i < NSL_CONV_WRAP_COUNT; i++)
		uiGeneralTab.cbWrap->addItem(i18n(nsl_conv_wrap_name[i]));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYConvolutionCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.sbSamplingInterval, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYConvolutionCurveDock::samplingIntervalChanged);
	connect(uiGeneralTab.cbKernel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYConvolutionCurveDock::kernelChanged);
	connect(uiGeneralTab.sbKernelSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYConvolutionCurveDock::kernelSizeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYConvolutionCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYConvolutionCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYConvolutionCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.cbDirection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYConvolutionCurveDock::directionChanged);
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYConvolutionCurveDock::typeChanged);
	connect(uiGeneralTab.cbNorm, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYConvolutionCurveDock::normChanged);
	connect(uiGeneralTab.cbWrap, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYConvolutionCurveDock::wrapChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYConvolutionCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYConvolutionCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYConvolutionCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYConvolutionCurveDock::yDataColumnChanged);
	connect(cbY2DataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYConvolutionCurveDock::y2DataColumnChanged);
}

void XYConvolutionCurveDock::initGeneralTab() {
	// show the properties of the first curve
	//  hide x-Range per default
	uiGeneralTab.lXRange->setEnabled(false);
	uiGeneralTab.cbAutoRange->setEnabled(false);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_convolutionCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_convolutionCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_convolutionCurve->xDataColumn(), m_convolutionCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_convolutionCurve->yDataColumn(), m_convolutionCurve->yDataColumnPath());
	cbY2DataColumn->setColumn(m_convolutionCurve->y2DataColumn(), m_convolutionCurve->y2DataColumnPath());
	uiGeneralTab.sbSamplingInterval->setValue(m_convolutionData.samplingInterval);
	uiGeneralTab.cbKernel->setCurrentIndex(m_convolutionData.kernel);
	uiGeneralTab.sbKernelSize->setValue((int)m_convolutionData.kernelSize);
	uiGeneralTab.cbAutoRange->setChecked(m_convolutionData.autoRange);

	const auto numberLocale = QLocale();
	uiGeneralTab.leMin->setText(numberLocale.toString(m_convolutionData.xRange.first()));
	uiGeneralTab.leMax->setText(numberLocale.toString(m_convolutionData.xRange.last()));
	this->autoRangeChanged();
	y2DataColumnChanged(cbY2DataColumn->currentModelIndex());

	// settings
	uiGeneralTab.cbDirection->setCurrentIndex(m_convolutionData.direction);
	uiGeneralTab.cbType->setCurrentIndex(m_convolutionData.type);
	// m_convolutionData.method not used
	uiGeneralTab.cbNorm->setCurrentIndex(m_convolutionData.normalize);
	uiGeneralTab.cbWrap->setCurrentIndex(m_convolutionData.wrap);

	this->directionChanged();

	this->showConvolutionResult();

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_convolutionCurve, &XYConvolutionCurve::dataSourceTypeChanged, this, &XYConvolutionCurveDock::curveDataSourceTypeChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::dataSourceCurveChanged, this, &XYConvolutionCurveDock::curveDataSourceCurveChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::xDataColumnChanged, this, &XYConvolutionCurveDock::curveXDataColumnChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::yDataColumnChanged, this, &XYConvolutionCurveDock::curveYDataColumnChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::y2DataColumnChanged, this, &XYConvolutionCurveDock::curveY2DataColumnChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::convolutionDataChanged, this, &XYConvolutionCurveDock::curveConvolutionDataChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::sourceDataChanged, this, &XYConvolutionCurveDock::enableRecalculate);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYConvolutionCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	setAnalysisCurves(list);
	m_convolutionCurve = static_cast<XYConvolutionCurve*>(m_curve);
	m_convolutionData = m_convolutionCurve->convolutionData();

	const auto numberLocale = QLocale();
	uiGeneralTab.sbSamplingInterval->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	setSymbols(list);
	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYConvolutionCurveDock **
//*************************************************************
void XYConvolutionCurveDock::dataSourceTypeChanged(int index) {
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
		uiGeneralTab.lKernel->setText(i18n("or Kernel/Size:"));
	} else { // xy-curve data source
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
		uiGeneralTab.lKernel->setEnabled(true);
		uiGeneralTab.lKernel->setText(i18n("with Kernel/Size:"));
		uiGeneralTab.cbKernel->setEnabled(true);
		uiGeneralTab.sbKernelSize->setEnabled(true);
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYConvolutionCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

void XYConvolutionCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());
	for (auto* curve : m_curvesList)
		static_cast<XYConvolutionCurve*>(curve)->setXDataColumn(column);

	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
		uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
	}

	enableRecalculate();
}

void XYConvolutionCurveDock::samplingIntervalChanged() {
	double samplingInterval = uiGeneralTab.sbSamplingInterval->value();
	m_convolutionData.samplingInterval = samplingInterval;

	enableRecalculate();
}

void XYConvolutionCurveDock::kernelChanged() {
	auto kernel = (nsl_conv_kernel_type)uiGeneralTab.cbKernel->currentIndex();
	m_convolutionData.kernel = kernel;

	// TODO: change selectable sizes
	uiGeneralTab.sbKernelSize->setEnabled(true);
	switch (kernel) {
	case nsl_conv_kernel_avg: // all values allowed
	case nsl_conv_kernel_smooth_triangle:
	case nsl_conv_kernel_gaussian:
	case nsl_conv_kernel_lorentzian:
		uiGeneralTab.sbKernelSize->setMinimum(2);
		uiGeneralTab.sbKernelSize->setMaximum(999);
		uiGeneralTab.sbKernelSize->setSingleStep(1);
		uiGeneralTab.sbKernelSize->setValue(2);
		break;
	case nsl_conv_kernel_smooth_gaussian:
		uiGeneralTab.sbKernelSize->setMinimum(5);
		uiGeneralTab.sbKernelSize->setMaximum(9);
		uiGeneralTab.sbKernelSize->setSingleStep(2);
		uiGeneralTab.sbKernelSize->setValue(5);
		break;
	case nsl_conv_kernel_first_derivative:
		uiGeneralTab.sbKernelSize->setMinimum(2);
		uiGeneralTab.sbKernelSize->setValue(2);
		uiGeneralTab.sbKernelSize->setEnabled(false);
		break;
	case nsl_conv_kernel_smooth_first_derivative:
		uiGeneralTab.sbKernelSize->setMinimum(3);
		uiGeneralTab.sbKernelSize->setMaximum(999);
		uiGeneralTab.sbKernelSize->setSingleStep(2);
		uiGeneralTab.sbKernelSize->setValue(3);
		break;
	case nsl_conv_kernel_second_derivative:
		uiGeneralTab.sbKernelSize->setMinimum(3);
		uiGeneralTab.sbKernelSize->setValue(3);
		uiGeneralTab.sbKernelSize->setEnabled(false);
		break;
	case nsl_conv_kernel_third_derivative:
		uiGeneralTab.sbKernelSize->setMinimum(4);
		uiGeneralTab.sbKernelSize->setValue(4);
		uiGeneralTab.sbKernelSize->setEnabled(false);
		break;
	case nsl_conv_kernel_fourth_derivative:
		uiGeneralTab.sbKernelSize->setMinimum(5);
		uiGeneralTab.sbKernelSize->setValue(5);
		uiGeneralTab.sbKernelSize->setEnabled(false);
		break;
	}

	enableRecalculate();
}

void XYConvolutionCurveDock::kernelSizeChanged() {
	size_t kernelSize = uiGeneralTab.sbKernelSize->value();
	m_convolutionData.kernelSize = kernelSize;

	enableRecalculate();
}

void XYConvolutionCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_convolutionData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.leMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.leMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_convolutionCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_convolutionCurve->xDataColumn();
		else {
			if (m_convolutionCurve->dataSourceCurve())
				xDataColumn = m_convolutionCurve->dataSourceCurve()->xColumn();
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
void XYConvolutionCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_convolutionData.xRange.first(), uiGeneralTab.leMin);
}

void XYConvolutionCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_convolutionData.xRange.last(), uiGeneralTab.leMax);
}

void XYConvolutionCurveDock::directionChanged() {
	auto dir = (nsl_conv_direction_type)uiGeneralTab.cbDirection->currentIndex();
	m_convolutionData.direction = dir;

	// change name if still default
	if (m_curve->name().compare(i18n("Convolution")) == 0 && dir == nsl_conv_direction_backward) {
		m_curve->setName(i18n("Deconvolution"));
		uiGeneralTab.leName->setText(m_curve->name());
	}
	if (m_curve->name().compare(i18n("Deconvolution")) == 0 && dir == nsl_conv_direction_forward) {
		m_curve->setName(i18n("Convolution"));
		uiGeneralTab.leName->setText(m_curve->name());
	}

	enableRecalculate();
}

void XYConvolutionCurveDock::typeChanged() {
	auto type = (nsl_conv_type_type)uiGeneralTab.cbType->currentIndex();
	m_convolutionData.type = type;

	enableRecalculate();
}

void XYConvolutionCurveDock::normChanged() {
	auto norm = (nsl_conv_norm_type)uiGeneralTab.cbNorm->currentIndex();
	m_convolutionData.normalize = norm;

	enableRecalculate();
}

void XYConvolutionCurveDock::wrapChanged() {
	auto wrap = (nsl_conv_wrap_type)uiGeneralTab.cbWrap->currentIndex();
	m_convolutionData.wrap = wrap;

	enableRecalculate();
}

void XYConvolutionCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		static_cast<XYConvolutionCurve*>(curve)->setConvolutionData(m_convolutionData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	if (m_convolutionData.direction == nsl_conv_direction_forward)
		Q_EMIT info(i18n("Convolution status: %1", m_convolutionCurve->convolutionResult().status));
	else
		Q_EMIT info(i18n("Deconvolution status: %1", m_convolutionCurve->convolutionResult().status));
	QApplication::restoreOverrideCursor();
}

/*!
 * show the result and details of the convolution
 */
void XYConvolutionCurveDock::showConvolutionResult() {
	showResult(m_convolutionCurve, uiGeneralTab.teResult);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYConvolutionCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
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
	cbXDataColumn->setColumn(column, m_convolutionCurve->xDataColumnPath());
	enableRecalculate();
}

void XYConvolutionCurveDock::curveY2DataColumnChanged(const AbstractColumn* column) {
	if (column) {
		DEBUG("Y2 Column available");
		uiGeneralTab.lKernel->setEnabled(false);
		uiGeneralTab.cbKernel->setEnabled(false);
		uiGeneralTab.sbKernelSize->setEnabled(false);
	} else {
		DEBUG("Y2 Column not available");
		uiGeneralTab.lKernel->setEnabled(true);
		uiGeneralTab.cbKernel->setEnabled(true);
		uiGeneralTab.sbKernelSize->setEnabled(true);
	}

	CONDITIONAL_LOCK_RETURN;
	cbY2DataColumn->setColumn(column, m_convolutionCurve->y2DataColumnPath());
	enableRecalculate();
}

void XYConvolutionCurveDock::curveConvolutionDataChanged(const XYConvolutionCurve::ConvolutionData& convolutionData) {
	CONDITIONAL_LOCK_RETURN;
	m_convolutionData = convolutionData;
	this->directionChanged();

	this->showConvolutionResult();
}
