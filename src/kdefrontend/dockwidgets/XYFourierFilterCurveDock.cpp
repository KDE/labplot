/***************************************************************************
    File             : XYFourierFilterCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of Fourier filter curves

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

#include "XYFourierFilterCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <KMessageBox>

/*!
  \class XYFourierFilterCurveDock
 \brief  Provides a widget for editing the properties of the XYFourierFilterCurves
		(2D-curves defined by a Fourier filter) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYFourierFilterCurveDock::XYFourierFilterCurveDock(QWidget* parent) : XYCurveDock(parent),
	cbDataSourceCurve(nullptr),
	cbXDataColumn(nullptr),
	cbYDataColumn(nullptr),
	m_filterCurve(nullptr) {

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYFourierFilterCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);

	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2,2,2,2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 3);
	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 2);

	for(int i=0; i < NSL_FILTER_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_filter_type_name[i]));

	for(int i=0; i < NSL_FILTER_FORM_COUNT; i++)
		uiGeneralTab.cbForm->addItem(i18n(nsl_filter_form_name[i]));

	for(int i=0; i < NSL_FILTER_CUTOFF_UNIT_COUNT; i++) {
		uiGeneralTab.cbUnit->addItem(i18n(nsl_filter_cutoff_unit_name[i]));
		uiGeneralTab.cbUnit2->addItem(i18n(nsl_filter_cutoff_unit_name[i]));
	}

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYFourierFilterCurveDock::nameChanged );
	connect( uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYFourierFilterCurveDock::commentChanged );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( uiGeneralTab.cbDataSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataSourceTypeChanged(int)) );
	connect( uiGeneralTab.cbAutoRange, SIGNAL(clicked(bool)), this, SLOT(autoRangeChanged()) );
	connect( uiGeneralTab.sbMin, SIGNAL(valueChanged(double)), this, SLOT(xRangeMinChanged()) );
	connect( uiGeneralTab.sbMax, SIGNAL(valueChanged(double)), this, SLOT(xRangeMaxChanged()) );

	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()) );
	connect( uiGeneralTab.cbForm, SIGNAL(currentIndexChanged(int)), this, SLOT(formChanged()) );
	connect( uiGeneralTab.sbOrder, SIGNAL(valueChanged(int)), this, SLOT(orderChanged()) );
	connect( uiGeneralTab.sbCutoff, SIGNAL(valueChanged(double)), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.sbCutoff2, SIGNAL(valueChanged(double)), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.cbUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitChanged()) );
	connect( uiGeneralTab.cbUnit2, SIGNAL(currentIndexChanged(int)), this, SLOT(unit2Changed()) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );

	connect( cbDataSourceCurve, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(dataSourceCurveChanged(QModelIndex)) );
	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)) );
	connect( cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)) );
}

void XYFourierFilterCurveDock::initGeneralTab() {
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size()==1){
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.leComment->setEnabled(true);

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.leComment->setText(m_curve->comment());
	}else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.leComment->setEnabled(false);

		uiGeneralTab.leName->setText("");
		uiGeneralTab.leComment->setText("");
	}

	//show the properties of the first curve
	m_filterCurve = dynamic_cast<XYFourierFilterCurve*>(m_curve);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(m_filterCurve->dataSourceType());
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_filterCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_filterCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_filterCurve->yDataColumn());
	uiGeneralTab.cbAutoRange->setChecked(m_filterData.autoRange);
	uiGeneralTab.sbMin->setValue(m_filterData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_filterData.xRange.last());
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

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_filterCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_filterCurve, SIGNAL(dataSourceTypeChanged(XYAnalysisCurve::DataSourceType)), this, SLOT(curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType)));
	connect(m_filterCurve, SIGNAL(dataSourceCurveChanged(const XYCurve*)), this, SLOT(curveDataSourceCurveChanged(const XYCurve*)));
	connect(m_filterCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_filterCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_filterCurve, SIGNAL(filterDataChanged(XYFourierFilterCurve::FilterData)), this, SLOT(curveFilterDataChanged(XYFourierFilterCurve::FilterData)));
	connect(m_filterCurve, SIGNAL(sourceDataChanged()), this, SLOT(enableRecalculate()));
}

void XYFourierFilterCurveDock::setModel() {
	QList<const char*>  list;
	list<<"Folder"<<"Datapicker"<<"Worksheet"<<"CartesianPlot"<<"XYCurve";
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list.clear();
	list<<"Folder"<<"Workbook"<<"Datapicker"<<"DatapickerCurve"<<"Spreadsheet"
		<<"FileDataSource"<<"Column"<<"Worksheet"<<"CartesianPlot"<<"XYFitCurve";
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
void XYFourierFilterCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_filterCurve = dynamic_cast<XYFourierFilterCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_filterData = m_filterCurve->filterData();
	initGeneralTab();
	initTabs();
	m_initializing=false;
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYFourierFilterCurveDock::nameChanged(){
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYFourierFilterCurveDock::commentChanged(){
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYFourierFilterCurveDock::dataSourceTypeChanged(int index) {
	XYAnalysisCurve::DataSourceType type = (XYAnalysisCurve::DataSourceType)index;
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
		dynamic_cast<XYFourierFilterCurve*>(curve)->setDataSourceType(type);
}

void XYFourierFilterCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	XYCurve* dataSourceCurve = nullptr;
	if (aspect)
		dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	// update range of cutoff spin boxes (like a unit change)
	unitChanged();
	unit2Changed();

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYFourierFilterCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setXDataColumn(column);

	// update range of cutoff spin boxes (like a unit change)
	unitChanged();
	unit2Changed();

	if (column != nullptr) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			uiGeneralTab.sbMin->setValue(column->minimum());
			uiGeneralTab.sbMax->setValue(column->maximum());
		}
	}
}

void XYFourierFilterCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setYDataColumn(column);
}

void XYFourierFilterCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_filterData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);

		const AbstractColumn* xDataColumn = 0;
		if (m_filterCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
			xDataColumn = m_filterCurve->xDataColumn();
		else {
			if (m_filterCurve->dataSourceCurve())
				xDataColumn = m_filterCurve->dataSourceCurve()->xColumn();
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
void XYFourierFilterCurveDock::xRangeMinChanged() {
	double xMin = uiGeneralTab.sbMin->value();

	m_filterData.xRange.first() = xMin;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYFourierFilterCurveDock::xRangeMaxChanged() {
	double xMax = uiGeneralTab.sbMax->value();

	m_filterData.xRange.last() = xMax;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYFourierFilterCurveDock::typeChanged() {
	nsl_filter_type type = (nsl_filter_type)uiGeneralTab.cbType->currentIndex();
	m_filterData.type = type;

	switch (type) {
	case nsl_filter_type_low_pass:
	case nsl_filter_type_high_pass:
		uiGeneralTab.lCutoff->setText(i18n("Cutoff"));
		uiGeneralTab.lCutoff2->setVisible(false);
		uiGeneralTab.sbCutoff2->setVisible(false);
		uiGeneralTab.cbUnit2->setVisible(false);
		break;
	case nsl_filter_type_band_pass:
	case nsl_filter_type_band_reject:
		uiGeneralTab.lCutoff2->setVisible(true);
		uiGeneralTab.lCutoff->setText(i18n("Lower Cutoff"));
		uiGeneralTab.lCutoff2->setText(i18n("Upper Cutoff"));
		uiGeneralTab.sbCutoff2->setVisible(true);
		uiGeneralTab.cbUnit2->setVisible(true);
		break;
//TODO
/*	case nsl_filter_type_threshold:
		uiGeneralTab.lCutoff->setText(i18n("Value"));
		uiGeneralTab.lCutoff2->setVisible(false);
		uiGeneralTab.sbCutoff2->setVisible(false);
		uiGeneralTab.cbUnit2->setVisible(false);
*/
	}

	enableRecalculate();
}

void XYFourierFilterCurveDock::formChanged() {
	nsl_filter_form form = (nsl_filter_form)uiGeneralTab.cbForm->currentIndex();
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

void XYFourierFilterCurveDock::unitChanged() {
	nsl_filter_cutoff_unit unit = (nsl_filter_cutoff_unit)uiGeneralTab.cbUnit->currentIndex();
	nsl_filter_cutoff_unit oldUnit = m_filterData.unit;
	double oldValue = uiGeneralTab.sbCutoff->value();
	m_filterData.unit = unit;

	int n = 100;
	double f = 1.0; // sample frequency
	const AbstractColumn* xDataColumn = nullptr;
	if (m_filterCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
		xDataColumn = m_filterCurve->xDataColumn();
	else {
		if (m_filterCurve->dataSourceCurve())
			xDataColumn = m_filterCurve->dataSourceCurve()->xColumn();
	}

	if (xDataColumn != nullptr) {
		n = xDataColumn->rowCount();
		double range = xDataColumn->maximum() - xDataColumn->minimum();
		f = (n-1)/range/2.;
		DEBUG(" n =" << n << " sample frequency =" << f);
	}

	switch (unit) {
	case nsl_filter_cutoff_unit_frequency:
		uiGeneralTab.sbCutoff->setDecimals(6);
		uiGeneralTab.sbCutoff->setMaximum(f);
		uiGeneralTab.sbCutoff->setSingleStep(0.01*f);
		uiGeneralTab.sbCutoff->setSuffix(" Hz");
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			break;
		case nsl_filter_cutoff_unit_fraction:
			uiGeneralTab.sbCutoff->setValue(oldValue*f);
			break;
		case nsl_filter_cutoff_unit_index:
			uiGeneralTab.sbCutoff->setValue(oldValue*f/n);
			break;
		}
		break;
	case nsl_filter_cutoff_unit_fraction:
		uiGeneralTab.sbCutoff->setDecimals(6);
		uiGeneralTab.sbCutoff->setMaximum(1.0);
		uiGeneralTab.sbCutoff->setSingleStep(0.01);
		uiGeneralTab.sbCutoff->setSuffix("");
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			uiGeneralTab.sbCutoff->setValue(oldValue/f);
			break;
		case nsl_filter_cutoff_unit_fraction:
			break;
		case nsl_filter_cutoff_unit_index:
			uiGeneralTab.sbCutoff->setValue(oldValue/n);
			break;
		}
		break;
	case nsl_filter_cutoff_unit_index:
		uiGeneralTab.sbCutoff->setDecimals(0);
		uiGeneralTab.sbCutoff->setSingleStep(1);
		uiGeneralTab.sbCutoff->setMaximum(n);
		uiGeneralTab.sbCutoff->setSuffix("");
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			uiGeneralTab.sbCutoff->setValue(oldValue*n/f);
			break;
		case nsl_filter_cutoff_unit_fraction:
			uiGeneralTab.sbCutoff->setValue(oldValue*n);
			break;
		case nsl_filter_cutoff_unit_index:
			break;
		}
		break;
	}

	enableRecalculate();
}

void XYFourierFilterCurveDock::unit2Changed() {
	nsl_filter_cutoff_unit unit = (nsl_filter_cutoff_unit)uiGeneralTab.cbUnit2->currentIndex();
	nsl_filter_cutoff_unit oldUnit = m_filterData.unit2;
	double oldValue = uiGeneralTab.sbCutoff2->value();
	m_filterData.unit2 = unit;

	int n = 100;
	double f = 1.0; // sample frequency
	const AbstractColumn* xDataColumn = nullptr;
	if (m_filterCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
		xDataColumn = m_filterCurve->xDataColumn();
	else {
		if (m_filterCurve->dataSourceCurve())
			xDataColumn = m_filterCurve->dataSourceCurve()->xColumn();
	}

	if (xDataColumn != nullptr) {
		n = xDataColumn->rowCount();
		double range = xDataColumn->maximum() - xDataColumn->minimum();
		f = (n-1)/range/2.;
		DEBUG(" n =" << n << " sample frequency =" << f);
	}

	switch (unit) {
	case nsl_filter_cutoff_unit_frequency:
		uiGeneralTab.sbCutoff2->setDecimals(6);
		uiGeneralTab.sbCutoff2->setMaximum(f);
		uiGeneralTab.sbCutoff2->setSingleStep(0.01*f);
		uiGeneralTab.sbCutoff2->setSuffix(" Hz");
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			break;
		case nsl_filter_cutoff_unit_fraction:
			uiGeneralTab.sbCutoff2->setValue(oldValue*f);
			break;
		case nsl_filter_cutoff_unit_index:
			uiGeneralTab.sbCutoff2->setValue(oldValue*f/n);
			break;
		}
		break;
	case nsl_filter_cutoff_unit_fraction:
		uiGeneralTab.sbCutoff2->setDecimals(6);
		uiGeneralTab.sbCutoff2->setMaximum(1.0);
		uiGeneralTab.sbCutoff2->setSingleStep(0.01);
		uiGeneralTab.sbCutoff2->setSuffix("");
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			uiGeneralTab.sbCutoff2->setValue(oldValue/f);
			break;
		case nsl_filter_cutoff_unit_fraction:
			break;
		case nsl_filter_cutoff_unit_index:
			uiGeneralTab.sbCutoff2->setValue(oldValue/n);
			break;
		}
		break;
	case nsl_filter_cutoff_unit_index:
		uiGeneralTab.sbCutoff2->setDecimals(0);
		uiGeneralTab.sbCutoff2->setSingleStep(1);
		uiGeneralTab.sbCutoff2->setMaximum(n);
		uiGeneralTab.sbCutoff2->setSuffix("");
		switch (oldUnit) {
		case nsl_filter_cutoff_unit_frequency:
			uiGeneralTab.sbCutoff2->setValue(oldValue*n/f);
			break;
		case nsl_filter_cutoff_unit_fraction:
			uiGeneralTab.sbCutoff2->setValue(oldValue*n);
			break;
		case nsl_filter_cutoff_unit_index:
			break;
		}
		break;
	}

	enableRecalculate();
}

void XYFourierFilterCurveDock::recalculateClicked() {
	m_filterData.cutoff = uiGeneralTab.sbCutoff->value();
	m_filterData.cutoff2 = uiGeneralTab.sbCutoff2->value();

	if ((m_filterData.type == nsl_filter_type_band_pass || m_filterData.type == nsl_filter_type_band_reject)
			&& m_filterData.cutoff2 <= m_filterData.cutoff) {
		KMessageBox::sorry(this, i18n("The band width is <= 0 since lower cutoff value is not smaller than upper cutoff value. Please fix this."),
			                   i18n("band width <= 0") );
		return;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setFilterData(m_filterData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Fourier-Filter status: ") + m_filterCurve->filterResult().status);
	QApplication::restoreOverrideCursor();
}

void XYFourierFilterCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no filtering possible without the x- and y-data
	bool hasSourceData = false;
	if (m_filterCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet) {
		AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectX!=0 && aspectY!=0);
	} else {
		 hasSourceData = (m_filterCurve->dataSourceCurve() != NULL);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the filter
 */
void XYFourierFilterCurveDock::showFilterResult() {
	const XYFourierFilterCurve::FilterResult& filterResult = m_filterCurve->filterResult();
	if (!filterResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status:") + ' ' + filterResult.status + "<br>";

	if (!filterResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (filterResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s").arg(QString::number(filterResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms").arg(QString::number(filterResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last filter
	uiGeneralTab.pbRecalculate->setEnabled(m_filterCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYFourierFilterCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(type);
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveFilterDataChanged(const XYFourierFilterCurve::FilterData& filterData) {
	m_initializing = true;
	m_filterData = filterData;
	uiGeneralTab.cbType->setCurrentIndex(m_filterData.type);
	this->typeChanged();

	this->showFilterResult();
	m_initializing = false;
}

void XYFourierFilterCurveDock::dataChanged() {
	this->enableRecalculate();
}
