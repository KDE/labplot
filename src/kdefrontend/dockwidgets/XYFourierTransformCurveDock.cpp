/***************************************************************************
    File             : XYFourierTransformCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of Fourier transform curves

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

#include "XYFourierTransformCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

/*!
  \class XYFourierTransformCurveDock
 \brief  Provides a widget for editing the properties of the XYFourierTransformCurves
		(2D-curves defined by a Fourier transform) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYFourierTransformCurveDock::XYFourierTransformCurveDock(QWidget *parent) : XYCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYFourierTransformCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_teComment = uiGeneralTab.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2,2,2,2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 5, 2, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 6, 2, 1, 2);

	for (int i = 0; i < NSL_SF_WINDOW_TYPE_COUNT; i++)
		uiGeneralTab.cbWindowType->addItem(i18n(nsl_sf_window_type_name[i]));
	for (int i = 0; i < NSL_DFT_RESULT_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_dft_result_type_name[i]));
	for (int i = 0; i < NSL_DFT_XSCALE_COUNT; i++)
		uiGeneralTab.cbXScale->addItem(i18n(nsl_dft_xscale_name[i]));

	uiGeneralTab.leMin->setValidator( new QDoubleValidator(uiGeneralTab.leMin) );
	uiGeneralTab.leMax->setValidator( new QDoubleValidator(uiGeneralTab.leMax) );

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYFourierTransformCurveDock::nameChanged);
	connect(uiGeneralTab.teComment, &QTextEdit::textChanged, this, &XYFourierTransformCurveDock::commentChanged);
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYFourierTransformCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYFourierTransformCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leMin, &QLineEdit::textChanged, this, &XYFourierTransformCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.leMax, &QLineEdit::textChanged, this, &XYFourierTransformCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.cbWindowType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierTransformCurveDock::windowTypeChanged);
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierTransformCurveDock::typeChanged);
	connect(uiGeneralTab.cbTwoSided, &QCheckBox::stateChanged, this, &XYFourierTransformCurveDock::twoSidedChanged);
	connect(uiGeneralTab.cbShifted, &QCheckBox::stateChanged, this, &XYFourierTransformCurveDock::shiftedChanged);
	connect(uiGeneralTab.cbXScale, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierTransformCurveDock::xScaleChanged);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFourierTransformCurveDock::plotRangeChanged );
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYFourierTransformCurveDock::recalculateClicked);

	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFourierTransformCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFourierTransformCurveDock::yDataColumnChanged);
}

void XYFourierTransformCurveDock::initGeneralTab() {
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
	cbXDataColumn->setColumn(m_transformCurve->xDataColumn(), m_transformCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_transformCurve->yDataColumn(), m_transformCurve->yDataColumnPath());
	uiGeneralTab.cbAutoRange->setChecked(m_transformData.autoRange);
	SET_NUMBER_LOCALE
	uiGeneralTab.leMin->setText( numberLocale.toString(m_transformData.xRange.first()) );
	uiGeneralTab.leMax->setText( numberLocale.toString(m_transformData.xRange.last()) );
	this->autoRangeChanged();

	uiGeneralTab.cbWindowType->setCurrentIndex(m_transformData.windowType);
	this->windowTypeChanged();
	uiGeneralTab.cbType->setCurrentIndex(m_transformData.type);
	this->typeChanged();
	uiGeneralTab.cbTwoSided->setChecked(m_transformData.twoSided);
	this->twoSidedChanged();	// show/hide shifted check box
	uiGeneralTab.cbShifted->setChecked(m_transformData.shifted);
	this->shiftedChanged();
	uiGeneralTab.cbXScale->setCurrentIndex(m_transformData.xScale);
	this->xScaleChanged();
	this->showTransformResult();

	//enable the "recalculate"-button if the source data was changed since the last transform
	uiGeneralTab.pbRecalculate->setEnabled(m_transformCurve->isSourceDataChangedSinceLastRecalc());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_transformCurve, &XYFourierTransformCurve::aspectDescriptionChanged, this, &XYFourierTransformCurveDock::curveDescriptionChanged);
	connect(m_transformCurve, &XYFourierTransformCurve::xDataColumnChanged, this, &XYFourierTransformCurveDock::curveXDataColumnChanged);
	connect(m_transformCurve, &XYFourierTransformCurve::yDataColumnChanged, this, &XYFourierTransformCurveDock::curveYDataColumnChanged);
	connect(m_transformCurve, &XYFourierTransformCurve::transformDataChanged, this, &XYFourierTransformCurveDock::curveTransformDataChanged);
	connect(m_transformCurve, &XYFourierTransformCurve::sourceDataChanged, this, &XYFourierTransformCurveDock::enableRecalculate);
	connect(m_transformCurve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &XYFourierTransformCurveDock::curveVisibilityChanged);
	connect(m_transformCurve, &WorksheetElement::plotRangeListChanged, this, &XYFourierTransformCurveDock::updatePlotRanges);
}

void XYFourierTransformCurveDock::setModel() {
	QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Datapicker,
	                       AspectType::DatapickerCurve, AspectType::Spreadsheet, AspectType::LiveDataSource,
	                       AspectType::Column, AspectType::Worksheet, AspectType::CartesianPlot,
	                       AspectType::XYFitCurve, AspectType::CantorWorksheet};
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);

	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYFourierTransformCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = m_curve;
	m_transformCurve = static_cast<XYFourierTransformCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_transformData = m_transformCurve->transformData();

	initGeneralTab();
	initTabs();
	m_initializing = false;

	updatePlotRanges();
}

void XYFourierTransformCurveDock::updatePlotRanges() const {
	updatePlotRangeList(uiGeneralTab.cbPlotRanges);
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYFourierTransformCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierTransformCurve*>(curve)->setXDataColumn(column);

	if (column) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			SET_NUMBER_LOCALE
			uiGeneralTab.leMin->setText( numberLocale.toString(column->minimum()) );
			uiGeneralTab.leMax->setText( numberLocale.toString(column->maximum()) );
		}
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYFourierTransformCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierTransformCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYFourierTransformCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_transformData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.leMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.leMax->setEnabled(false);
		m_transformCurve = dynamic_cast<XYFourierTransformCurve*>(m_curve);
		if (m_transformCurve->xDataColumn()) {
			SET_NUMBER_LOCALE
			uiGeneralTab.leMin->setText( numberLocale.toString(m_transformCurve->xDataColumn()->minimum()) );
			uiGeneralTab.leMax->setText( numberLocale.toString(m_transformCurve->xDataColumn()->maximum()) );
		}
	} else {
		uiGeneralTab.lMin->setEnabled(true);
		uiGeneralTab.leMin->setEnabled(true);
		uiGeneralTab.lMax->setEnabled(true);
		uiGeneralTab.leMax->setEnabled(true);
	}

}
void XYFourierTransformCurveDock::xRangeMinChanged() {
	SET_DOUBLE_FROM_LE_REC(m_transformData.xRange.first(), uiGeneralTab.leMin);
}

void XYFourierTransformCurveDock::xRangeMaxChanged() {
	SET_DOUBLE_FROM_LE_REC(m_transformData.xRange.last(), uiGeneralTab.leMax);
}

void XYFourierTransformCurveDock::windowTypeChanged() {
	auto windowType = (nsl_sf_window_type)uiGeneralTab.cbWindowType->currentIndex();
	m_transformData.windowType = windowType;

	enableRecalculate();
}

void XYFourierTransformCurveDock::typeChanged() {
	auto type = (nsl_dft_result_type)uiGeneralTab.cbType->currentIndex();
	m_transformData.type = type;

	enableRecalculate();
}

void XYFourierTransformCurveDock::twoSidedChanged() {
	bool twoSided = uiGeneralTab.cbTwoSided->isChecked();
	m_transformData.twoSided = twoSided;

	if (twoSided)
		uiGeneralTab.cbShifted->setEnabled(true);
	else {
		uiGeneralTab.cbShifted->setEnabled(false);
		uiGeneralTab.cbShifted->setChecked(false);
	}

	enableRecalculate();
}

void XYFourierTransformCurveDock::shiftedChanged() {
	bool shifted = uiGeneralTab.cbShifted->isChecked();
	m_transformData.shifted = shifted;

	enableRecalculate();
}

void XYFourierTransformCurveDock::xScaleChanged() {
	auto xScale = (nsl_dft_xscale)uiGeneralTab.cbXScale->currentIndex();
	m_transformData.xScale = xScale;

	enableRecalculate();
}

void XYFourierTransformCurveDock::recalculateClicked() {

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	for (auto* curve : m_curvesList)
		dynamic_cast<XYFourierTransformCurve*>(curve)->setTransformData(m_transformData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Fourier transformation status: %1", m_transformCurve->transformResult().status));
	QApplication::restoreOverrideCursor();
}

void XYFourierTransformCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no transforming possible without the x- and y-data
	AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
	bool data = (aspectX && aspectY);
	if (aspectX) {
		cbXDataColumn->useCurrentIndexText(true);
		cbXDataColumn->setInvalid(false);
	}
	if (aspectY) {
		cbYDataColumn->useCurrentIndexText(true);
		cbYDataColumn->setInvalid(false);
	}

	uiGeneralTab.pbRecalculate->setEnabled(data);
}

/*!
 * show the result and details of the transform
 */
void XYFourierTransformCurveDock::showTransformResult() {
	const XYFourierTransformCurve::TransformResult& transformResult = m_transformCurve->transformResult();
	if (!transformResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", transformResult.status) + "<br>";

	if (!transformResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	SET_NUMBER_LOCALE
	if (transformResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", numberLocale.toString(transformResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", numberLocale.toString(transformResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYFourierTransformCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.teComment->text())
		uiGeneralTab.teComment->setText(aspect->comment());
	m_initializing = false;
}

void XYFourierTransformCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXDataColumn->setColumn(column, m_transformCurve->xDataColumnPath());
	m_initializing = false;
}

void XYFourierTransformCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbYDataColumn->setColumn(column, m_transformCurve->yDataColumnPath());
	m_initializing = false;
}

void XYFourierTransformCurveDock::curveTransformDataChanged(const XYFourierTransformCurve::TransformData& transformData) {
	m_initializing = true;
	m_transformData = transformData;
	uiGeneralTab.cbType->setCurrentIndex(m_transformData.type);
	this->typeChanged();

	this->showTransformResult();
	m_initializing = false;
}

void XYFourierTransformCurveDock::dataChanged() {
	this->enableRecalculate();
}

void XYFourierTransformCurveDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	uiGeneralTab.chkVisible->setChecked(on);
	m_initializing = false;
}
