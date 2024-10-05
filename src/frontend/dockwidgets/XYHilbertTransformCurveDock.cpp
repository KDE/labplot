/*
	File             : XYHilbertTransformCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of Hilbert transform curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYHilbertTransformCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYHilbertTransformCurve.h"
#include "frontend/widgets/TreeViewComboBox.h"

/*!
  \class XYHilbertTransformCurveDock
 \brief  Provides a widget for editing the properties of the XYHilbertTransformCurves
		(2D-curves defined by a Hilbert transform) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/

XYHilbertTransformCurveDock::XYHilbertTransformCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYHilbertTransformCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment, uiGeneralTab.pbRecalculate);
	setVisibilityWidgets(uiGeneralTab.chkVisible, uiGeneralTab.chkLegendVisible);

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2, 2, 2, 2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 5, 2, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 6, 2, 1, 2);

	for (int i = 0; i < NSL_HILBERT_RESULT_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_hilbert_result_type_name[i]));

	uiGeneralTab.leMin->setValidator(new QDoubleValidator(uiGeneralTab.leMin));
	uiGeneralTab.leMax->setValidator(new QDoubleValidator(uiGeneralTab.leMax));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Slots
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYHilbertTransformCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYHilbertTransformCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYHilbertTransformCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYHilbertTransformCurveDock::typeChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYHilbertTransformCurveDock::recalculateClicked);

	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYHilbertTransformCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYHilbertTransformCurveDock::yDataColumnChanged);
}

void XYHilbertTransformCurveDock::initGeneralTab() {
	// show the properties of the first curve
	cbXDataColumn->setColumn(m_transformCurve->xDataColumn(), m_transformCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_transformCurve->yDataColumn(), m_transformCurve->yDataColumnPath());
	uiGeneralTab.cbAutoRange->setChecked(m_transformData.autoRange);

	const auto numberLocale = QLocale();
	uiGeneralTab.leMin->setText(numberLocale.toString(m_transformData.xRange.first()));
	uiGeneralTab.leMax->setText(numberLocale.toString(m_transformData.xRange.last()));
	this->autoRangeChanged();

	uiGeneralTab.cbType->setCurrentIndex(m_transformData.type);
	this->typeChanged();
	this->showTransformResult();

	// enable the "recalculate"-button if the source data was changed since the last transform
	uiGeneralTab.pbRecalculate->setEnabled(m_transformCurve->isSourceDataChangedSinceLastRecalc());

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_transformCurve, &XYHilbertTransformCurve::xDataColumnChanged, this, &XYHilbertTransformCurveDock::curveXDataColumnChanged);
	connect(m_transformCurve, &XYHilbertTransformCurve::yDataColumnChanged, this, &XYHilbertTransformCurveDock::curveYDataColumnChanged);
	connect(m_transformCurve, &XYHilbertTransformCurve::transformDataChanged, this, &XYHilbertTransformCurveDock::curveTransformDataChanged);
	connect(m_transformCurve, &XYHilbertTransformCurve::sourceDataChanged, this, &XYHilbertTransformCurveDock::enableRecalculate);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYHilbertTransformCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	setAnalysisCurves(list);
	m_transformCurve = static_cast<XYHilbertTransformCurve*>(m_curve);
	m_transformData = m_transformCurve->transformData();

	initGeneralTab();
	initTabs();
	setSymbols(list);

	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYHilbertTransformCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<AbstractColumn*>(index.internalPointer());

	for (auto* curve : m_curvesList)
		static_cast<XYHilbertTransformCurve*>(curve)->setXDataColumn(column);

	if (column && uiGeneralTab.cbAutoRange->isChecked()) {
		const auto numberLocale = QLocale();
		uiGeneralTab.leMin->setText(numberLocale.toString(column->minimum()));
		uiGeneralTab.leMax->setText(numberLocale.toString(column->maximum()));
	}

	enableRecalculate();
}

void XYHilbertTransformCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_transformData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.leMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.leMax->setEnabled(false);
		m_transformCurve = static_cast<XYHilbertTransformCurve*>(m_curve);
		if (m_transformCurve->xDataColumn()) {
			const auto numberLocale = QLocale();
			uiGeneralTab.leMin->setText(numberLocale.toString(m_transformCurve->xDataColumn()->minimum()));
			uiGeneralTab.leMax->setText(numberLocale.toString(m_transformCurve->xDataColumn()->maximum()));
		}
	} else {
		uiGeneralTab.lMin->setEnabled(true);
		uiGeneralTab.leMin->setEnabled(true);
		uiGeneralTab.lMax->setEnabled(true);
		uiGeneralTab.leMax->setEnabled(true);
	}
}
void XYHilbertTransformCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_transformData.xRange.first(), uiGeneralTab.leMin);
}

void XYHilbertTransformCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_transformData.xRange.last(), uiGeneralTab.leMax);
}

void XYHilbertTransformCurveDock::typeChanged() {
	auto type = (nsl_hilbert_result_type)uiGeneralTab.cbType->currentIndex();
	m_transformData.type = type;

	enableRecalculate();
}

void XYHilbertTransformCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (auto* curve : m_curvesList)
		static_cast<XYHilbertTransformCurve*>(curve)->setTransformData(m_transformData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	Q_EMIT info(i18n("Hilbert transformation status: %1", m_transformCurve->result().status));
	QApplication::restoreOverrideCursor();
}

/*!
 * show the result and details of the transform
 */
void XYHilbertTransformCurveDock::showTransformResult() {
	showResult(m_transformCurve, uiGeneralTab.teResult);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYHilbertTransformCurveDock::curveTransformDataChanged(const XYHilbertTransformCurve::TransformData& transformData) {
	CONDITIONAL_LOCK_RETURN;
	m_transformData = transformData;
	uiGeneralTab.cbType->setCurrentIndex(m_transformData.type);
	this->typeChanged();

	this->showTransformResult();
}
