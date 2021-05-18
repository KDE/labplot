/***************************************************************************
    File             : XYFourierFilterCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include <KMessageBox>

#include <QMenu>
#include <QWidgetAction>

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

XYFourierFilterCurveDock::XYFourierFilterCurveDock(QWidget* parent) : XYCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYFourierFilterCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_teComment = uiGeneralTab.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2,2,2,2);
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

	uiGeneralTab.leMin->setValidator( new QDoubleValidator(uiGeneralTab.leMin) );
	uiGeneralTab.leMax->setValidator( new QDoubleValidator(uiGeneralTab.leMax) );

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYFourierFilterCurveDock::nameChanged );
	connect(uiGeneralTab.teComment, &QTextEdit::textChanged, this, &XYFourierFilterCurveDock::commentChanged );
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYFourierFilterCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYFourierFilterCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYFourierFilterCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYFourierFilterCurveDock::xRangeMaxChanged);

	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::typeChanged);
	connect(uiGeneralTab.cbForm, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::formChanged);
	connect(uiGeneralTab.sbOrder, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYFourierFilterCurveDock::orderChanged);
	connect(uiGeneralTab.sbCutoff, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYFourierFilterCurveDock::enableRecalculate);
	connect(uiGeneralTab.sbCutoff2, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYFourierFilterCurveDock::enableRecalculate);
	connect(uiGeneralTab.cbUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::unitChanged);
	connect(uiGeneralTab.cbUnit2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::unit2Changed);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYFourierFilterCurveDock::recalculateClicked);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierFilterCurveDock::plotRangeChanged );

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYFourierFilterCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFourierFilterCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFourierFilterCurveDock::yDataColumnChanged);
}

void XYFourierFilterCurveDock::initGeneralTab() {
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
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_filterCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_filterCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_filterCurve->xDataColumn(), m_filterCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_filterCurve->yDataColumn(), m_filterCurve->xDataColumnPath());
	uiGeneralTab.cbAutoRange->setChecked(m_filterData.autoRange);
	SET_NUMBER_LOCALE
	uiGeneralTab.leMin->setText( numberLocale.toString(m_filterData.xRange.first()) );
	uiGeneralTab.leMax->setText( numberLocale.toString(m_filterData.xRange.last()) );
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
	connect(m_filterCurve, &XYFourierFilterCurve::aspectDescriptionChanged, this, &XYFourierFilterCurveDock::curveDescriptionChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::dataSourceTypeChanged, this, &XYFourierFilterCurveDock::curveDataSourceTypeChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::dataSourceCurveChanged, this, &XYFourierFilterCurveDock::curveDataSourceCurveChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::xDataColumnChanged, this, &XYFourierFilterCurveDock::curveXDataColumnChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::yDataColumnChanged, this, &XYFourierFilterCurveDock::curveYDataColumnChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::filterDataChanged, this, &XYFourierFilterCurveDock::curveFilterDataChanged);
	connect(m_filterCurve, &XYFourierFilterCurve::sourceDataChanged, this, &XYFourierFilterCurveDock::enableRecalculate);
	connect(m_filterCurve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &XYFourierFilterCurveDock::curveVisibilityChanged);
	connect(m_filterCurve, &WorksheetElement::plotRangeListChanged, this, &XYFourierFilterCurveDock::updatePlotRanges);
}

void XYFourierFilterCurveDock::setModel() {
	QList<AspectType> list{AspectType::Folder, AspectType::Datapicker, AspectType::Worksheet,
							AspectType::CartesianPlot, AspectType::XYCurve, AspectType::XYAnalysisCurve};
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
void XYFourierFilterCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = m_curve;
	m_filterCurve = static_cast<XYFourierFilterCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_filterData = m_filterCurve->filterData();

	SET_NUMBER_LOCALE
	uiGeneralTab.sbCutoff->setLocale(numberLocale);
	uiGeneralTab.sbCutoff2->setLocale(numberLocale);

	initGeneralTab();
	initTabs();
	m_initializing = false;

	updatePlotRanges();
}

void XYFourierFilterCurveDock::updatePlotRanges() const {
	updatePlotRangeList(uiGeneralTab.cbPlotRanges);
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

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setDataSourceType(type);
}

void XYFourierFilterCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	XYCurve* dataSourceCurve{};
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

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setXDataColumn(column);

	// update range of cutoff spin boxes (like a unit change)
	unitChanged();
	unit2Changed();

	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		SET_NUMBER_LOCALE
		uiGeneralTab.leMin->setText( numberLocale.toString(column->minimum()) );
		uiGeneralTab.leMax->setText( numberLocale.toString(column->maximum()) );
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYFourierFilterCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYFourierFilterCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_filterData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.leMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.leMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_filterCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_filterCurve->xDataColumn();
		else {
			if (m_filterCurve->dataSourceCurve())
				xDataColumn = m_filterCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			SET_NUMBER_LOCALE
			uiGeneralTab.leMin->setText( numberLocale.toString(xDataColumn->minimum()) );
			uiGeneralTab.leMax->setText( numberLocale.toString(xDataColumn->maximum()) );
		}
	} else {
		uiGeneralTab.lMin->setEnabled(true);
		uiGeneralTab.leMin->setEnabled(true);
		uiGeneralTab.lMax->setEnabled(true);
		uiGeneralTab.leMax->setEnabled(true);
	}

}
void XYFourierFilterCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_filterData.xRange.first(), uiGeneralTab.leMin);
}

void XYFourierFilterCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_filterData.xRange.last(), uiGeneralTab.leMax);
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
//TODO
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

void XYFourierFilterCurveDock::unitChanged() {
	auto unit = (nsl_filter_cutoff_unit)uiGeneralTab.cbUnit->currentIndex();
	nsl_filter_cutoff_unit oldUnit = m_filterData.unit;
	double oldValue = uiGeneralTab.sbCutoff->value();
	m_filterData.unit = unit;

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
		uiGeneralTab.sbCutoff->setSuffix(QString());
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
		uiGeneralTab.sbCutoff->setSuffix(QString());
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
	auto unit = (nsl_filter_cutoff_unit)uiGeneralTab.cbUnit2->currentIndex();
	nsl_filter_cutoff_unit oldUnit = m_filterData.unit2;
	double oldValue = uiGeneralTab.sbCutoff2->value();
	m_filterData.unit2 = unit;

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
		uiGeneralTab.sbCutoff2->setSuffix(QString());
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
		uiGeneralTab.sbCutoff2->setSuffix(QString());
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
	emit info(i18n("Fourier-Filter status: %1", m_filterCurve->filterResult().status));
	QApplication::restoreOverrideCursor();
}

void XYFourierFilterCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no filtering possible without the x- and y-data
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
	const XYFourierFilterCurve::FilterResult& filterResult = m_filterCurve->filterResult();
	if (!filterResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", filterResult.status) + "<br>";

	if (!filterResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	SET_NUMBER_LOCALE
	if (filterResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", numberLocale.toString(filterResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", numberLocale.toString(filterResult.elapsedTime)) + "<br>";

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
	else if (aspect->comment() != uiGeneralTab.teComment->text())
		uiGeneralTab.teComment->setText(aspect->comment());
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	cbDataSourceCurve->setAspect(curve);
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXDataColumn->setColumn(column, m_filterCurve->xDataColumnPath());
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbYDataColumn->setColumn(column, m_filterCurve->yDataColumnPath());
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

void XYFourierFilterCurveDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	uiGeneralTab.chkVisible->setChecked(on);
	m_initializing = false;
}
