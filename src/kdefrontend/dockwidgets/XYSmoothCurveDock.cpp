/***************************************************************************
    File             : XYSmoothCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright        : (C) 2017-2020 Alexander Semke (alexander.semke@web.de)
    Description      : widget for editing properties of smooth curves

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

#include "XYSmoothCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QStandardItemModel>

/*!
  \class XYSmoothCurveDock
 \brief  Provides a widget for editing the properties of the XYSmoothCurves
		(2D-curves defined by an smooth) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYSmoothCurveDock::XYSmoothCurveDock(QWidget* parent) : XYCurveDock(parent) {
	//hide the line connection type
	ui.cbLineType->setDisabled(true);

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYSmoothCurveDock::setupGeneral() {
	DEBUG("XYSmoothCurveDock::setupGeneral()");

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
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 2);
	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 2);

	for (int i = 0; i < NSL_SMOOTH_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_smooth_type_name[i]));

	for (int i = 0; i < NSL_SMOOTH_WEIGHT_TYPE_COUNT; i++)
		uiGeneralTab.cbWeight->addItem(i18n(nsl_smooth_weight_type_name[i]));

	for (int i = 0; i < NSL_SMOOTH_PAD_MODE_COUNT; i++)
		uiGeneralTab.cbMode->addItem(i18n(nsl_smooth_pad_mode_name[i]));

	uiGeneralTab.sbMin->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	uiGeneralTab.sbMax->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYSmoothCurveDock::nameChanged );
	connect(uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYSmoothCurveDock::commentChanged );
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYSmoothCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYSmoothCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYSmoothCurveDock::autoRangeChanged);
	connect(uiGeneralTab.sbMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYSmoothCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.sbMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYSmoothCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &QDateTimeEdit::dateTimeChanged, this, &XYSmoothCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &QDateTimeEdit::dateTimeChanged, this, &XYSmoothCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYSmoothCurveDock::typeChanged);
	connect(uiGeneralTab.sbPoints, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYSmoothCurveDock::pointsChanged);
	connect(uiGeneralTab.cbWeight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYSmoothCurveDock::weightChanged);
	connect(uiGeneralTab.sbPercentile, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYSmoothCurveDock::percentileChanged);
	connect(uiGeneralTab.sbOrder, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYSmoothCurveDock::orderChanged);
	connect(uiGeneralTab.cbMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYSmoothCurveDock::modeChanged);
	connect(uiGeneralTab.sbLeftValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYSmoothCurveDock::valueChanged);
	connect(uiGeneralTab.sbRightValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYSmoothCurveDock::valueChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYSmoothCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYSmoothCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYSmoothCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYSmoothCurveDock::yDataColumnChanged);
}

void XYSmoothCurveDock::initGeneralTab() {
	DEBUG("XYSmoothCurveDock::initGeneralTab()");

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
	m_smoothCurve = static_cast<XYSmoothCurve*>(m_curve);
	checkColumnAvailability(cbXDataColumn, m_smoothCurve->xDataColumn(), m_smoothCurve->xDataColumnPath());
	checkColumnAvailability(cbYDataColumn, m_smoothCurve->yDataColumn(), m_smoothCurve->yDataColumnPath());

	//data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(m_smoothCurve->dataSourceType());
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_smoothCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_smoothCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_smoothCurve->yDataColumn());

	//range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_smoothCurve->parentAspect());
	m_dateTimeRange = (plot->xRangeFormat() != CartesianPlot::Numeric);
	if (!m_dateTimeRange) {
		uiGeneralTab.sbMin->setValue(m_smoothData.xRange.first());
		uiGeneralTab.sbMax->setValue(m_smoothData.xRange.last());
	} else {
		uiGeneralTab.dateTimeEditMin->setDateTime( QDateTime::fromMSecsSinceEpoch(m_smoothData.xRange.first()) );
		uiGeneralTab.dateTimeEditMax->setDateTime( QDateTime::fromMSecsSinceEpoch(m_smoothData.xRange.last()) );
	}

	uiGeneralTab.lMin->setVisible(!m_dateTimeRange);
	uiGeneralTab.sbMin->setVisible(!m_dateTimeRange);
	uiGeneralTab.lMax->setVisible(!m_dateTimeRange);
	uiGeneralTab.sbMax->setVisible(!m_dateTimeRange);
	uiGeneralTab.lMinDateTime->setVisible(m_dateTimeRange);
	uiGeneralTab.dateTimeEditMin->setVisible(m_dateTimeRange);
	uiGeneralTab.lMaxDateTime->setVisible(m_dateTimeRange);
	uiGeneralTab.dateTimeEditMax->setVisible(m_dateTimeRange);

	//auto range
	uiGeneralTab.cbAutoRange->setChecked(m_smoothData.autoRange);
	this->autoRangeChanged();

	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbType->setCurrentIndex(m_smoothData.type);
	typeChanged(uiGeneralTab.cbType->currentIndex());	// needed, when type does not change
	uiGeneralTab.sbPoints->setValue((int)m_smoothData.points);
	uiGeneralTab.cbWeight->setCurrentIndex(m_smoothData.weight);
	uiGeneralTab.sbPercentile->setValue(m_smoothData.percentile);
	uiGeneralTab.sbOrder->setValue((int)m_smoothData.order);
	uiGeneralTab.cbMode->setCurrentIndex(m_smoothData.mode);
	modeChanged(uiGeneralTab.cbMode->currentIndex());	// needed, when mode does not change
	uiGeneralTab.sbLeftValue->setValue(m_smoothData.lvalue);
	uiGeneralTab.sbRightValue->setValue(m_smoothData.rvalue);
	valueChanged();
	this->showSmoothResult();

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_smoothCurve, &XYSmoothCurve::aspectDescriptionChanged, this, &XYSmoothCurveDock::curveDescriptionChanged);
	connect(m_smoothCurve, &XYSmoothCurve::dataSourceTypeChanged, this, &XYSmoothCurveDock::curveDataSourceTypeChanged);
	connect(m_smoothCurve, &XYSmoothCurve::dataSourceCurveChanged, this, &XYSmoothCurveDock::curveDataSourceCurveChanged);
	connect(m_smoothCurve, &XYSmoothCurve::xDataColumnChanged, this, &XYSmoothCurveDock::curveXDataColumnChanged);
	connect(m_smoothCurve, &XYSmoothCurve::yDataColumnChanged, this, &XYSmoothCurveDock::curveYDataColumnChanged);
	connect(m_smoothCurve, &XYSmoothCurve::smoothDataChanged, this, &XYSmoothCurveDock::curveSmoothDataChanged);
	connect(m_smoothCurve, &XYSmoothCurve::sourceDataChanged, this, &XYSmoothCurveDock::enableRecalculate);
}

void XYSmoothCurveDock::setModel() {
	QList<AspectType> list{AspectType::Folder, AspectType::Datapicker, AspectType::Worksheet,
	                       AspectType::CartesianPlot, AspectType::XYCurve};
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list = {AspectType::Folder, AspectType::Workbook, AspectType::Datapicker,
	        AspectType::DatapickerCurve, AspectType::Spreadsheet, AspectType::LiveDataSource,
	        AspectType::Column, AspectType::Worksheet, AspectType::CartesianPlot,
	        AspectType::XYFitCurve, AspectType::CantorWorksheet
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
void XYSmoothCurveDock::setCurves(QList<XYCurve*> list) {
	DEBUG("XYSmoothCurveDock::setCurves()");

	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = m_curve;
	m_smoothCurve = dynamic_cast<XYSmoothCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_smoothData = m_smoothCurve->smoothData();
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
void XYSmoothCurveDock::dataSourceTypeChanged(int index) {
	auto type = (XYAnalysisCurve::DataSourceType)index;
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
		dynamic_cast<XYSmoothCurve*>(curve)->setDataSourceType(type);
}

void XYSmoothCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYSmoothCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYSmoothCurveDock::xDataColumnChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYSmoothCurve*>(curve)->setXDataColumn(column);

	// disable types that need more data points
	if (column != nullptr) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			uiGeneralTab.sbMin->setValue(column->minimum());
			uiGeneralTab.sbMax->setValue(column->maximum());
		}

		unsigned int n = 0;
		for (int row = 0; row < column->rowCount(); row++) {
			if (column->isMasked(row))
				continue;

			switch (column->columnMode()) {
			case AbstractColumn::Numeric:
			case AbstractColumn::Integer:
			case AbstractColumn::BigInt:
				if (!std::isnan(column->valueAt(row)))
					n++;
				break;
			case AbstractColumn::DateTime:
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				if (!std::isnan(column->dateTimeAt(row).toMSecsSinceEpoch()))
					n++;
				break;
			case AbstractColumn::Text:
				break;
			}
		}

		DEBUG("	Set maximum points to " << n)
		// set maximum of sbPoints to number of columns
		uiGeneralTab.sbPoints->setMaximum((int)n);
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYSmoothCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYSmoothCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYSmoothCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_smoothData.autoRange = autoRange;

	uiGeneralTab.lMin->setEnabled(!autoRange);
	uiGeneralTab.sbMin->setEnabled(!autoRange);
	uiGeneralTab.lMax->setEnabled(!autoRange);
	uiGeneralTab.sbMax->setEnabled(!autoRange);
	uiGeneralTab.lMinDateTime->setEnabled(!autoRange);
	uiGeneralTab.dateTimeEditMin->setEnabled(!autoRange);
	uiGeneralTab.lMaxDateTime->setEnabled(!autoRange);
	uiGeneralTab.dateTimeEditMax->setEnabled(!autoRange);

	if (autoRange) {
		const AbstractColumn* xDataColumn = nullptr;
		if (m_smoothCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
			xDataColumn = m_smoothCurve->xDataColumn();
		else {
			if (m_smoothCurve->dataSourceCurve())
				xDataColumn = m_smoothCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			if (!m_dateTimeRange) {
				uiGeneralTab.sbMin->setValue(xDataColumn->minimum());
				uiGeneralTab.sbMax->setValue(xDataColumn->maximum());
			} else {
				uiGeneralTab.dateTimeEditMin->setDateTime(QDateTime::fromMSecsSinceEpoch(xDataColumn->minimum()));
				uiGeneralTab.dateTimeEditMax->setDateTime(QDateTime::fromMSecsSinceEpoch(xDataColumn->maximum()));
			}
		}
	}
}

void XYSmoothCurveDock::xRangeMinChanged(double value) {
	m_smoothData.xRange.first() = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::xRangeMaxChanged(double value) {
	m_smoothData.xRange.last() = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::xRangeMinDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	m_smoothData.xRange.first() = dateTime.toMSecsSinceEpoch();
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::xRangeMaxDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	m_smoothData.xRange.last() = dateTime.toMSecsSinceEpoch();
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::typeChanged(int index) {
	auto type = (nsl_smooth_type)index;
	m_smoothData.type = type;

	const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbMode->model());
	QStandardItem* pad_interp_item = model->item(nsl_smooth_pad_interp);
	if (type == nsl_smooth_type_moving_average || type == nsl_smooth_type_moving_average_lagged) {
		uiGeneralTab.lWeight->show();
		uiGeneralTab.cbWeight->show();
		// disable interp pad model for MA and MAL
		pad_interp_item->setFlags(pad_interp_item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
	} else {
		uiGeneralTab.lWeight->hide();
		uiGeneralTab.cbWeight->hide();
		pad_interp_item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
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
		pad_interp_item->setFlags(pad_interp_item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
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
		dynamic_cast<XYSmoothCurve*>(curve)->setSmoothData(m_smoothData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Smoothing status: %1", m_smoothCurve->smoothResult().status));
	QApplication::restoreOverrideCursor();
}

void XYSmoothCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no smoothing possible without the x- and y-data
	bool hasSourceData = false;
	if (m_smoothCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet) {
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
		 hasSourceData = (m_smoothCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the smooth
 */
void XYSmoothCurveDock::showSmoothResult() {
	const XYSmoothCurve::SmoothResult& smoothResult = m_smoothCurve->smoothResult();
	if (!smoothResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	//const XYSmoothCurve::SmoothData& smoothData = m_smoothCurve->smoothData();
	QString str = i18n("status: %1", smoothResult.status) + "<br>";

	if (!smoothResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (smoothResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s", QString::number(smoothResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", QString::number(smoothResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last smooth
	uiGeneralTab.pbRecalculate->setEnabled(m_smoothCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYSmoothCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYSmoothCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(type);
	m_initializing = false;
}

void XYSmoothCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYSmoothCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	m_initializing = false;
}

void XYSmoothCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYSmoothCurveDock::curveSmoothDataChanged(const XYSmoothCurve::SmoothData& smoothData) {
	m_initializing = true;
	m_smoothData = smoothData;
	uiGeneralTab.cbType->setCurrentIndex(m_smoothData.type);

	this->showSmoothResult();
	m_initializing = false;
}

void XYSmoothCurveDock::dataChanged() {
	this->enableRecalculate();
}
