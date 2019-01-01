/***************************************************************************
    File             : XYEquationCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Description      : widget for editing properties of equation curves

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

#include "XYEquationCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/gsl/ExpressionParser.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QMenu>
#include <QWidgetAction>

#include <KLocalizedString>

/*!
  \class XYEquationCurveDock
  \brief  Provides a widget for editing the properties of the XYEquationCurves
		(2D-curves defined by a mathematical equation) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYEquationCurveDock::XYEquationCurveDock(QWidget *parent): XYCurveDock(parent) {
	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYEquationCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	auto* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2,2,2,2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	uiGeneralTab.tbConstants1->setIcon( QIcon::fromTheme("labplot-format-text-symbol") );
	uiGeneralTab.tbFunctions1->setIcon( QIcon::fromTheme("preferences-desktop-font") );

	uiGeneralTab.tbConstants2->setIcon( QIcon::fromTheme("labplot-format-text-symbol") );
	uiGeneralTab.tbFunctions2->setIcon( QIcon::fromTheme("preferences-desktop-font") );

	uiGeneralTab.cbType->addItem(i18n("Cartesian"));
	uiGeneralTab.cbType->addItem(i18n("Polar"));
	uiGeneralTab.cbType->addItem(i18n("Parametric"));
// 	uiGeneralTab.cbType->addItem(i18n("Implicit"));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	uiGeneralTab.teEquation2->setExpressionType(XYEquationCurve::Parametric);

	uiGeneralTab.teEquation1->setMaximumHeight(uiGeneralTab.leName->sizeHint().height()*2);
	uiGeneralTab.teEquation2->setMaximumHeight(uiGeneralTab.leName->sizeHint().height()*2);
	uiGeneralTab.teMin->setMaximumHeight(uiGeneralTab.leName->sizeHint().height());
	uiGeneralTab.teMax->setMaximumHeight(uiGeneralTab.leName->sizeHint().height());

	//Slots
	connect( uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYEquationCurveDock::nameChanged );
	connect( uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYEquationCurveDock::commentChanged );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)) );
	connect( uiGeneralTab.teEquation1, SIGNAL(expressionChanged()), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.teEquation2, SIGNAL(expressionChanged()), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.tbConstants1, SIGNAL(clicked()), this, SLOT(showConstants()) );
	connect( uiGeneralTab.tbFunctions1, SIGNAL(clicked()), this, SLOT(showFunctions()) );
	connect( uiGeneralTab.tbConstants2, SIGNAL(clicked()), this, SLOT(showConstants()) );
	connect( uiGeneralTab.tbFunctions2, SIGNAL(clicked()), this, SLOT(showFunctions()) );
	connect( uiGeneralTab.teMin, SIGNAL(expressionChanged()), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.teMax, SIGNAL(expressionChanged()), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.sbCount, SIGNAL(valueChanged(int)), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYEquationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_equationCurve = dynamic_cast<XYEquationCurve*>(m_curve);
	Q_ASSERT(m_equationCurve);
	m_aspectTreeModel =  new AspectTreeModel(m_curve->project());
	XYCurveDock::setModel();
	initGeneralTab();
	initTabs();
	uiGeneralTab.pbRecalculate->setEnabled(false);
	m_initializing=false;
}

void XYEquationCurveDock::initGeneralTab() {
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
	const auto* equationCurve = dynamic_cast<const XYEquationCurve*>(m_curve);
	Q_ASSERT(equationCurve);
	const XYEquationCurve::EquationData& data = equationCurve->equationData();
	uiGeneralTab.cbType->setCurrentIndex(data.type);
	this->typeChanged(data.type);
	uiGeneralTab.teEquation1->setText(data.expression1);
	uiGeneralTab.teEquation2->setText(data.expression2);
	uiGeneralTab.teMin->setText(data.min);
	uiGeneralTab.teMax->setText(data.max);
	uiGeneralTab.sbCount->setValue(data.count);

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_equationCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),
			this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_equationCurve, SIGNAL(equationDataChanged(XYEquationCurve::EquationData)),
			this, SLOT(curveEquationDataChanged(XYEquationCurve::EquationData)));
}

//*************************************************************
//**** SLOTs for changes triggered in XYEquationCurveDock *****
//*************************************************************
void XYEquationCurveDock::nameChanged(){
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYEquationCurveDock::commentChanged(){
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYEquationCurveDock::typeChanged(int index) {
	const auto type = XYEquationCurve::EquationType(index);
	if (type == XYEquationCurve::Cartesian) {
		uiGeneralTab.lEquation1->setText("y=f(x)");
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.teMin->show();
		uiGeneralTab.teMax->show();
		uiGeneralTab.lMin->setText(i18n("x, min"));
		uiGeneralTab.lMax->setText(i18n("x, max"));
	} else if (type == XYEquationCurve::Polar) {
		uiGeneralTab.lEquation1->setText(QString::fromUtf8("r(φ)"));
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.teMin->show();
		uiGeneralTab.teMax->show();
		uiGeneralTab.lMin->setText(i18n("φ, min"));
		uiGeneralTab.lMax->setText(i18n("φ, max"));
	} else if (type == XYEquationCurve::Parametric) {
		uiGeneralTab.lEquation1->setText("x=f(t)");
		uiGeneralTab.lEquation2->setText("y=f(t)");
		uiGeneralTab.lEquation2->show();
		uiGeneralTab.teEquation2->show();
		uiGeneralTab.tbFunctions2->show();
		uiGeneralTab.tbConstants2->show();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.teMin->show();
		uiGeneralTab.teMax->show();
		uiGeneralTab.lMin->setText(i18n("t, min"));
		uiGeneralTab.lMax->setText(i18n("t, max"));
	} else if (type == XYEquationCurve::Implicit) {
		uiGeneralTab.lEquation1->setText("f(x,y)");
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->hide();
		uiGeneralTab.lMax->hide();
		uiGeneralTab.teMin->hide();
		uiGeneralTab.teMax->hide();
	}

	uiGeneralTab.teEquation1->setExpressionType(type);
	this->enableRecalculate();
}

void XYEquationCurveDock::recalculateClicked() {
	XYEquationCurve::EquationData data;
	data.type = (XYEquationCurve::EquationType)uiGeneralTab.cbType->currentIndex();
	data.expression1 = uiGeneralTab.teEquation1->document()->toPlainText();
	data.expression2 = uiGeneralTab.teEquation2->document()->toPlainText();
	data.min = uiGeneralTab.teMin->document()->toPlainText();
	data.max = uiGeneralTab.teMax->document()->toPlainText();
	data.count = uiGeneralTab.sbCount->value();

	for (auto* curve : m_curvesList)
		dynamic_cast<XYEquationCurve*>(curve)->setEquationData(data);

	uiGeneralTab.pbRecalculate->setEnabled(false);
}

void XYEquationCurveDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	if (QObject::sender() == uiGeneralTab.tbConstants1)
		connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insertConstant1(QString)));
	else
		connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insertConstant2(QString)));

	connect(&constants, SIGNAL(constantSelected(QString)), &menu, SLOT(close()));
	connect(&constants, SIGNAL(canceled()), &menu, SLOT(close()));

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	if (QObject::sender() == uiGeneralTab.tbConstants1) {
		QPoint pos(-menu.sizeHint().width()+uiGeneralTab.tbConstants1->width(),-menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbConstants1->mapToGlobal(pos));
	} else {
		QPoint pos(-menu.sizeHint().width()+uiGeneralTab.tbConstants2->width(),-menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbConstants2->mapToGlobal(pos));
	}
}

void XYEquationCurveDock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	if (QObject::sender() == uiGeneralTab.tbFunctions1)
		connect(&functions, SIGNAL(functionSelected(QString)), this, SLOT(insertFunction1(QString)));
	else
		connect(&functions, SIGNAL(functionSelected(QString)), this, SLOT(insertFunction2(QString)));

	connect(&functions, SIGNAL(functionSelected(QString)), &menu, SLOT(close()));
	connect(&functions, SIGNAL(canceled()), &menu, SLOT(close()));

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	if (QObject::sender() == uiGeneralTab.tbFunctions1) {
		QPoint pos(-menu.sizeHint().width()+uiGeneralTab.tbFunctions1->width(),-menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbFunctions1->mapToGlobal(pos));
	} else {
		QPoint pos(-menu.sizeHint().width()+uiGeneralTab.tbFunctions2->width(),-menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbFunctions2->mapToGlobal(pos));
	}
}

void XYEquationCurveDock::insertFunction1(const QString& str) {
	//TODO: not all functions have only one argument
	const auto type = XYEquationCurve::EquationType(uiGeneralTab.cbType->currentIndex());
	if (type == XYEquationCurve::Cartesian)
		uiGeneralTab.teEquation1->insertPlainText(str + "(x)");
	else if (type == XYEquationCurve::Polar)
		uiGeneralTab.teEquation1->insertPlainText(str + "(phi)");
	else if (type == XYEquationCurve::Parametric)
		uiGeneralTab.teEquation1->insertPlainText(str + "(t)");
}

void XYEquationCurveDock::insertConstant1(const QString& str) {
	uiGeneralTab.teEquation1->insertPlainText(str);
}

void XYEquationCurveDock::insertFunction2(const QString& str) {
	//TODO: not all functions have only one argument
	uiGeneralTab.teEquation2->insertPlainText(str + "(t)");
}

void XYEquationCurveDock::insertConstant2(const QString& str) {
	uiGeneralTab.teEquation2->insertPlainText(str);
}

void XYEquationCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//check whether the formular expressions are correct
	bool valid = false;
	const auto type = XYEquationCurve::EquationType(uiGeneralTab.cbType->currentIndex());
	if (type != XYEquationCurve::Parametric)
		valid = uiGeneralTab.teEquation1->isValid();
	else
		valid = (uiGeneralTab.teEquation1->isValid() && uiGeneralTab.teEquation2->isValid());

	valid = (valid && uiGeneralTab.teMin->isValid() && uiGeneralTab.teMax->isValid());
	uiGeneralTab.pbRecalculate->setEnabled(valid);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYEquationCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
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

void XYEquationCurveDock::curveEquationDataChanged(const XYEquationCurve::EquationData& data) {
	m_initializing = true;
	uiGeneralTab.cbType->setCurrentIndex(data.type);
	uiGeneralTab.teEquation1->setText(data.expression1);
	uiGeneralTab.teEquation2->setText(data.expression2);
	uiGeneralTab.teMin->setText(data.min);
	uiGeneralTab.teMax->setText(data.max);
	uiGeneralTab.sbCount->setValue(data.count);
	m_initializing = false;
}
