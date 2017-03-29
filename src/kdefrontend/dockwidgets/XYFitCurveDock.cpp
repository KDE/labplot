/***************************************************************************
    File             : XYFitCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014-2016 Alexander Semke (alexander.semke@web.de)
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of fit curves

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

#include "XYFitCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"
#include "kdefrontend/widgets/FitOptionsWidget.h"
#include "kdefrontend/widgets/FitParametersWidget.h"

#include <KStandardDirs>
#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>
#include <QFileInfo>
#include <cfloat>	// DBL_MAX

extern "C" {
#include "backend/nsl/nsl_sf_stats.h"
}

/*!
  \class XYFitCurveDock
  \brief  Provides a widget for editing the properties of the XYFitCurves
		(2D-curves defined by a fit model) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYFitCurveDock::XYFitCurveDock(QWidget *parent)
	 : XYCurveDock(parent), cbXDataColumn(0), cbYDataColumn(0), cbWeightsColumn(0), m_fitCurve(0) {

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYFitCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2,2,2,2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 4, 4, 1, 2);

	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 5, 4, 1, 2);

	cbWeightsColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbWeightsColumn, 6, 4, 1, 2);

	for(int i = 0; i < NSL_FIT_MODEL_CATEGORY_COUNT; i++)
		uiGeneralTab.cbCategory->addItem(nsl_fit_model_category_name[i]);

	//show the fit-model category for the currently selected default (first) fit-model category
	categoryChanged(uiGeneralTab.cbCategory->currentIndex());

	uiGeneralTab.teEquation->setMaximumHeight(uiGeneralTab.leName->sizeHint().height() * 2);

	//use white background in the preview label
	QPalette p;
	p.setColor(QPalette::Window, Qt::white);
	uiGeneralTab.lFuncPic->setAutoFillBackground(true);
	uiGeneralTab.lFuncPic->setPalette(p);

	uiGeneralTab.tbConstants->setIcon( QIcon::fromTheme("labplot-format-text-symbol") );
	uiGeneralTab.tbFunctions->setIcon( QIcon::fromTheme("preferences-desktop-font") );
	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( uiGeneralTab.cbAutoRange, SIGNAL(clicked(bool)), this, SLOT(autoRangeChanged()) );
	connect( uiGeneralTab.sbMin, SIGNAL(valueChanged(double)), this, SLOT(xRangeMinChanged()) );
	connect( uiGeneralTab.sbMax, SIGNAL(valueChanged(double)), this, SLOT(xRangeMaxChanged()) );

	connect( uiGeneralTab.cbCategory, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryChanged(int)) );
	connect( uiGeneralTab.cbModel, SIGNAL(currentIndexChanged(int)), this, SLOT(modelChanged(int)) );
	connect( uiGeneralTab.sbDegree, SIGNAL(valueChanged(int)), this, SLOT(updateModelEquation()) );
	connect( uiGeneralTab.teEquation, SIGNAL(expressionChanged()), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.tbConstants, SIGNAL(clicked()), this, SLOT(showConstants()) );
	connect( uiGeneralTab.tbFunctions, SIGNAL(clicked()), this, SLOT(showFunctions()) );
	connect( uiGeneralTab.pbParameters, SIGNAL(clicked()), this, SLOT(showParameters()) );
	connect( uiGeneralTab.pbOptions, SIGNAL(clicked()), this, SLOT(showOptions()) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

void XYFitCurveDock::initGeneralTab() {
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

		uiGeneralTab.leName->setText("");
		uiGeneralTab.leComment->setText("");
	}

	//show the properties of the first curve
	m_fitCurve = dynamic_cast<XYFitCurve*>(m_curve);
	Q_ASSERT(m_fitCurve);
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, m_fitCurve->xDataColumn());
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, m_fitCurve->yDataColumn());
	XYCurveDock::setModelIndexFromColumn(cbWeightsColumn, m_fitCurve->weightsColumn());
	uiGeneralTab.cbAutoRange->setChecked(m_fitData.autoRange);
	uiGeneralTab.sbMin->setValue(m_fitData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_fitData.xRange.last());
	this->autoRangeChanged();

	unsigned int tmpModelType = m_fitData.modelType;	// save type because it's reset when category changes
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		uiGeneralTab.cbCategory->setCurrentIndex(uiGeneralTab.cbCategory->count() - 1);
	else
		uiGeneralTab.cbCategory->setCurrentIndex(m_fitData.modelCategory);
	m_fitData.modelType = tmpModelType;
	if (m_fitData.modelCategory != nsl_fit_model_custom)
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.sbDegree->setValue(m_fitData.degree);
	this->showFitResult();

	//enable the "recalculate"-button if the source data was changed since the last fit
	uiGeneralTab.pbRecalculate->setEnabled(m_fitCurve->isSourceDataChangedSinceLastFit());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_fitCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_fitCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(weightsColumnChanged(const AbstractColumn*)), this, SLOT(curveWeightsColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(fitDataChanged(XYFitCurve::FitData)), this, SLOT(curveFitDataChanged(XYFitCurve::FitData)));
	connect(m_fitCurve, SIGNAL(sourceDataChangedSinceLastFit()), this, SLOT(enableRecalculate()));
}

void XYFitCurveDock::setModel() {
	QList<const char*>  list;
	list << "Folder" << "Workbook" << "Spreadsheet" << "FileDataSource" << "Column" << "CantorWorksheet" << "Datapicker";
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);
	cbWeightsColumn->setTopLevelClasses(list);

	list.clear();
	list << "Column";
	cbXDataColumn->setSelectableClasses(list);
	cbYDataColumn->setSelectableClasses(list);
	cbWeightsColumn->setSelectableClasses(list);

	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);
	cbWeightsColumn->setModel(m_aspectTreeModel);

	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)) );
	connect( cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)) );
	connect( cbWeightsColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(weightsColumnChanged(QModelIndex)) );
	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYFitCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_fitCurve = dynamic_cast<XYFitCurve*>(m_curve);
	Q_ASSERT(m_fitCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_fitData = m_fitCurve->fitData();
	initGeneralTab();
	initTabs();
	m_initializing = false;
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYFitCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYFitCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYFitCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach (XYCurve* curve, m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setXDataColumn(column);

	if (column != 0) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			uiGeneralTab.sbMin->setValue(column->minimum());
			uiGeneralTab.sbMax->setValue(column->maximum());
		}
	}
}

void XYFitCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach (XYCurve* curve, m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setYDataColumn(column);
}

void XYFitCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_fitData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);
		m_fitCurve = dynamic_cast<XYFitCurve*>(m_curve);
		Q_ASSERT(m_fitCurve);
		if (m_fitCurve->xDataColumn()) {
			uiGeneralTab.sbMin->setValue(m_fitCurve->xDataColumn()->minimum());
			uiGeneralTab.sbMax->setValue(m_fitCurve->xDataColumn()->maximum());
		}
	} else {
		uiGeneralTab.lMin->setEnabled(true);
		uiGeneralTab.sbMin->setEnabled(true);
		uiGeneralTab.lMax->setEnabled(true);
		uiGeneralTab.sbMax->setEnabled(true);
	}

}
void XYFitCurveDock::xRangeMinChanged() {
	double xMin = uiGeneralTab.sbMin->value();

	m_fitData.xRange.first() = xMin;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYFitCurveDock::xRangeMaxChanged() {
	double xMax = uiGeneralTab.sbMax->value();

	m_fitData.xRange.last() = xMax;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYFitCurveDock::weightsColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach (XYCurve* curve, m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setWeightsColumn(column);
}

void XYFitCurveDock::categoryChanged(int index) {
	QDEBUG("categoryChanged() category =" << nsl_fit_model_category_name[index] << ", type =" << m_fitData.modelType);
	if (uiGeneralTab.cbCategory->currentIndex() == uiGeneralTab.cbCategory->count() - 1)
		m_fitData.modelCategory = nsl_fit_model_custom;
	else
		m_fitData.modelCategory = (nsl_fit_model_category)index;
	m_initializing = true;
	uiGeneralTab.cbModel->clear();

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		for(int i = 0; i < NSL_FIT_MODEL_BASIC_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_basic_name[i]);
		break;
	case nsl_fit_model_peak:
		for(int i = 0; i < NSL_FIT_MODEL_PEAK_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_peak_name[i]);
		break;
	case nsl_fit_model_growth:
		for(int i = 0; i < NSL_FIT_MODEL_GROWTH_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_growth_name[i]);
		break;
	case nsl_fit_model_distribution: {
		for(int i = 0; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_sf_stats_distribution_name[i]);

		// non-used items are disabled here
        	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbModel->model());

		for(int i = 1; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++) {
			//TODO: implement following distribution models
			if (i == nsl_sf_stats_gaussian_tail || i == nsl_sf_stats_exponential || i == nsl_sf_stats_exponential_power ||
				i == nsl_sf_stats_rayleigh_tail || i == nsl_sf_stats_landau || i == nsl_sf_stats_levy_alpha_stable ||
				i == nsl_sf_stats_levy_skew_alpha_stable || i == nsl_sf_stats_flat || i == nsl_sf_stats_fdist ||
				i == nsl_sf_stats_tdist || i == nsl_sf_stats_beta || i == nsl_sf_stats_gumbel2 || i == nsl_sf_stats_bernoulli ||
				i == nsl_sf_stats_binomial || i == nsl_sf_stats_negative_bionomial || i == nsl_sf_stats_pascal || i == nsl_sf_stats_geometric
				|| i == nsl_sf_stats_hypergeometric || i ==  nsl_sf_stats_logarithmic || i == nsl_sf_stats_pareto) {
					QStandardItem* item = model->item(i);
					item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			}
		}
		break;
	}
	case nsl_fit_model_custom:
		uiGeneralTab.cbModel->addItem(i18n("Custom"));
	}

	m_fitData.modelType = 0;
	uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	m_initializing = false;

	//show the fit-model for the currently selected default (first) fit-model
	modelChanged(m_fitData.modelType);
}

void XYFitCurveDock::modelChanged(int index) {
	QDEBUG("modelChanged() type =" << index << ", initializing =" << m_initializing);
	// leave if there is no selection
	if(index == -1)
		return;

	unsigned int type = 0;
	bool custom = false;
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		custom = true;
	else
		type = (unsigned int)index;
	m_fitData.modelType = type;
	uiGeneralTab.teEquation->setReadOnly(!custom);
	uiGeneralTab.tbFunctions->setVisible(custom);
	uiGeneralTab.tbConstants->setVisible(custom);

	// default settings
	uiGeneralTab.lDegree->setText(i18n("Degree"));

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		switch (type) {
		case nsl_fit_model_polynomial:
		case nsl_fit_model_fourier:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(9);
			uiGeneralTab.sbDegree->setValue(1);
			break;
		case nsl_fit_model_power:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(2);
			uiGeneralTab.sbDegree->setValue(1);
			break;
		case nsl_fit_model_exponential:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(3);
			uiGeneralTab.sbDegree->setValue(1);
			break;
		default:
			uiGeneralTab.lDegree->setVisible(false);
			uiGeneralTab.sbDegree->setVisible(false);
		}
		break;
	case nsl_fit_model_peak:	// all models support multiple peaks
		uiGeneralTab.lDegree->setText(i18n("Number of peaks"));
		uiGeneralTab.lDegree->setVisible(true);
		uiGeneralTab.sbDegree->setVisible(true);
		uiGeneralTab.sbDegree->setMaximum(9);
		uiGeneralTab.sbDegree->setValue(1);
		break;
	case nsl_fit_model_growth:
	case nsl_fit_model_distribution:
	case nsl_fit_model_custom:
		uiGeneralTab.lDegree->setVisible(false);
		uiGeneralTab.sbDegree->setVisible(false);
	}

	this->updateModelEquation();
}

void XYFitCurveDock::updateModelEquation() {
	DEBUG("updateModelEquation() type =" << m_fitData.modelType);

	int num = uiGeneralTab.sbDegree->value();
	QStringList vars; // variables/parameter that are known in ExpressionTextEdit teEquation
	vars << "x";
	// indices used in multi peak parameter models
	QStringList indices;
	indices << QString::fromUtf8("\u2081") << QString::fromUtf8("\u2082") << QString::fromUtf8("\u2083") << QString::fromUtf8("\u2084") << QString::fromUtf8("\u2085")
		<< QString::fromUtf8("\u2086") << QString::fromUtf8("\u2087") << QString::fromUtf8("\u2088") << QString::fromUtf8("\u2089");

	switch(m_fitData.modelCategory) {
        case nsl_fit_model_basic:
		m_fitData.model = nsl_fit_model_basic_equation[m_fitData.modelType];
		break;
        case nsl_fit_model_peak:
		m_fitData.model = nsl_fit_model_peak_equation[m_fitData.modelType];
		break;
        case nsl_fit_model_growth:
		m_fitData.model = nsl_fit_model_growth_equation[m_fitData.modelType];
		break;
        case nsl_fit_model_distribution:
		m_fitData.model = nsl_sf_stats_distribution_equation[m_fitData.modelType];
		break;
        case nsl_fit_model_custom:
		// use the equation of the last selected predefined model
		uiGeneralTab.teEquation->setText(m_fitData.model);
		break;
	}
	// custom keeps the parameter from previous selected model
	if (m_fitData.modelCategory != nsl_fit_model_custom) {
		m_fitData.paramNames.clear();
		m_fitData.paramNamesUtf8.clear();
	}

	switch(m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		switch (m_fitData.modelType) {
		case nsl_fit_model_polynomial:
			m_fitData.paramNames << "c0" << "c1";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("c\u2080") << QString::fromUtf8("c\u2081");
			if (num == 2) {
				m_fitData.model += " + c2*x^2";
				m_fitData.paramNames << "c2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("c\u2082");
			} else if (num > 2) {
				QString numStr = QString::number(num);
				for (int i = 2; i <= num; ++i) {
					numStr = QString::number(i);
					m_fitData.model += "+c" + numStr + "*x^" + numStr;
					m_fitData.paramNames << "c" + numStr;
					m_fitData.paramNamesUtf8 << "c" + indices[i-1];
				}
			}
			break;
		case nsl_fit_model_power:
			if (num == 1)
				m_fitData.paramNames << "a" << "b";
			else {
				m_fitData.paramNames << "a" << "b" << "c";
				m_fitData.model = "a + b*x^c";
			}
			break;
		case nsl_fit_model_exponential:
			switch (num) {
			case 1:
				m_fitData.paramNames << "a" << "b";
				break;
			case 2:
				m_fitData.model = "a1*exp(b1*x) + a2*exp(b2*x)";
				m_fitData.paramNames << "a1" << "b1" << "a2" << "b2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("a\u2081") << QString::fromUtf8("b\u2081")
						<< QString::fromUtf8("a\u2082") << QString::fromUtf8("b\u2082");
				break;
			case 3:
				m_fitData.model = "a1*exp(b1*x) + a2*exp(b2*x) + a3*exp(b3*x)";
				m_fitData.paramNames << "a1" << "b1" << "a2" << "b2" << "a3" << "b3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("a\u2081") << QString::fromUtf8("b\u2081")
						<< QString::fromUtf8("a\u2082") << QString::fromUtf8("b\u2082")
						<< QString::fromUtf8("a\u2083") << QString::fromUtf8("b\u2083");
				break;
			//TODO: up to 9 exponentials
			}
			break;
		case nsl_fit_model_inverse_exponential:
			num = 1;
			m_fitData.paramNames << "a" << "b" << "c";
			break;
		case nsl_fit_model_fourier:
			m_fitData.paramNames << "w" << "a0" << "a1" << "b1";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c9") << QString::fromUtf8("a\u2080")
				<< QString::fromUtf8("a\u2081") << QString::fromUtf8("b\u2081");
			if (num > 1) {
				for (int i = 1; i <= num; ++i) {
					QString numStr = QString::number(i);
					m_fitData.model += "+ (a" + numStr + "*cos(" + numStr + "*w*x) + b" + numStr + "*sin(" + numStr + "*w*x))";
					m_fitData.paramNames << "a"+numStr << "b"+numStr;
					m_fitData.paramNamesUtf8 << "a" + indices[i-1] << "b" + indices[i-1];
				}
			}
			break;
		}
		break;
	case nsl_fit_model_peak:
		switch ((nsl_fit_model_type_peak)m_fitData.modelType) {
		case nsl_fit_model_gaussian:
			switch (num) {
			case 1:
				m_fitData.paramNames << "s" << "mu" << "a";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				m_fitData.model = "1./sqrt(2*pi) * (a1/s1 * exp(-((x-mu1)/s1)^2/2) + a2/s2 * exp(-((x-mu2)/s2)^2/2))";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081") 
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				m_fitData.model = "1./sqrt(2*pi) * (a1/s1 * exp(-((x-mu1)/s1)^2/2) + a2/s2 * exp(-((x-mu2)/s2)^2/2) + a3/s3 * exp(-((x-mu3)/s3)^2/2))";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081") 
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				m_fitData.model = "1./sqrt(2*pi) * (";
				for (int i = 1; i <= num; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						m_fitData.model += " + ";
					m_fitData.model += "a" + numStr + "/s" + numStr + "* exp(-((x-mu" + numStr + ")/s" + numStr + ")^2/2)";
					m_fitData.paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				m_fitData.model += ")";
			}
			break;
		case nsl_fit_model_lorentz:
			switch (num) {
			case 1:
				m_fitData.paramNames << "g" << "mu" << "a";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				m_fitData.model = "1./pi * (a1 * g1/(g1^2+(x-mu1)^2) + a2 * g2/(g2^2+(x-mu2)^2))";
				m_fitData.paramNames << "g1" << "mu1" << "a1" << "g2" << "mu2" << "a2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03b3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				m_fitData.model = "1./pi * (a1 * g1/(g1^2+(x-mu1)^2) + a2 * g2/(g2^2+(x-mu2)^2) + a3 * g3/(g3^2+(x-mu3)^2))";
				m_fitData.paramNames << "g1" << "mu1" << "a1" << "g2" << "mu2" << "a2" << "g3" << "mu3" << "a3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03b3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03b3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				QString numStr = QString::number(num);
				m_fitData.model = "1./pi * (";
				for (int i = 1; i <= num; ++i) {
					numStr = QString::number(i);
					if (i > 1)
						m_fitData.model += " + ";
					m_fitData.model += "a" + numStr + " * g" + numStr + "/(g" + numStr + "^2+(x-mu" + numStr + ")^2)";
					m_fitData.paramNames << "g" + numStr << "mu" + numStr << "a" + numStr;
					m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				m_fitData.model += ")";
			}
			break;
		case nsl_fit_model_sech:
			switch (num) {
			case 1:
				m_fitData.paramNames << "s" << "mu" << "a";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				m_fitData.model = "1/pi * (a1/s1 * sech((x-mu1)/s1) + a2/s2 * sech((x-mu2)/s2))";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081") 
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				m_fitData.model = "1/pi * (a1/s1 * sech((x-mu1)/s1) + a2/s2 * sech((x-mu2)/s2) + a3/s3 * sech((x-mu3)/s3))";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081") 
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				QString numStr = QString::number(num);
				m_fitData.model = "1/pi * (";
				for (int i = 1; i <= num; ++i) {
					numStr = QString::number(i);
					if (i > 1)
						m_fitData.model += " + ";
					m_fitData.model += "a" + numStr + "/s" + numStr + "* sech((x-mu" + numStr + ")/s" + numStr + ")";
					m_fitData.paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				m_fitData.model += ")";
			}
			break;
		case nsl_fit_model_logistic:
			switch (num) {
			case 1:
				m_fitData.paramNames << "s" << "mu" << "a";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				m_fitData.model = "1/4 * (a1/s1 * sech((x-mu1)/2/s1)**2 + a2/s2 * sech((x-mu2)/2/s2)**2)";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081") 
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				m_fitData.model = "1/4 * (a1/s1 * sech((x-mu1)/2/s1)**2 + a2/s2 * sech((x-mu2)/2/s2)**2 + a3/s3 * sech((x-mu3)/2/s3)**2)";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081") 
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				QString numStr = QString::number(num);
				m_fitData.model = "1/4 * (";
				for (int i = 1; i <= num; ++i) {
					numStr = QString::number(i);
					if (i > 1)
						m_fitData.model += " + ";
					m_fitData.model += "a" + numStr + "/s" + numStr + "* sech((x-mu" + numStr + ")/2/s" + numStr + ")**2";
					m_fitData.paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				m_fitData.model += ")";
			}
			break;
		}
		break;
	case nsl_fit_model_growth:
		switch ((nsl_fit_model_type_growth)m_fitData.modelType) {
		case nsl_fit_model_atan:
		case nsl_fit_model_tanh:
		case nsl_fit_model_algebraic_sigmoid:
		case nsl_fit_model_erf:
		case nsl_fit_model_gudermann:
			m_fitData.paramNames << "s" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_fit_model_sigmoid:
			m_fitData.paramNames << "k" << "mu" << "a";
			m_fitData.paramNamesUtf8 << "k" << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_fit_model_hill:
			m_fitData.paramNames << "s" << "n" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << "n" << "A";
			break;
		case nsl_fit_model_gompertz:
			m_fitData.paramNames << "a" << "b" << "c";
			break;
		}
		break;
	case nsl_fit_model_distribution:
		switch ((nsl_sf_stats_distribution)m_fitData.modelType) {
		// TODO: add missing GSL distributions
		case nsl_sf_stats_gaussian:
		case nsl_sf_stats_laplace:
		case nsl_sf_stats_lognormal:
		case nsl_sf_stats_logistic:
		case nsl_sf_stats_sech:
			m_fitData.paramNames << "s" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gaussian_tail:
		case nsl_sf_stats_exponential:
		case nsl_sf_stats_exponential_power:
			break;
		case nsl_sf_stats_cauchy_lorentz:
		case nsl_sf_stats_levy:
			m_fitData.paramNames << "g" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_rayleigh:
			m_fitData.paramNames << "s" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << "A";
			break;
		case nsl_sf_stats_rayleigh_tail:
		case nsl_sf_stats_landau:
		case nsl_sf_stats_levy_alpha_stable:
		case nsl_sf_stats_levy_skew_alpha_stable:
			break;
		case nsl_sf_stats_gamma:
			m_fitData.paramNames << "t" << "k" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b8") << "k" << "A";
			break;
		case nsl_sf_stats_flat:
			break;
		case nsl_sf_stats_chi_squared:
			m_fitData.paramNames << "n" << "a";
			m_fitData.paramNamesUtf8 << "n" << "A";
			break;
		case nsl_sf_stats_fdist:
		case nsl_sf_stats_tdist:
		case nsl_sf_stats_beta:
		case nsl_sf_stats_pareto:
			break;
		case nsl_sf_stats_weibull:
			m_fitData.paramNames << "k" << "l" << "mu" << "a";
			m_fitData.paramNamesUtf8 << "k" << QString::fromUtf8("\u03bb") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gumbel1:
			m_fitData.paramNames << "s" << "b" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03b2") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gumbel2:
			break;
		case nsl_sf_stats_poisson:
			m_fitData.paramNames << "l" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03bb") << "A";
			break;
		case nsl_sf_stats_bernoulli:
		case nsl_sf_stats_binomial:
		case nsl_sf_stats_negative_bionomial:
		case nsl_sf_stats_pascal:
		case nsl_sf_stats_geometric:
		case nsl_sf_stats_hypergeometric:
		case nsl_sf_stats_logarithmic:
			break;
		case nsl_sf_stats_maxwell_boltzmann:
			m_fitData.paramNames << "s" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << "A";
			break;
		case nsl_sf_stats_frechet:
			m_fitData.paramNames << "g" << "mu" << "s" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << QString::fromUtf8("\u03c3") << "A";
			break;
		}
		break;
	case nsl_fit_model_custom:
		break;
	}
	vars << m_fitData.paramNames;

	// use normal param names if no utf8 param names are defined
	if (m_fitData.paramNamesUtf8.isEmpty()) 
		m_fitData.paramNamesUtf8 << m_fitData.paramNames;

	//resize the vector for the start values and set the elements to 1.0
	//in case a custom model is used, do nothing, we take over the previous values
	//when initializing, don't do anything - we use start values already
	//available - unless there're no values available
	if (m_fitData.modelCategory != nsl_fit_model_custom || 
	        !(m_initializing && m_fitData.paramNames.size() == m_fitData.paramStartValues.size())) {
		QDEBUG(" number of start values" << m_fitData.paramNames.size() << m_fitData.paramStartValues.size());
		m_fitData.paramStartValues.resize(m_fitData.paramNames.size());
		m_fitData.paramFixed.resize(m_fitData.paramNames.size());
		m_fitData.paramLowerLimits.resize(m_fitData.paramNames.size());
		m_fitData.paramUpperLimits.resize(m_fitData.paramNames.size());

		for (int i = 0; i < m_fitData.paramNames.size(); ++i) {
			m_fitData.paramStartValues[i] = 1.0;
			m_fitData.paramFixed[i] = false;
			m_fitData.paramLowerLimits[i] = -DBL_MAX;
			m_fitData.paramUpperLimits[i] = DBL_MAX;
		}

		// model-dependent start values
		if (m_fitData.modelCategory == nsl_fit_model_distribution) {
			if ((nsl_sf_stats_distribution)m_fitData.modelType == nsl_sf_stats_weibull)
				m_fitData.paramStartValues[2] = 0.0;
			if ((nsl_sf_stats_distribution)m_fitData.modelType == nsl_sf_stats_frechet || (nsl_sf_stats_distribution)m_fitData.modelType == nsl_sf_stats_levy)
				m_fitData.paramStartValues[1] = 0.0;
		}
	}

	uiGeneralTab.teEquation->setVariables(vars);

	// set formula picture
	uiGeneralTab.lEquation->setText(("f(x) ="));
	QString file;
	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic: {
		// formula pic depends on degree
		QString numSuffix = QString::number(num);
		if (num > 4)
			numSuffix = "4";
		if ((nsl_fit_model_type_basic)m_fitData.modelType == nsl_fit_model_power && num > 2)
			numSuffix = "2";
		file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "labplot2/pics/fit_models/"
			+ QString(nsl_fit_model_basic_pic_name[m_fitData.modelType]) + numSuffix + ".jpg");
		if (!QFileInfo(file).exists())
			file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
				+ QString(nsl_fit_model_basic_pic_name[m_fitData.modelType]) + numSuffix + ".jpg");
		break;
	}
	case nsl_fit_model_peak: {
		// formula pic depends on number of peaks
		QString numSuffix = QString::number(num);
		if (num > 4)
			numSuffix = "4";
		file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "labplot2/pics/fit_models/"
			+ QString(nsl_fit_model_peak_pic_name[m_fitData.modelType]) + numSuffix + ".jpg");
		if (!QFileInfo(file).exists())
			file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
				+ QString(nsl_fit_model_peak_pic_name[m_fitData.modelType]) + numSuffix + ".jpg");
		break;
	}
	case nsl_fit_model_growth:
		file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "labplot2/pics/fit_models/"
			+ QString(nsl_fit_model_growth_pic_name[m_fitData.modelType]) + ".jpg");
		if (!QFileInfo(file).exists())
			file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
				+ QString(nsl_fit_model_growth_pic_name[m_fitData.modelType]) + ".jpg");
		break;
	case nsl_fit_model_distribution:
		file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "labplot2/pics/gsl_distributions/"
			+ QString(nsl_sf_stats_distribution_pic_name[m_fitData.modelType]) + ".jpg");
		if (!QFileInfo(file).exists())
			file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/gsl_distributions/"
				+ QString(nsl_sf_stats_distribution_pic_name[m_fitData.modelType]) + ".jpg");
		// change label
		if (m_fitData.modelType == nsl_sf_stats_poisson)
			uiGeneralTab.lEquation->setText(("f(k)/A ="));
		else
			uiGeneralTab.lEquation->setText(("f(x)/A ="));
		break;
	case nsl_fit_model_custom:
		uiGeneralTab.teEquation->show();
		uiGeneralTab.lFuncPic->hide();
	}

	if (m_fitData.modelCategory != nsl_fit_model_custom) {
		uiGeneralTab.lFuncPic->setPixmap(QPixmap(file));
		uiGeneralTab.lFuncPic->show();
		uiGeneralTab.teEquation->hide();
	}
}

void XYFitCurveDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insertConstant(QString)));
	connect(&constants, SIGNAL(constantSelected(QString)), &menu, SLOT(close()));
	connect(&constants, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+uiGeneralTab.tbConstants->width(),-menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbConstants->mapToGlobal(pos));
}

void XYFitCurveDock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, SIGNAL(functionSelected(QString)), this, SLOT(insertFunction(QString)));
	connect(&functions, SIGNAL(functionSelected(QString)), &menu, SLOT(close()));
	connect(&functions, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+uiGeneralTab.tbFunctions->width(),-menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbFunctions->mapToGlobal(pos));
}

void XYFitCurveDock::showParameters() {
	QMenu menu;
	FitParametersWidget w(&menu, &m_fitData);
	connect(&w, SIGNAL(finished()), &menu, SLOT(close()));
	connect(&w, SIGNAL(parametersChanged()), this, SLOT(parametersChanged()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&w);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+uiGeneralTab.pbParameters->width(),-menu.sizeHint().height());
	menu.exec(uiGeneralTab.pbParameters->mapToGlobal(pos));
}

/*!
 * called when parameter names and/or start values for the custom model were changed
 */
void XYFitCurveDock::parametersChanged() {
	//parameter names were (probably) changed -> set the new names in EquationTextEdit
	uiGeneralTab.teEquation->setVariables(m_fitData.paramNames);
	enableRecalculate();
}

void XYFitCurveDock::showOptions() {
	QMenu menu;
	FitOptionsWidget w(&menu, &m_fitData);
	connect(&w, SIGNAL(finished()), &menu, SLOT(close()));
	connect(&w, SIGNAL(optionsChanged()), this, SLOT(enableRecalculate()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&w);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+uiGeneralTab.pbParameters->width(),-menu.sizeHint().height());
	menu.exec(uiGeneralTab.pbOptions->mapToGlobal(pos));
}

void XYFitCurveDock::insertFunction(const QString& str) {
	uiGeneralTab.teEquation->insertPlainText(str + "(x)");
}

void XYFitCurveDock::insertConstant(const QString& str) {
	uiGeneralTab.teEquation->insertPlainText(str);
}

void XYFitCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_fitData.degree = uiGeneralTab.sbDegree->value();
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		m_fitData.model = uiGeneralTab.teEquation->toPlainText();

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setFitData(m_fitData);

	this->showFitResult();
	uiGeneralTab.pbRecalculate->setEnabled(false);
	QApplication::restoreOverrideCursor();
}

void XYFitCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no fitting possible without the x- and y-data
	AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
	bool data = (aspectX != 0 && aspectY != 0);

	nsl_fit_model_category category = (nsl_fit_model_category)uiGeneralTab.cbCategory->currentIndex();
	if (category == nsl_fit_model_custom)
		uiGeneralTab.pbRecalculate->setEnabled( data && uiGeneralTab.teEquation->isValid() );
	else
		uiGeneralTab.pbRecalculate->setEnabled(data);
}

/*!
 * show the result and details of fit
 */
void XYFitCurveDock::showFitResult() {
	const XYFitCurve::FitResult& fitResult = m_fitCurve->fitResult();
	if (!fitResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	const XYFitCurve::FitData& fitData = m_fitCurve->fitData();
	QString str = i18n("status:") + ' ' + fitResult.status + "<br>";

	if (!fitResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	str += i18n("iterations:") + ' ' + QString::number(fitResult.iterations) + "<br>";
	if (fitResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", fitResult.elapsedTime/1000) + "<br>";
	else
		str += i18n("calculation time: %1 ms", fitResult.elapsedTime) + "<br>";

	str += i18n("degrees of freedom:") + ' ' + QString::number(fitResult.dof) + "<br><br>";

	str += "<b>" +i18n("Parameters:") + "</b>";
	for (int i = 0; i < fitResult.paramValues.size(); i++) {
		if (fitData.paramFixed.at(i))
			str += "<br>" + fitData.paramNamesUtf8.at(i) + QString(" = ") + QString::number(fitResult.paramValues.at(i));
		else
			str += "<br>" + fitData.paramNamesUtf8.at(i) + QString(" = ") + QString::number(fitResult.paramValues.at(i))
				+ QString::fromUtf8("\u00b1") + QString::number(fitResult.errorValues.at(i))
				+ " (" + QString::number(100.*fitResult.errorValues.at(i)/fabs(fitResult.paramValues.at(i))) + " %)";
	}

	str += "<br><br><b>" + i18n("Goodness of fit:") + "</b><br>";
	str += i18n("sum of squared errors") + " (" + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2") + "): " + QString::number(fitResult.sse) + "<br>";
	str += i18n("mean squared error:") + ' ' + QString::number(fitResult.mse) + "<br>";
	str += i18n("root-mean squared error") + " (" + i18n("reduced") + ' ' + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2") + "): " + QString::number(fitResult.rmse) + "<br>";
	str += i18n("mean absolute error:") + ' ' + QString::number(fitResult.mae) + "<br>";

	if (fitResult.dof != 0) {
		str += i18n("residual mean square:") + ' ' + QString::number(fitResult.rms) + "<br>";
		str += i18n("residual standard deviation:") + ' ' + QString::number(fitResult.rsd) + "<br>";
	}

	str += i18n("coefficient of determination") + " (R" + QString::fromUtf8("\u00b2") + "): " + QString::number(fitResult.rsquared) + "<br>";
	str += i18n("adj. coefficient of determination")+ " (R" + QString::fromUtf8("\u0304") + QString::fromUtf8("\u00b2")
		+ "): " + QString::number(fitResult.rsquaredAdj) + "<br>";
// 	str += "<br><br>";
//
// 	QStringList iterations = fitResult.solverOutput.split(';');
// 	for (int i = 0; i<iterations.size(); ++i)
// 		str += "<br>" + iterations.at(i);

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYFitCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
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

void XYFitCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveWeightsColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbWeightsColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveFitDataChanged(const XYFitCurve::FitData& data) {
	m_initializing = true;
	m_fitData = data;
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		uiGeneralTab.teEquation->setPlainText(m_fitData.model);
	else
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.sbDegree->setValue(m_fitData.degree);
	this->showFitResult();
	m_initializing = false;
}

void XYFitCurveDock::dataChanged() {
	this->enableRecalculate();
}
