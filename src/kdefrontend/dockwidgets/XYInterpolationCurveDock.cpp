/***************************************************************************
    File             : XYInterpolationCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright        : (C) 20016-2017 Alexander Semke (alexander.semke@web.de)
    Description      : widget for editing properties of interpolation curves

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

#include "XYInterpolationCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

extern "C" {
#include <gsl/gsl_interp.h>	// gsl_interp types
}
#include <cmath>        // isnan

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

XYInterpolationCurveDock::XYInterpolationCurveDock(QWidget* parent): XYCurveDock(parent) {
	//hide the line connection type
	ui.cbLineType->setDisabled(true);

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYInterpolationCurveDock::setupGeneral() {
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

	for (int i = 0; i < NSL_INTERP_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_interp_type_name[i]));
#if GSL_MAJOR_VERSION < 2
	// disable Steffen spline item
	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbType->model());
	QStandardItem* item = model->item(nsl_interp_type_steffen);
	item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
#endif
	for (int i = 0; i < NSL_INTERP_PCH_VARIANT_COUNT; i++)
		uiGeneralTab.cbVariant->addItem(i18n(nsl_interp_pch_variant_name[i]));
	for (int i = 0; i < NSL_INTERP_EVALUATE_COUNT; i++)
		uiGeneralTab.cbEval->addItem(i18n(nsl_interp_evaluate_name[i]));

	uiGeneralTab.cbPointsMode->addItem(i18n("Auto (5x data points)"));
	uiGeneralTab.cbPointsMode->addItem(i18n("Multiple of data points"));
	uiGeneralTab.cbPointsMode->addItem(i18n("Custom"));

	uiGeneralTab.sbMin->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	uiGeneralTab.sbMax->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYInterpolationCurveDock::nameChanged );
	connect( uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYInterpolationCurveDock::commentChanged );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( uiGeneralTab.cbDataSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataSourceTypeChanged(int)) );
	connect( uiGeneralTab.cbAutoRange, SIGNAL(clicked(bool)), this, SLOT(autoRangeChanged()) );
	connect( uiGeneralTab.sbMin, SIGNAL(valueChanged(double)), this, SLOT(xRangeMinChanged()) );
	connect( uiGeneralTab.sbMax, SIGNAL(valueChanged(double)), this, SLOT(xRangeMaxChanged()) );
	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()) );
	connect( uiGeneralTab.cbVariant, SIGNAL(currentIndexChanged(int)), this, SLOT(variantChanged()) );
	connect( uiGeneralTab.sbTension, SIGNAL(valueChanged(double)), this, SLOT(tensionChanged()) );
	connect( uiGeneralTab.sbContinuity, SIGNAL(valueChanged(double)), this, SLOT(continuityChanged()) );
	connect( uiGeneralTab.sbBias, SIGNAL(valueChanged(double)), this, SLOT(biasChanged()) );
	connect( uiGeneralTab.cbEval, SIGNAL(currentIndexChanged(int)), this, SLOT(evaluateChanged()) );
	connect( uiGeneralTab.sbPoints, SIGNAL(valueChanged(double)), this, SLOT(numberOfPointsChanged()) );
	connect( uiGeneralTab.cbPointsMode, SIGNAL(currentIndexChanged(int)), this, SLOT(pointsModeChanged()) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );

	connect( cbDataSourceCurve, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(dataSourceCurveChanged(QModelIndex)) );
	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)) );
	connect( cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)) );
}

void XYInterpolationCurveDock::initGeneralTab() {
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

	auto* analysisCurve = dynamic_cast<XYAnalysisCurve*>(m_curve);
	Q_ASSERT(analysisCurve);

	checkColumnAvailability(cbXDataColumn, analysisCurve->xDataColumn(), analysisCurve->xDataColumnPath());
	checkColumnAvailability(cbYDataColumn, analysisCurve->yDataColumn(), analysisCurve->yDataColumnPath());

	//show the properties of the first curve
	m_interpolationCurve = dynamic_cast<XYInterpolationCurve*>(m_curve);
	Q_ASSERT(m_interpolationCurve);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(m_interpolationCurve->dataSourceType());
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_interpolationCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_interpolationCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_interpolationCurve->yDataColumn());
	uiGeneralTab.cbAutoRange->setChecked(m_interpolationData.autoRange);
	uiGeneralTab.sbMin->setValue(m_interpolationData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_interpolationData.xRange.last());
	this->autoRangeChanged();
	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbType->setCurrentIndex(m_interpolationData.type);
	this->typeChanged();
	uiGeneralTab.cbVariant->setCurrentIndex(m_interpolationData.variant);
	this->variantChanged();
	uiGeneralTab.sbTension->setValue(m_interpolationData.tension);
	uiGeneralTab.sbContinuity->setValue(m_interpolationData.continuity);
	uiGeneralTab.sbBias->setValue(m_interpolationData.bias);
	uiGeneralTab.cbEval->setCurrentIndex(m_interpolationData.evaluate);

	if (m_interpolationData.pointsMode == XYInterpolationCurve::Multiple)
		uiGeneralTab.sbPoints->setValue(m_interpolationData.npoints/5.);
	else
		uiGeneralTab.sbPoints->setValue(m_interpolationData.npoints);
	uiGeneralTab.cbPointsMode->setCurrentIndex(m_interpolationData.pointsMode);

	this->showInterpolationResult();

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_interpolationCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_interpolationCurve, SIGNAL(dataSourceTypeChanged(XYAnalysisCurve::DataSourceType)), this, SLOT(curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType)));
	connect(m_interpolationCurve, SIGNAL(dataSourceCurveChanged(const XYCurve*)), this, SLOT(curveDataSourceCurveChanged(const XYCurve*)));
	connect(m_interpolationCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_interpolationCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_interpolationCurve, SIGNAL(interpolationDataChanged(XYInterpolationCurve::InterpolationData)), this, SLOT(curveInterpolationDataChanged(XYInterpolationCurve::InterpolationData)));
	connect(m_interpolationCurve, SIGNAL(sourceDataChanged()), this, SLOT(enableRecalculate()));
}

void XYInterpolationCurveDock::setModel() {
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
void XYInterpolationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_interpolationCurve = dynamic_cast<XYInterpolationCurve*>(m_curve);
	Q_ASSERT(m_interpolationCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_interpolationData = m_interpolationCurve->interpolationData();
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
void XYInterpolationCurveDock::dataSourceTypeChanged(int index) {
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

	for (XYCurve* curve: m_curvesList)
		dynamic_cast<XYInterpolationCurve*>(curve)->setDataSourceType(type);
}

void XYInterpolationCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	// disable types that need more data points
	if (dataSourceCurve)
		this->updateSettings(dataSourceCurve->xColumn());

	if (m_initializing)
		return;

	for (XYCurve* curve: m_curvesList)
		dynamic_cast<XYInterpolationCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYInterpolationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	this->updateSettings(column);

	if (m_initializing)
		return;

	for (XYCurve* curve: m_curvesList)
		dynamic_cast<XYInterpolationCurve*>(curve)->setXDataColumn(column);

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYInterpolationCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	// disable types that need more data points
	if (uiGeneralTab.cbAutoRange->isChecked()) {
		uiGeneralTab.sbMin->setValue(column->minimum());
		uiGeneralTab.sbMax->setValue(column->maximum());
	}

	unsigned int n = 0;
	for (int row = 0; row < column->rowCount(); row++)
		if (!std::isnan(column->valueAt(row)) && !column->isMasked(row))
			n++;
	dataPoints = n;
	if (m_interpolationData.pointsMode == XYInterpolationCurve::Auto)
		pointsModeChanged();

	const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbType->model());
	QStandardItem* item = model->item(nsl_interp_type_polynomial);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_polynomial) || dataPoints > 100) {	// not good for many points
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_polynomial)
			uiGeneralTab.cbType->setCurrentIndex(0);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

	item = model->item(nsl_interp_type_cspline);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_cspline)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_cspline)
			uiGeneralTab.cbType->setCurrentIndex(0);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

	item = model->item(nsl_interp_type_cspline_periodic);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_cspline_periodic)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_cspline_periodic)
			uiGeneralTab.cbType->setCurrentIndex(0);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

	item = model->item(nsl_interp_type_akima);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_akima)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_akima)
			uiGeneralTab.cbType->setCurrentIndex(0);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

	item = model->item(nsl_interp_type_akima_periodic);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_akima_periodic)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_akima_periodic)
			uiGeneralTab.cbType->setCurrentIndex(0);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

#if GSL_MAJOR_VERSION >= 2
	item = model->item(nsl_interp_type_steffen);
	if (dataPoints < gsl_interp_type_min_size(gsl_interp_steffen)) {
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
		if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_steffen)
			uiGeneralTab.cbType->setCurrentIndex(0);
	}
	else
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
#endif
	// own types work with 2 or more data points
}

void XYInterpolationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (XYCurve* curve: m_curvesList)
		dynamic_cast<XYInterpolationCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYInterpolationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_interpolationData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_interpolationCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
			xDataColumn = m_interpolationCurve->xDataColumn();
		else {
			if (m_interpolationCurve->dataSourceCurve())
				xDataColumn = m_interpolationCurve->dataSourceCurve()->xColumn();
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
void XYInterpolationCurveDock::xRangeMinChanged() {
	double xMin = uiGeneralTab.sbMin->value();

	m_interpolationData.xRange.first() = xMin;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::xRangeMaxChanged() {
	double xMax = uiGeneralTab.sbMax->value();

	m_interpolationData.xRange.last() = xMax;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::typeChanged() {
	const auto type = (nsl_interp_type)uiGeneralTab.cbType->currentIndex();
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

void XYInterpolationCurveDock::variantChanged() {
	const auto variant = (nsl_interp_pch_variant)uiGeneralTab.cbVariant->currentIndex();
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

void XYInterpolationCurveDock::tensionChanged() {
	m_interpolationData.tension = uiGeneralTab.sbTension->value();

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::continuityChanged() {
	m_interpolationData.continuity = uiGeneralTab.sbContinuity->value();

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::biasChanged() {
	m_interpolationData.bias = uiGeneralTab.sbBias->value();

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::evaluateChanged() {
	m_interpolationData.evaluate = (nsl_interp_evaluate)uiGeneralTab.cbEval->currentIndex();


	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::pointsModeChanged() {
	const auto mode = (XYInterpolationCurve::PointsMode)uiGeneralTab.cbPointsMode->currentIndex();

	switch (mode) {
	case XYInterpolationCurve::Auto:
		uiGeneralTab.sbPoints->setEnabled(false);
		uiGeneralTab.sbPoints->setDecimals(0);
		uiGeneralTab.sbPoints->setSingleStep(1.0);
		uiGeneralTab.sbPoints->setValue(5*dataPoints);
		break;
	case XYInterpolationCurve::Multiple:
		uiGeneralTab.sbPoints->setEnabled(true);
		if (m_interpolationData.pointsMode != XYInterpolationCurve::Multiple && dataPoints > 0) {
			uiGeneralTab.sbPoints->setDecimals(2);
			uiGeneralTab.sbPoints->setValue(uiGeneralTab.sbPoints->value()/(double)dataPoints);
			uiGeneralTab.sbPoints->setSingleStep(0.01);
		}
		break;
	case XYInterpolationCurve::Custom:
		uiGeneralTab.sbPoints->setEnabled(true);
		if (m_interpolationData.pointsMode == XYInterpolationCurve::Multiple) {
			uiGeneralTab.sbPoints->setDecimals(0);
			uiGeneralTab.sbPoints->setSingleStep(1.0);
			uiGeneralTab.sbPoints->setValue(uiGeneralTab.sbPoints->value()*dataPoints);
		}
		break;
	}

	m_interpolationData.pointsMode = mode;
}

void XYInterpolationCurveDock::numberOfPointsChanged() {
	if (uiGeneralTab.cbPointsMode->currentIndex() == XYInterpolationCurve::Multiple)
		m_interpolationData.npoints = uiGeneralTab.sbPoints->value()*dataPoints;
	else
		m_interpolationData.npoints = uiGeneralTab.sbPoints->value();

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

	for (XYCurve* curve: m_curvesList)
		dynamic_cast<XYInterpolationCurve*>(curve)->setInterpolationData(m_interpolationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Interpolation status: %1", m_interpolationCurve->interpolationResult().status));
	QApplication::restoreOverrideCursor();
}

void XYInterpolationCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no interpolation possible without the x- and y-data
	bool hasSourceData = false;
	if (m_interpolationCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet) {
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
		 hasSourceData = (m_interpolationCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the interpolation
 */
void XYInterpolationCurveDock::showInterpolationResult() {
	const XYInterpolationCurve::InterpolationResult& interpolationResult = m_interpolationCurve->interpolationResult();
	if (!interpolationResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", interpolationResult.status) + "<br>";

	if (!interpolationResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (interpolationResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s", QString::number(interpolationResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", QString::number(interpolationResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last interpolation
	uiGeneralTab.pbRecalculate->setEnabled(m_interpolationCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYInterpolationCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text()) {
		uiGeneralTab.leName->setText(aspect->name());
	} else if (aspect->comment() != uiGeneralTab.leComment->text()) {
		uiGeneralTab.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void XYInterpolationCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(type);
	m_initializing = false;
}

void XYInterpolationCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYInterpolationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	m_initializing = false;
}

void XYInterpolationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYInterpolationCurveDock::curveInterpolationDataChanged(const XYInterpolationCurve::InterpolationData& data) {
	m_initializing = true;
	m_interpolationData = data;
	uiGeneralTab.cbType->setCurrentIndex(m_interpolationData.type);
	this->typeChanged();

	this->showInterpolationResult();
	m_initializing = false;
}

void XYInterpolationCurveDock::dataChanged() {
	this->enableRecalculate();
}
