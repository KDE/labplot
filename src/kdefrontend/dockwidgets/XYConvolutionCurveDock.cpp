/***************************************************************************
    File             : XYConvolutionCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of convolution curves

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "XYConvolutionCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

extern "C" {
#include "backend/nsl/nsl_conv.h"
}

/*!
  \class XYConvolutionCurveDock
 \brief  Provides a widget for editing the properties of the XYConvolutionCurves
		(2D-curves defined by a convolution) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYConvolutionCurveDock::XYConvolutionCurveDock(QWidget* parent) : XYCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYConvolutionCurveDock::setupGeneral() {
	DEBUG("XYConvolutionCurveDock::setupGeneral()");
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_leComment = uiGeneralTab.leComment;

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2,2,2,2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

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

	uiGeneralTab.sbMin->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	uiGeneralTab.sbMax->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	for (int i = 0; i < NSL_CONV_DIRECTION_COUNT; i++)
		uiGeneralTab.cbDirection->addItem(i18n(nsl_conv_direction_name[i]));
	for (int i = 0; i < NSL_CONV_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_conv_type_name[i]));
	// nsl_conv_method_type not exposed to user
	for (int i = 0; i < NSL_CONV_NORM_COUNT; i++)
		uiGeneralTab.cbNorm->addItem(i18n(nsl_conv_norm_name[i]));
	for (int i = 0; i < NSL_CONV_WRAP_COUNT; i++)
		uiGeneralTab.cbWrap->addItem(i18n(nsl_conv_wrap_name[i]));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	DEBUG("XYConvolutionCurveDock::setupGeneral() DONE");

	//Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYConvolutionCurveDock::nameChanged );
	connect(uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYConvolutionCurveDock::commentChanged );
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYConvolutionCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYConvolutionCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.sbSamplingInterval, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYConvolutionCurveDock::samplingIntervalChanged);
	connect(uiGeneralTab.cbKernel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYConvolutionCurveDock::kernelChanged);
	connect(uiGeneralTab.sbKernelSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYConvolutionCurveDock::kernelSizeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYConvolutionCurveDock::autoRangeChanged);
	connect(uiGeneralTab.sbMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYConvolutionCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.sbMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYConvolutionCurveDock::xRangeMaxChanged);
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
	DEBUG("XYConvolutionCurveDock::initGeneralTab()");
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1) {
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.leComment->setEnabled(true);

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.leComment->setText(m_curve->comment());
	} else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.leComment->setEnabled(false);

		uiGeneralTab.leName->setText(QString());
		uiGeneralTab.leComment->setText(QString());
	}

	//show the properties of the first curve
	// hide x-Range per default
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
	uiGeneralTab.sbMin->setValue(m_convolutionData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_convolutionData.xRange.last());
	this->autoRangeChanged();
	y2DataColumnChanged(cbY2DataColumn->currentModelIndex());

	// settings
	uiGeneralTab.cbDirection->setCurrentIndex(m_convolutionData.direction);
	uiGeneralTab.cbType->setCurrentIndex(m_convolutionData.type);
	//m_convolutionData.method not used
	uiGeneralTab.cbNorm->setCurrentIndex(m_convolutionData.normalize);
	uiGeneralTab.cbWrap->setCurrentIndex(m_convolutionData.wrap);

	this->directionChanged();

	this->showConvolutionResult();

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_convolutionCurve, &XYConvolutionCurve::aspectDescriptionChanged, this, &XYConvolutionCurveDock::curveDescriptionChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::dataSourceTypeChanged, this, &XYConvolutionCurveDock::curveDataSourceTypeChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::dataSourceCurveChanged, this, &XYConvolutionCurveDock::curveDataSourceCurveChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::xDataColumnChanged, this, &XYConvolutionCurveDock::curveXDataColumnChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::yDataColumnChanged, this, &XYConvolutionCurveDock::curveYDataColumnChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::y2DataColumnChanged, this, &XYConvolutionCurveDock::curveY2DataColumnChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::convolutionDataChanged, this, &XYConvolutionCurveDock::curveConvolutionDataChanged);
	connect(m_convolutionCurve, &XYConvolutionCurve::sourceDataChanged, this, &XYConvolutionCurveDock::enableRecalculate);
	connect(m_convolutionCurve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &XYConvolutionCurveDock::curveVisibilityChanged);
}

void XYConvolutionCurveDock::setModel() {
	DEBUG("XYConvolutionCurveDock::setModel()");
	QList<AspectType> list{AspectType::Folder, AspectType::Datapicker, AspectType::Worksheet,
	                       AspectType::CartesianPlot, AspectType::XYCurve, AspectType::XYAnalysisCurve};
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list = {AspectType::Folder, AspectType::Workbook, AspectType::Datapicker, AspectType::DatapickerCurve,
			AspectType::Spreadsheet, AspectType::LiveDataSource, AspectType::Column,
			AspectType::Worksheet, AspectType::CartesianPlot, AspectType::XYConvolutionCurve
	};
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);
	cbY2DataColumn->setTopLevelClasses(list);

	cbDataSourceCurve->setModel(m_aspectTreeModel);
	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);
	cbY2DataColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
	DEBUG("XYConvolutionCurveDock::setModel() DONE");
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYConvolutionCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_convolutionCurve = static_cast<XYConvolutionCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_convolutionData = m_convolutionCurve->convolutionData();

	SET_NUMBER_LOCALE
	uiGeneralTab.sbSamplingInterval->setLocale(numberLocale);
	uiGeneralTab.sbMin->setLocale(numberLocale);
	uiGeneralTab.sbMax->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	m_initializing = false;

	//hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
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
	} else {	//xy-curve data source
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

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

void XYConvolutionCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYConvolutionCurveDock::xDataColumnChanged(const QModelIndex& index) {
	DEBUG("XYConvolutionCurveDock::xDataColumnChanged()");
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setXDataColumn(column);

	if (column != nullptr) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			uiGeneralTab.sbMin->setValue(column->minimum());
			uiGeneralTab.sbMax->setValue(column->maximum());
		}
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYConvolutionCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;
	DEBUG("yDataColumnChanged()");

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYConvolutionCurveDock::y2DataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;
	DEBUG("y2DataColumnChanged()");

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYConvolutionCurve*>(curve)->setY2DataColumn(column);

	cbY2DataColumn->useCurrentIndexText(true);
	cbY2DataColumn->setInvalid(false);
}

void XYConvolutionCurveDock::samplingIntervalChanged() {
	double samplingInterval =  uiGeneralTab.sbSamplingInterval->value();
	m_convolutionData.samplingInterval = samplingInterval;

	enableRecalculate();
}

void XYConvolutionCurveDock::kernelChanged() {
	auto kernel = (nsl_conv_kernel_type) uiGeneralTab.cbKernel->currentIndex();
	m_convolutionData.kernel = kernel;

	//TODO: change selectable sizes
	uiGeneralTab.sbKernelSize->setEnabled(true);
	switch (kernel) {
	case nsl_conv_kernel_avg:	// all values allowed
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
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_convolutionCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_convolutionCurve->xDataColumn();
		else {
			if (m_convolutionCurve->dataSourceCurve())
				xDataColumn = m_convolutionCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			uiGeneralTab.sbMin->setValue(xDataColumn->minimum());
			uiGeneralTab.sbMax->setValue(xDataColumn->maximum());
		}
	} else {
		uiGeneralTab.lMin->setEnabled(true);
		uiGeneralTab.sbMin->setEnabled(true);
		uiGeneralTab.lMax->setEnabled(true);
		uiGeneralTab.sbMax->setEnabled(true);
	}

}
void XYConvolutionCurveDock::xRangeMinChanged() {
	double xMin = uiGeneralTab.sbMin->value();

	m_convolutionData.xRange.first() = xMin;
	enableRecalculate();
}

void XYConvolutionCurveDock::xRangeMaxChanged() {
	double xMax = uiGeneralTab.sbMax->value();

	m_convolutionData.xRange.last() = xMax;
	enableRecalculate();
}

void XYConvolutionCurveDock::directionChanged() {
	DEBUG("XYConvolutionCurveDock::directionChanged()");
	auto dir = (nsl_conv_direction_type) uiGeneralTab.cbDirection->currentIndex();
	m_convolutionData.direction = dir;

	// change name if still default
	if ( m_curve->name().compare(i18n("Convolution")) == 0  && dir == nsl_conv_direction_backward) {
		m_curve->setName(i18n("Deconvolution"));
		uiGeneralTab.leName->setText(m_curve->name());
	}
	if (m_curve->name().compare(i18n("Deconvolution")) == 0  && dir == nsl_conv_direction_forward) {
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
		dynamic_cast<XYConvolutionCurve*>(curve)->setConvolutionData(m_convolutionData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	if (m_convolutionData.direction == nsl_conv_direction_forward)
		emit info(i18n("Convolution status: %1", m_convolutionCurve->convolutionResult().status));
	else
		emit info(i18n("Deconvolution status: %1", m_convolutionCurve->convolutionResult().status));
	QApplication::restoreOverrideCursor();
}

void XYConvolutionCurveDock::enableRecalculate() const {
	DEBUG("XYConvolutionCurveDock::enableRecalculate()");
	if (m_initializing)
		return;

	bool hasSourceData = false;
	//no convolution possible without the y-data
	if (m_convolutionCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectY != nullptr);
		if (aspectY) {
			cbYDataColumn->useCurrentIndexText(true);
			cbYDataColumn->setInvalid(false);
		}
	} else {
		 hasSourceData = (m_convolutionCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the convolution
 */
void XYConvolutionCurveDock::showConvolutionResult() {
	const XYConvolutionCurve::ConvolutionResult& convolutionResult = m_convolutionCurve->convolutionResult();
	if (!convolutionResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", convolutionResult.status) + "<br>";

	if (!convolutionResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	SET_NUMBER_LOCALE
	if (convolutionResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", numberLocale.toString(convolutionResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", numberLocale.toString(convolutionResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last convolution
	uiGeneralTab.pbRecalculate->setEnabled(m_convolutionCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYConvolutionCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYConvolutionCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void XYConvolutionCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	cbDataSourceCurve->setAspect(curve);
	m_initializing = false;
}

void XYConvolutionCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYConvolutionCurveDock::curveXDataColumnChanged()");
	m_initializing = true;
	cbXDataColumn->setColumn(column, m_convolutionCurve->xDataColumnPath());
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
	m_initializing = false;
}

void XYConvolutionCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYConvolutionCurveDock::curveYDataColumnChanged()");
	m_initializing = true;
	cbYDataColumn->setColumn(column, m_convolutionCurve->yDataColumnPath());
	m_initializing = false;
}

void XYConvolutionCurveDock::curveY2DataColumnChanged(const AbstractColumn* column) {
	DEBUG("XYConvolutionCurveDock::curveY2DataColumnChanged()");
	m_initializing = true;
	cbY2DataColumn->setColumn(column, m_convolutionCurve->y2DataColumnPath());
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
	m_initializing = false;
}

void XYConvolutionCurveDock::curveConvolutionDataChanged(const XYConvolutionCurve::ConvolutionData& convolutionData) {
	m_initializing = true;
	m_convolutionData = convolutionData;
	this->directionChanged();

	this->showConvolutionResult();
	m_initializing = false;
}

void XYConvolutionCurveDock::dataChanged() {
	this->enableRecalculate();
}

void XYConvolutionCurveDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	uiGeneralTab.chkVisible->setChecked(on);
	m_initializing = false;
}
