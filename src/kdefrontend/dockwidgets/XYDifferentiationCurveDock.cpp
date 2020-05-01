/***************************************************************************
    File             : XYDifferentiationCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright        : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Description      : widget for editing properties of differentiation curves

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

#include "XYDifferentiationCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

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

  \ingroup kdefrontend
*/

XYDifferentiationCurveDock::XYDifferentiationCurveDock(QWidget* parent) : XYCurveDock(parent) {
	//hide the line connection type
	ui.cbLineType->setDisabled(true);

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYDifferentiationCurveDock::setupGeneral() {
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
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 3);

	for (int i = 0; i < NSL_DIFF_DERIV_ORDER_COUNT; ++i)
		uiGeneralTab.cbDerivOrder->addItem(i18n(nsl_diff_deriv_order_name[i]));

	uiGeneralTab.sbMin->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	uiGeneralTab.sbMax->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	uiGeneralTab.pbRecalculate->setIcon( QIcon::fromTheme("run-build") );

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYDifferentiationCurveDock::nameChanged);
	connect(uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYDifferentiationCurveDock::commentChanged);

	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYDifferentiationCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYDifferentiationCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYDifferentiationCurveDock::autoRangeChanged);
	connect(uiGeneralTab.sbMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYDifferentiationCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.sbMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYDifferentiationCurveDock::xRangeMaxChanged);
	connect( uiGeneralTab.cbDerivOrder, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYDifferentiationCurveDock::derivOrderChanged);
	connect( uiGeneralTab.sbAccOrder, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYDifferentiationCurveDock::accOrderChanged);
	connect( uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYDifferentiationCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYDifferentiationCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYDifferentiationCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYDifferentiationCurveDock::yDataColumnChanged);
}

void XYDifferentiationCurveDock::initGeneralTab() {
	//if there are more than one curve in the list, disable the tab "general"
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

	auto* analysisCurve = dynamic_cast<XYAnalysisCurve*>(m_curve);
	checkColumnAvailability(cbXDataColumn, analysisCurve->xDataColumn(), analysisCurve->xDataColumnPath());
	checkColumnAvailability(cbYDataColumn, analysisCurve->yDataColumn(), analysisCurve->yDataColumnPath());

	//show the properties of the first curve
	m_differentiationCurve = dynamic_cast<XYDifferentiationCurve*>(m_curve);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(m_differentiationCurve->dataSourceType());
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_differentiationCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_differentiationCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_differentiationCurve->yDataColumn());
	uiGeneralTab.cbAutoRange->setChecked(m_differentiationData.autoRange);
	uiGeneralTab.sbMin->setValue(m_differentiationData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_differentiationData.xRange.last());
	this->autoRangeChanged();
	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbDerivOrder->setCurrentIndex(m_differentiationData.derivOrder);
	this->derivOrderChanged();
	uiGeneralTab.sbAccOrder->setValue(m_differentiationData.accOrder);
	this->accOrderChanged();

	this->showDifferentiationResult();

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_differentiationCurve, &XYDifferentiationCurve::aspectDescriptionChanged, this, &XYDifferentiationCurveDock::curveDescriptionChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::dataSourceTypeChanged, this, &XYDifferentiationCurveDock::curveDataSourceTypeChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::dataSourceCurveChanged, this, &XYDifferentiationCurveDock::curveDataSourceCurveChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::xDataColumnChanged, this, &XYDifferentiationCurveDock::curveXDataColumnChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::yDataColumnChanged, this, &XYDifferentiationCurveDock::curveYDataColumnChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::differentiationDataChanged, this, &XYDifferentiationCurveDock::curveDifferentiationDataChanged);
	connect(m_differentiationCurve, &XYDifferentiationCurve::sourceDataChanged, this, &XYDifferentiationCurveDock::enableRecalculate);
}

void XYDifferentiationCurveDock::setModel() {
	QList<AspectType> list{AspectType::Folder, AspectType::Datapicker, AspectType::Worksheet,
	                       AspectType::CartesianPlot, AspectType::XYCurve};
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list = {AspectType::Folder, AspectType::Workbook, AspectType::Datapicker,
	        AspectType::DatapickerCurve, AspectType::Spreadsheet, AspectType::LiveDataSource,
	        AspectType::Column, AspectType::Worksheet, AspectType::CartesianPlot, AspectType::XYFitCurve
	       };
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
void XYDifferentiationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = m_curve;
	m_differentiationCurve = dynamic_cast<XYDifferentiationCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_differentiationData = m_differentiationCurve->differentiationData();
	initGeneralTab();
	initTabs();
	m_initializing = false;

	//hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYDifferentiationCurveDock::dataSourceTypeChanged(int index) {
	const auto type = (XYAnalysisCurve::DataSourceType)index;
	if (type == XYAnalysisCurve::DataSourceSpreadsheet) {
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

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDifferentiationCurve*>(curve)->setDataSourceType(type);
}

void XYDifferentiationCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	// disable deriv orders and accuracies that need more data points
	this->updateSettings(dataSourceCurve->xColumn());

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDifferentiationCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYDifferentiationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	// disable deriv orders and accuracies that need more data points
	this->updateSettings(column);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDifferentiationCurve*>(curve)->setXDataColumn(column);

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYDifferentiationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDifferentiationCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

/*!
 * disable deriv orders and accuracies that need more data points
 */
void XYDifferentiationCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	if (uiGeneralTab.cbAutoRange->isChecked()) {
		uiGeneralTab.sbMin->setValue(column->minimum());
		uiGeneralTab.sbMax->setValue(column->maximum());
	}

	size_t n = 0;
	for (int row = 0; row < column->rowCount(); ++row)
		if (!std::isnan(column->valueAt(row)) && !column->isMasked(row))
			n++;

	const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbDerivOrder->model());
	QStandardItem* item = model->item(nsl_diff_deriv_order_first);
	if (n < 3)
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
	else {
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
		if (n < 5)
			uiGeneralTab.sbAccOrder->setMinimum(2);
	}

	item = model->item(nsl_diff_deriv_order_second);
	if (n < 3) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_second)
				uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	}
	else {
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
		if (n < 4)
			uiGeneralTab.sbAccOrder->setMinimum(1);
		else if (n < 5)
			uiGeneralTab.sbAccOrder->setMinimum(2);
	}

	item = model->item(nsl_diff_deriv_order_third);
	if (n < 5) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_third)
				uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

	item = model->item(nsl_diff_deriv_order_fourth);
	if (n < 5) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_fourth)
				uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	}
	else {
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
		if (n < 7)
			uiGeneralTab.sbAccOrder->setMinimum(1);
	}

	item = model->item(nsl_diff_deriv_order_fifth);
	if (n < 7) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_fifth)
				uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

	item = model->item(nsl_diff_deriv_order_sixth);
	if (n < 7) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbDerivOrder->currentIndex() == nsl_diff_deriv_order_sixth)
				uiGeneralTab.cbDerivOrder->setCurrentIndex(nsl_diff_deriv_order_first);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}

void XYDifferentiationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_differentiationData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_differentiationCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
			xDataColumn = m_differentiationCurve->xDataColumn();
		else {
			if (m_differentiationCurve->dataSourceCurve())
				xDataColumn = m_differentiationCurve->dataSourceCurve()->xColumn();
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

void XYDifferentiationCurveDock::xRangeMinChanged() {
	double xMin = uiGeneralTab.sbMin->value();

	m_differentiationData.xRange.first() = xMin;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYDifferentiationCurveDock::xRangeMaxChanged() {
	double xMax = uiGeneralTab.sbMax->value();

	m_differentiationData.xRange.last() = xMax;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYDifferentiationCurveDock::derivOrderChanged() {
	const auto derivOrder = (nsl_diff_deriv_order_type)uiGeneralTab.cbDerivOrder->currentIndex();
	m_differentiationData.derivOrder = derivOrder;

	// update avail. accuracies
	switch (derivOrder) {
	case nsl_diff_deriv_order_first:
		uiGeneralTab.sbAccOrder->setMinimum(2);
		uiGeneralTab.sbAccOrder->setMaximum(4);
		uiGeneralTab.sbAccOrder->setSingleStep(2);
		uiGeneralTab.sbAccOrder->setValue(4);
		break;
	case nsl_diff_deriv_order_second:
		uiGeneralTab.sbAccOrder->setMinimum(1);
		uiGeneralTab.sbAccOrder->setMaximum(3);
		uiGeneralTab.sbAccOrder->setSingleStep(1);
		uiGeneralTab.sbAccOrder->setValue(3);
		break;
	case nsl_diff_deriv_order_third:
		uiGeneralTab.sbAccOrder->setMinimum(2);
		uiGeneralTab.sbAccOrder->setMaximum(2);
		break;
	case nsl_diff_deriv_order_fourth:
		uiGeneralTab.sbAccOrder->setMinimum(1);
		uiGeneralTab.sbAccOrder->setMaximum(3);
		uiGeneralTab.sbAccOrder->setSingleStep(2);
		uiGeneralTab.sbAccOrder->setValue(3);
		break;
	case nsl_diff_deriv_order_fifth:
		uiGeneralTab.sbAccOrder->setMinimum(2);
		uiGeneralTab.sbAccOrder->setMaximum(2);
		break;
	case nsl_diff_deriv_order_sixth:
		uiGeneralTab.sbAccOrder->setMinimum(1);
		uiGeneralTab.sbAccOrder->setMaximum(1);
		break;
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYDifferentiationCurveDock::accOrderChanged() {
	const auto accOrder = (int)uiGeneralTab.sbAccOrder->value();
	m_differentiationData.accOrder = accOrder;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYDifferentiationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		if (curve != nullptr)
			dynamic_cast<XYDifferentiationCurve*>(curve)->setDifferentiationData(m_differentiationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Differentiation status: %1", m_differentiationCurve->differentiationResult().status));
	QApplication::restoreOverrideCursor();
}

void XYDifferentiationCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no differentiation possible without the x- and y-data
	bool hasSourceData = false;
	if (m_differentiationCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet) {
		AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectX != nullptr && aspectY != nullptr);
		if (aspectX) {
			cbXDataColumn->useCurrentIndexText(true);
			cbXDataColumn->setInvalid(false);
		}
		if (aspectY) {
			cbYDataColumn->useCurrentIndexText(true);
			cbYDataColumn->setInvalid(false);
		}
	} else {
		 hasSourceData = (m_differentiationCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the differentiation
 */
void XYDifferentiationCurveDock::showDifferentiationResult() {
	const XYDifferentiationCurve::DifferentiationResult& differentiationResult = m_differentiationCurve->differentiationResult();
	if (!differentiationResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", differentiationResult.status) + "<br>";

	if (!differentiationResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (differentiationResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s", QString::number(differentiationResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", QString::number(differentiationResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last differentiation
	uiGeneralTab.pbRecalculate->setEnabled(m_differentiationCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*** SLOTs for changes triggered in XYDifferentiationCurve ***
//*************************************************************
//General-Tab
void XYDifferentiationCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYDifferentiationCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(type);
	m_initializing = false;
}

void XYDifferentiationCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYDifferentiationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	m_initializing = false;
}

void XYDifferentiationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYDifferentiationCurveDock::curveDifferentiationDataChanged(const XYDifferentiationCurve::DifferentiationData& differentiationData) {
	m_initializing = true;
	m_differentiationData = differentiationData;
	uiGeneralTab.cbDerivOrder->setCurrentIndex(m_differentiationData.derivOrder);
	this->derivOrderChanged();
	uiGeneralTab.sbAccOrder->setValue(m_differentiationData.accOrder);
	this->accOrderChanged();

	this->showDifferentiationResult();
	m_initializing = false;
}

void XYDifferentiationCurveDock::dataChanged() {
	this->enableRecalculate();
}
