/***************************************************************************
    File             : XYDifferentiationCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

extern "C" {
#include "backend/nsl/nsl_diff.h"
}
#include <cmath>        // isnan

/*!
  \class XYDifferentiationCurveDock
 \brief  Provides a widget for editing the properties of the XYDifferentiationCurves
		(2D-curves defined by a differentiation) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYDifferentiationCurveDock::XYDifferentiationCurveDock(QWidget *parent): 
	XYCurveDock(parent), cbXDataColumn(0), cbYDataColumn(0), m_differentiationCurve(0) {

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

	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2,2,2,2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 4, 3, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 5, 3, 1, 2);

	for (int i=0; i < NSL_DIFF_DERIV_ORDER_COUNT; i++)
		uiGeneralTab.cbDerivOrder->addItem(i18n(nsl_diff_deriv_order_name[i]));

	uiGeneralTab.pbRecalculate->setIcon(KIcon("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( uiGeneralTab.cbDerivOrder, SIGNAL(currentIndexChanged(int)), this, SLOT(derivOrderChanged()) );
	connect( uiGeneralTab.sbAccOrder, SIGNAL(valueChanged(int)), this, SLOT(accOrderChanged()) );

	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

void XYDifferentiationCurveDock::initGeneralTab() {
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size()==1) {
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
	m_differentiationCurve = dynamic_cast<XYDifferentiationCurve*>(m_curve);
	Q_ASSERT(m_differentiationCurve);
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, m_differentiationCurve->xDataColumn());
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, m_differentiationCurve->yDataColumn());
	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbDerivOrder->setCurrentIndex(m_differentiationData.derivOrder);
	this->derivOrderChanged();
	uiGeneralTab.sbAccOrder->setValue(m_differentiationData.accOrder);
	this->accOrderChanged();

	this->showDifferentiationResult();

	//enable the "recalculate"-button if the source data was changed since the last differentiation
	uiGeneralTab.pbRecalculate->setEnabled(m_differentiationCurve->isSourceDataChangedSinceLastDifferentiation());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_differentiationCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_differentiationCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_differentiationCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_differentiationCurve, SIGNAL(differentiationDataChanged(XYDifferentiationCurve::DifferentiationData)), this, SLOT(curveDifferentiationDataChanged(XYDifferentiationCurve::DifferentiationData)));
	connect(m_differentiationCurve, SIGNAL(sourceDataChangedSinceLastDifferentiation()), this, SLOT(enableRecalculate()));
}

void XYDifferentiationCurveDock::setModel() {
	QList<const char*>  list;
	list<<"Folder"<<"Workbook"<<"Datapicker"<<"DatapickerCurve"<<"Spreadsheet"
		<<"FileDataSource"<<"Column"<<"Worksheet"<<"CartesianPlot"<<"XYFitCurve";
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);

 	list.clear();
	list<<"Column";
	cbXDataColumn->setSelectableClasses(list);
	cbYDataColumn->setSelectableClasses(list);

	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)) );
	connect( cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)) );

	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYDifferentiationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_differentiationCurve = dynamic_cast<XYDifferentiationCurve*>(m_curve);
	Q_ASSERT(m_differentiationCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_differentiationData = m_differentiationCurve->differentiationData();
	initGeneralTab();
	initTabs();
	m_initializing=false;

	//hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYDifferentiationCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYDifferentiationCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYDifferentiationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYDifferentiationCurve*>(curve)->setXDataColumn(column);

	// disable deriv orders and accuracies that need more data points
	if (column != 0) {
		size_t n=0;
		for (int row=0; row < column->rowCount(); row++)
			if (!std::isnan(column->valueAt(row)) && !column->isMasked(row))
				n++;


		const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbDerivOrder->model());
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
}

void XYDifferentiationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYDifferentiationCurve*>(curve)->setYDataColumn(column);
}

void XYDifferentiationCurveDock::derivOrderChanged() {
	const nsl_diff_deriv_order_type derivOrder = (nsl_diff_deriv_order_type)uiGeneralTab.cbDerivOrder->currentIndex();
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
	int accOrder = (int)uiGeneralTab.sbAccOrder->value();
	m_differentiationData.accOrder = accOrder;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYDifferentiationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYDifferentiationCurve*>(curve)->setDifferentiationData(m_differentiationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	QApplication::restoreOverrideCursor();
}

void XYDifferentiationCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no differentiationing possible without the x- and y-data
	AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
	bool data = (aspectX!=0 && aspectY!=0);

	uiGeneralTab.pbRecalculate->setEnabled(data);
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

	//const XYDifferentiationCurve::DifferentiationData& differentiationData = m_differentiationCurve->differentiationData();
	QString str = i18n("status:") + ' ' + differentiationResult.status + "<br>";

	if (!differentiationResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (differentiationResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s").arg(QString::number(differentiationResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms").arg(QString::number(differentiationResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYDifferentiationCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
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

void XYDifferentiationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, column);
	m_initializing = false;
}

void XYDifferentiationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, column);
	m_initializing = false;
}

void XYDifferentiationCurveDock::curveDifferentiationDataChanged(const XYDifferentiationCurve::DifferentiationData& data) {
	m_initializing = true;
	m_differentiationData = data;
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
