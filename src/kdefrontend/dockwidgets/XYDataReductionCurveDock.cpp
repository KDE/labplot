/***************************************************************************
    File             : XYDataReductionCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright        : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Description      : widget for editing properties of data reduction curves

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

#include "XYDataReductionCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QProgressBar>

/*!
  \class XYDataReductionCurveDock
 \brief  Provides a widget for editing the properties of the XYDataReductionCurves
		(2D-curves defined by an data reduction) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYDataReductionCurveDock::XYDataReductionCurveDock(QWidget* parent, QStatusBar* sb) : XYCurveDock(parent), statusBar(sb) {
}

/*!
 * 	// Tab "General"
 */
void XYDataReductionCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_teComment = uiGeneralTab.teComment;
	m_teComment->setFixedHeight(m_leName->height());

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

	for (int i = 0; i < NSL_GEOM_LINESIM_TYPE_COUNT; ++i)
		uiGeneralTab.cbType->addItem(i18n(nsl_geom_linesim_type_name[i]));
	uiGeneralTab.cbType->setItemData(nsl_geom_linesim_type_visvalingam_whyatt, i18n("This method is much slower than any other"), Qt::ToolTipRole);

	uiGeneralTab.leMin->setValidator( new QDoubleValidator(uiGeneralTab.leMin) );
	uiGeneralTab.leMax->setValidator( new QDoubleValidator(uiGeneralTab.leMax) );
	uiGeneralTab.sbTolerance->setRange(0.0, std::numeric_limits<double>::max());
	uiGeneralTab.sbTolerance2->setRange(0.0, std::numeric_limits<double>::max());

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYDataReductionCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYDataReductionCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYDataReductionCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYDataReductionCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYDataReductionCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &QDateTimeEdit::dateTimeChanged, this, &XYDataReductionCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &QDateTimeEdit::dateTimeChanged, this, &XYDataReductionCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.chkAuto, &QCheckBox::clicked, this, &XYDataReductionCurveDock::autoToleranceChanged);
	connect(uiGeneralTab.sbTolerance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYDataReductionCurveDock::toleranceChanged);
	connect(uiGeneralTab.chkAuto2, &QCheckBox::clicked, this, &XYDataReductionCurveDock::autoTolerance2Changed);
	connect(uiGeneralTab.sbTolerance2, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYDataReductionCurveDock::tolerance2Changed);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYDataReductionCurveDock::plotRangeChanged );
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYDataReductionCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYDataReductionCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYDataReductionCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYDataReductionCurveDock::yDataColumnChanged);
}

void XYDataReductionCurveDock::initGeneralTab() {
	//if there are more then one curve in the list, disable the tab "general"
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

	//show the properties of the first curve
	//data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_dataReductionCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_dataReductionCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_dataReductionCurve->xDataColumn(), m_dataReductionCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_dataReductionCurve->yDataColumn(), m_dataReductionCurve->yDataColumnPath());

	//range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_dataReductionCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_curve->coordinateSystemIndex())->xIndex();
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		SET_NUMBER_LOCALE
		uiGeneralTab.leMin->setText( numberLocale.toString(m_dataReductionData.xRange.first()) );
		uiGeneralTab.leMax->setText( numberLocale.toString(m_dataReductionData.xRange.last()) );
	} else {
		uiGeneralTab.dateTimeEditMin->setDateTime( QDateTime::fromMSecsSinceEpoch(m_dataReductionData.xRange.first()) );
		uiGeneralTab.dateTimeEditMax->setDateTime( QDateTime::fromMSecsSinceEpoch(m_dataReductionData.xRange.last()) );
	}

	uiGeneralTab.lMin->setVisible(!m_dateTimeRange);
	uiGeneralTab.leMin->setVisible(!m_dateTimeRange);
	uiGeneralTab.lMax->setVisible(!m_dateTimeRange);
	uiGeneralTab.leMax->setVisible(!m_dateTimeRange);
	uiGeneralTab.lMinDateTime->setVisible(m_dateTimeRange);
	uiGeneralTab.dateTimeEditMin->setVisible(m_dateTimeRange);
	uiGeneralTab.lMaxDateTime->setVisible(m_dateTimeRange);
	uiGeneralTab.dateTimeEditMax->setVisible(m_dateTimeRange);

	//auto range
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

	//enable the "recalculate"-button if the source data was changed since the last dataReduction
	uiGeneralTab.pbRecalculate->setEnabled(m_dataReductionCurve->isSourceDataChangedSinceLastRecalc());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_dataReductionCurve, &XYDataReductionCurve::aspectDescriptionChanged, this, &XYDataReductionCurveDock::aspectDescriptionChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::dataSourceTypeChanged, this, &XYDataReductionCurveDock::curveDataSourceTypeChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::dataSourceCurveChanged, this, &XYDataReductionCurveDock::curveDataSourceCurveChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::xDataColumnChanged, this, &XYDataReductionCurveDock::curveXDataColumnChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::yDataColumnChanged, this, &XYDataReductionCurveDock::curveYDataColumnChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::dataReductionDataChanged, this, &XYDataReductionCurveDock::curveDataReductionDataChanged);
	connect(m_dataReductionCurve, &XYDataReductionCurve::sourceDataChanged, this, &XYDataReductionCurveDock::enableRecalculate);
	connect(m_dataReductionCurve, &WorksheetElement::plotRangeListChanged, this, &XYDataReductionCurveDock::updatePlotRanges);
	connect(m_dataReductionCurve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &XYDataReductionCurveDock::curveVisibilityChanged);
}

void XYDataReductionCurveDock::setModel() {
	QList<AspectType>  list{AspectType::Folder, AspectType::Datapicker, AspectType::Worksheet,
	                        AspectType::CartesianPlot, AspectType::XYCurve, AspectType::XYAnalysisCurve};
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list = {AspectType::Folder, AspectType::Workbook, AspectType::Datapicker,
	        AspectType::DatapickerCurve, AspectType::Spreadsheet, AspectType::LiveDataSource,
	        AspectType::Column, AspectType::Worksheet, AspectType::CartesianPlot,
	        AspectType::XYFitCurve
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
void XYDataReductionCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = m_curve;
	m_dataReductionCurve = static_cast<XYDataReductionCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_dataReductionData = m_dataReductionCurve->dataReductionData();

	SET_NUMBER_LOCALE
	uiGeneralTab.sbTolerance->setLocale(numberLocale);
	uiGeneralTab.sbTolerance2->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	m_initializing = false;

	updatePlotRanges();

	//hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

void XYDataReductionCurveDock::updatePlotRanges() const {
	updatePlotRangeList(uiGeneralTab.cbPlotRanges);
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

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDataReductionCurve*>(curve)->setDataSourceType(type);
}

void XYDataReductionCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

// 	// disable deriv orders and accuracies that need more data points
// 	this->updateSettings(dataSourceCurve->xColumn());

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDataReductionCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYDataReductionCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDataReductionCurve*>(curve)->setXDataColumn(column);

	//TODO: this->updateSettings(column); ?
	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		SET_NUMBER_LOCALE
		uiGeneralTab.leMin->setText( numberLocale.toString(column->minimum()) );
		uiGeneralTab.leMax->setText( numberLocale.toString(column->maximum()) );
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);

	updateTolerance();
	updateTolerance2();
}

void XYDataReductionCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDataReductionCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);

	updateTolerance();
	updateTolerance2();
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

	//copy all valid data points for calculating tolerance to temporary vectors
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
		m_dataReductionData.tolerance = xdataVector.size()/10.;	// reduction to 10%
	else
		m_dataReductionData.tolerance = 2.*nsl_geom_linesim_avg_dist_perpoint(xdataVector.data(), ydataVector.data(), xdataVector.size());
	//m_dataReductionData.tolerance = nsl_geom_linesim_clip_diag_perpoint(xdataVector.data(), ydataVector.data(), xdataVector.size());
	uiGeneralTab.sbTolerance->setValue(m_dataReductionData.tolerance);
}

void XYDataReductionCurveDock::updateTolerance2() {
	const auto type = (nsl_geom_linesim_type)uiGeneralTab.cbType->currentIndex();

	if (type == nsl_geom_linesim_type_perpdist)
		uiGeneralTab.sbTolerance2->setValue(10);
	else if (type == nsl_geom_linesim_type_opheim)
		uiGeneralTab.sbTolerance2->setValue(5*uiGeneralTab.sbTolerance->value());
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
				SET_NUMBER_LOCALE
				uiGeneralTab.leMin->setText( numberLocale.toString(xDataColumn->minimum()) );
				uiGeneralTab.leMax->setText( numberLocale.toString(xDataColumn->maximum()) );
			} else {
				uiGeneralTab.dateTimeEditMin->setDateTime(QDateTime::fromMSecsSinceEpoch(xDataColumn->minimum()));
				uiGeneralTab.dateTimeEditMax->setDateTime(QDateTime::fromMSecsSinceEpoch(xDataColumn->maximum()));
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

void XYDataReductionCurveDock::xRangeMinDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	m_dataReductionData.xRange.first() = dateTime.toMSecsSinceEpoch();
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYDataReductionCurveDock::xRangeMaxDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	m_dataReductionData.xRange.last() = dateTime.toMSecsSinceEpoch();
	uiGeneralTab.pbRecalculate->setEnabled(true);
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
	case nsl_geom_linesim_type_perpdist:	// repeat option
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
	case nsl_geom_linesim_type_opheim:	// min/max tol options
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
	case nsl_geom_linesim_type_lang:	// distance/region
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

	uiGeneralTab.pbRecalculate->setEnabled(true);
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
	uiGeneralTab.pbRecalculate->setEnabled(true);
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
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYDataReductionCurveDock::recalculateClicked() {
	//show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	connect(m_curve, SIGNAL(completed(int)), progressBar, SLOT(setValue(int)));
	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		dynamic_cast<XYDataReductionCurve*>(curve)->setDataReductionData(m_dataReductionData);

	QApplication::restoreOverrideCursor();
	statusBar->removeWidget(progressBar);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Data reduction status: %1", m_dataReductionCurve->dataReductionResult().status));
}

void XYDataReductionCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no dataReductioning possible without the x- and y-data
	bool hasSourceData = false;
	if (m_dataReductionCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
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
		 hasSourceData = (m_dataReductionCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the dataReduction
 */
void XYDataReductionCurveDock::showDataReductionResult() {
	const XYDataReductionCurve::DataReductionResult& dataReductionResult = m_dataReductionCurve->dataReductionResult();
	if (!dataReductionResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", dataReductionResult.status) + "<br>";

	if (!dataReductionResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	SET_NUMBER_LOCALE
	if (dataReductionResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", numberLocale.toString(dataReductionResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", numberLocale.toString(dataReductionResult.elapsedTime)) + "<br>";

	str += "<br>";

	str += i18n("number of points: %1", numberLocale.toString(static_cast<qulonglong>(dataReductionResult.npoints))) + "<br>";
	str += i18n("positional squared error: %1", numberLocale.toString(dataReductionResult.posError)) + "<br>";
	str += i18n("area error: %1", numberLocale.toString(dataReductionResult.areaError)) + "<br>";

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYDataReductionCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void XYDataReductionCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	cbDataSourceCurve->setAspect(curve);
	m_initializing = false;
}

void XYDataReductionCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXDataColumn->setColumn(column, m_dataReductionCurve->xDataColumnPath());
	m_initializing = false;
}

void XYDataReductionCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXDataColumn->setColumn(column, m_dataReductionCurve->xDataColumnPath());
	m_initializing = false;
}

void XYDataReductionCurveDock::curveDataReductionDataChanged(const XYDataReductionCurve::DataReductionData& dataReductionData) {
	m_initializing = true;
	m_dataReductionData = dataReductionData;
	//uiGeneralTab.cbType->setCurrentIndex(m_dataReductionData.type);
	//this->typeChanged();

	this->showDataReductionResult();
	m_initializing = false;
}

void XYDataReductionCurveDock::dataChanged() {
	this->enableRecalculate();
}

void XYDataReductionCurveDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	uiGeneralTab.chkVisible->setChecked(on);
	m_initializing = false;
}
