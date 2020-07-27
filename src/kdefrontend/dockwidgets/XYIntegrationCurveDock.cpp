/***************************************************************************
    File             : XYIntegrationCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of integration curves

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

#include "XYIntegrationCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

extern "C" {
#include "backend/nsl/nsl_int.h"
}

/*!
  \class XYIntegrationCurveDock
 \brief  Provides a widget for editing the properties of the XYIntegrationCurves
		(2D-curves defined by a integration) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYIntegrationCurveDock::XYIntegrationCurveDock(QWidget* parent) : XYCurveDock(parent) {
}

/*!
 * 	// Tab "General"
 */
void XYIntegrationCurveDock::setupGeneral() {
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

	for (int i = 0; i < NSL_INT_NETHOD_COUNT; i++)
		uiGeneralTab.cbMethod->addItem(i18n(nsl_int_method_name[i]));

	//TODO: use line edits
	uiGeneralTab.sbMin->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	uiGeneralTab.sbMax->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYIntegrationCurveDock::nameChanged );
	connect(uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYIntegrationCurveDock::commentChanged );
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYIntegrationCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYIntegrationCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::clicked, this, &XYIntegrationCurveDock::autoRangeChanged);
	connect(uiGeneralTab.sbMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYIntegrationCurveDock::xRangeMinChanged);
	connect(uiGeneralTab.sbMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYIntegrationCurveDock::xRangeMaxChanged);
	connect(uiGeneralTab.dateTimeEditMin, &QDateTimeEdit::dateTimeChanged, this, &XYIntegrationCurveDock::xRangeMinDateTimeChanged);
	connect(uiGeneralTab.dateTimeEditMax, &QDateTimeEdit::dateTimeChanged, this, &XYIntegrationCurveDock::xRangeMaxDateTimeChanged);
	connect(uiGeneralTab.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYIntegrationCurveDock::methodChanged);
	connect(uiGeneralTab.cbAbsolute, &QCheckBox::clicked, this, &XYIntegrationCurveDock::absoluteChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYIntegrationCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYIntegrationCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYIntegrationCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYIntegrationCurveDock::yDataColumnChanged);
}

void XYIntegrationCurveDock::initGeneralTab() {
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
	m_integrationCurve = static_cast<XYIntegrationCurve*>(m_curve);
	checkColumnAvailability(cbXDataColumn, m_integrationCurve->xDataColumn(), m_integrationCurve->xDataColumnPath());
	checkColumnAvailability(cbYDataColumn, m_integrationCurve->yDataColumn(), m_integrationCurve->yDataColumnPath());

	//data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_integrationCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_integrationCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_integrationCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_integrationCurve->yDataColumn());

	//range widgets
	const auto* plot = static_cast<const CartesianPlot*>(m_integrationCurve->parentAspect());
	m_dateTimeRange = (plot->xRangeFormat() != CartesianPlot::RangeFormat::Numeric);
	if (!m_dateTimeRange) {
		uiGeneralTab.sbMin->setValue(m_integrationData.xRange.first());
		uiGeneralTab.sbMax->setValue(m_integrationData.xRange.last());
	} else {
		uiGeneralTab.dateTimeEditMin->setDateTime( QDateTime::fromMSecsSinceEpoch(m_integrationData.xRange.first()) );
		uiGeneralTab.dateTimeEditMax->setDateTime( QDateTime::fromMSecsSinceEpoch(m_integrationData.xRange.last()) );
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
	uiGeneralTab.cbAutoRange->setChecked(m_integrationData.autoRange);
	this->autoRangeChanged();

	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbMethod->setCurrentIndex(m_integrationData.method);
	this->methodChanged(m_integrationData.method);
	uiGeneralTab.cbAbsolute->setChecked(m_integrationData.absolute);
	this->absoluteChanged();

	this->showIntegrationResult();

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_integrationCurve, &XYIntegrationCurve::aspectDescriptionChanged, this, &XYIntegrationCurveDock::curveDescriptionChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::dataSourceTypeChanged, this, &XYIntegrationCurveDock::curveDataSourceTypeChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::dataSourceCurveChanged, this, &XYIntegrationCurveDock::curveDataSourceCurveChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::xDataColumnChanged, this, &XYIntegrationCurveDock::curveXDataColumnChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::yDataColumnChanged, this, &XYIntegrationCurveDock::curveYDataColumnChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::integrationDataChanged, this, &XYIntegrationCurveDock::curveIntegrationDataChanged);
	connect(m_integrationCurve, &XYIntegrationCurve::sourceDataChanged, this, &XYIntegrationCurveDock::enableRecalculate);
	connect(m_integrationCurve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &XYIntegrationCurveDock::curveVisibilityChanged);
}

void XYIntegrationCurveDock::setModel() {
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
void XYIntegrationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = m_curve;
	m_integrationCurve = dynamic_cast<XYIntegrationCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_integrationData = m_integrationCurve->integrationData();

	SET_NUMBER_LOCALE
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
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************

void XYIntegrationCurveDock::dataSourceTypeChanged(int index) {
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
		dynamic_cast<XYIntegrationCurve*>(curve)->setDataSourceType(type);
}

void XYIntegrationCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	// disable integration orders and accuracies that need more data points
	this->updateSettings(dataSourceCurve->xColumn());

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYIntegrationCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYIntegrationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYIntegrationCurve*>(curve)->setXDataColumn(column);

	if (column != nullptr) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			uiGeneralTab.sbMin->setValue(column->minimum());
			uiGeneralTab.sbMax->setValue(column->maximum());
		}

		// disable integration methods that need more data points
		this->updateSettings(column);
	}

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

/*!
 * disable deriv orders and accuracies that need more data points
 */
void XYIntegrationCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	//TODO
// 	size_t n = 0;
// 	for (int row = 0; row < column->rowCount(); row++)
// 		if (!std::isnan(column->valueAt(row)) && !column->isMasked(row))
// 			n++;
}
void XYIntegrationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	cbYDataColumn->hidePopup();

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYIntegrationCurve*>(curve)->setYDataColumn(column);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYIntegrationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_integrationData.autoRange = autoRange;

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
		if (m_integrationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_integrationCurve->xDataColumn();
		else {
			if (m_integrationCurve->dataSourceCurve())
				xDataColumn = m_integrationCurve->dataSourceCurve()->xColumn();
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

void XYIntegrationCurveDock::xRangeMinChanged(double value) {
	m_integrationData.xRange.first() = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::xRangeMaxChanged(double value) {
	m_integrationData.xRange.last() = value;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::xRangeMinDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	m_integrationData.xRange.first() = dateTime.toMSecsSinceEpoch();
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::xRangeMaxDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	m_integrationData.xRange.last() = dateTime.toMSecsSinceEpoch();
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::methodChanged(int index) {
	const auto method = (nsl_int_method_type)index;
	m_integrationData.method = method;

	// update absolute option
	switch (method) {
	case nsl_int_method_rectangle:
	case nsl_int_method_trapezoid:
		uiGeneralTab.cbAbsolute->setEnabled(true);
		break;
	case nsl_int_method_simpson:
	case nsl_int_method_simpson_3_8:
		uiGeneralTab.cbAbsolute->setChecked(false);
		uiGeneralTab.cbAbsolute->setEnabled(false);
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::absoluteChanged() {
	bool absolute = uiGeneralTab.cbAbsolute->isChecked();
	m_integrationData.absolute = absolute;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (auto* curve : m_curvesList)
		dynamic_cast<XYIntegrationCurve*>(curve)->setIntegrationData(m_integrationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Integration status: %1", m_integrationCurve->integrationResult().status));
	QApplication::restoreOverrideCursor();
}

void XYIntegrationCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no integration possible without the x- and y-data
	bool hasSourceData = false;
	if (m_integrationCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
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
		 hasSourceData = (m_integrationCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the result and details of the integration
 */
void XYIntegrationCurveDock::showIntegrationResult() {
	const XYIntegrationCurve::IntegrationResult& integrationResult = m_integrationCurve->integrationResult();
	if (!integrationResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	QString str = i18n("status: %1", integrationResult.status) + "<br>";

	if (!integrationResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	SET_NUMBER_LOCALE
	if (integrationResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", numberLocale.toString(integrationResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms", numberLocale.toString(integrationResult.elapsedTime)) + "<br>";

	str += i18n("value: %1", numberLocale.toString(integrationResult.value)) + "<br>";
 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);

	//enable the "recalculate"-button if the source data was changed since the last integration
	uiGeneralTab.pbRecalculate->setEnabled(m_integrationCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYIntegrationCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYIntegrationCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void XYIntegrationCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYIntegrationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	m_initializing = false;
}

void XYIntegrationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYIntegrationCurveDock::curveIntegrationDataChanged(const XYIntegrationCurve::IntegrationData& integrationData) {
	m_initializing = true;
	m_integrationData = integrationData;
	uiGeneralTab.cbMethod->setCurrentIndex(m_integrationData.method);
	this->methodChanged(m_integrationData.method);
	uiGeneralTab.cbAbsolute->setChecked(m_integrationData.absolute);
	this->absoluteChanged();

	this->showIntegrationResult();
	m_initializing = false;
}

void XYIntegrationCurveDock::dataChanged() {
	this->enableRecalculate();
}

void XYIntegrationCurveDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	uiGeneralTab.chkVisible->setChecked(on);
	m_initializing = false;
}
