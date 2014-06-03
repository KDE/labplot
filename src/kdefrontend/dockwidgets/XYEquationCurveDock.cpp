/***************************************************************************
    File             : XYEquationCurveDock.h
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
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/gsl/ExpressionParser.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QMenu>
#include <QWidgetAction>
#include <QScrollBar>

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

XYEquationCurveDock::XYEquationCurveDock(QWidget *parent): XYCurveDock(parent){

}

/*!
 * 	// Tab "General"
 */
void XYEquationCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout ) {
	  gridLayout->setContentsMargins(2,2,2,2);
	  gridLayout->setHorizontalSpacing(2);
	  gridLayout->setVerticalSpacing(2);
	}

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	uiGeneralTab.tbConstants1->setIcon( KIcon("applications-education-mathematics") );
	uiGeneralTab.tbFunctions1->setIcon( KIcon("preferences-desktop-font") );

	uiGeneralTab.cbType->addItem(i18n("cartesian"));
	uiGeneralTab.cbType->addItem(i18n("polar"));
	uiGeneralTab.cbType->addItem(i18n("parametric"));
	uiGeneralTab.cbType->addItem(i18n("implicit"));

	uiGeneralTab.pbRecalculate->setIcon(KIcon("run-build"));

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)) );
	connect( uiGeneralTab.tbConstants1, SIGNAL(clicked()), this, SLOT(showConstants()) );
	connect( uiGeneralTab.tbFunctions1, SIGNAL(clicked()), this, SLOT(showFunctions()) );
	connect( uiGeneralTab.tbConstants2, SIGNAL(clicked()), this, SLOT(showConstants()) );
	connect( uiGeneralTab.tbFunctions2, SIGNAL(clicked()), this, SLOT(showFunctions()) );
	connect( uiGeneralTab.leMin, SIGNAL(textChanged(QString)), this, SLOT(validateExpression(QString)) );
	connect( uiGeneralTab.leMax, SIGNAL(textChanged(QString)), this, SLOT(validateExpression(QString)) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYEquationCurveDock::setCurves(QList<XYCurve*> list){
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_equationCurve = dynamic_cast<XYEquationCurve*>(m_curve);
	Q_ASSERT(m_equationCurve);
	initGeneralTab();
	initTabs();
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
	const XYEquationCurve* equationCurve = dynamic_cast<const XYEquationCurve*>(m_curve);
	Q_ASSERT(equationCurve);
	const XYEquationCurve::EquationData& data = equationCurve->equationData();
	uiGeneralTab.cbType->setCurrentIndex(data.type);
	uiGeneralTab.teEquation1->setText(data.expression1);
	uiGeneralTab.teEquation2->setText(data.expression2);
	uiGeneralTab.leMin->setText(data.min);
	uiGeneralTab.leMax->setText(data.max);
	uiGeneralTab.sbCount->setValue(data.count);
	this->typeChanged(equationCurve->equationData().type);

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
	XYEquationCurve::EquationType type = XYEquationCurve::EquationType(index);
	if (type==XYEquationCurve::Cartesian) {
		uiGeneralTab.lEquation1->setText("y=f(x)");
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.leMin->show();
		uiGeneralTab.leMax->show();
		uiGeneralTab.lMin->setText(i18n("x, min"));
		uiGeneralTab.lMax->setText(i18n("x, max"));
	} else if (type==XYEquationCurve::Polar) {
		uiGeneralTab.lEquation1->setText(QString::fromUtf8("r(φ)"));
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.leMin->show();
		uiGeneralTab.leMax->show();
		uiGeneralTab.lMin->setText(i18n("φ, min"));
		uiGeneralTab.lMax->setText(i18n("φ, max"));
	} else if (type==XYEquationCurve::Parametric) {
		uiGeneralTab.lEquation1->setText("x=f(t)");
		uiGeneralTab.lEquation2->setText("y=f(t)");
		uiGeneralTab.lEquation2->show();
		uiGeneralTab.teEquation2->show();
		uiGeneralTab.tbFunctions2->show();
		uiGeneralTab.tbConstants2->show();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.leMin->show();
		uiGeneralTab.leMax->show();
		uiGeneralTab.lMin->setText(i18n("t, min"));
		uiGeneralTab.lMax->setText(i18n("t, max"));
	} else if (type==XYEquationCurve::Implicit) {
		uiGeneralTab.lEquation1->setText("f(x,y)");
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->hide();
		uiGeneralTab.lMax->hide();
		uiGeneralTab.leMin->hide();
		uiGeneralTab.leMax->hide();
	}

	uiGeneralTab.teEquation1->setExpressionType(type);
}

void XYEquationCurveDock::recalculateClicked() {
	XYEquationCurve::EquationData data;
	data.type = (XYEquationCurve::EquationType)uiGeneralTab.cbType->currentIndex();
	data.expression1 = uiGeneralTab.teEquation1->document()->toPlainText();
	data.expression2 = uiGeneralTab.teEquation2->document()->toPlainText();
	data.min = uiGeneralTab.leMin->text();
	data.max = uiGeneralTab.leMax->text();
	data.count = uiGeneralTab.sbCount->value();
	m_equationCurve->setEquationData(data);
}

void XYEquationCurveDock::validateExpression(const QString& eq) {
	QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(QObject::sender());
	Q_ASSERT(lineEdit);
	XYEquationCurve::EquationType type = (XYEquationCurve::EquationType)uiGeneralTab.cbType->currentIndex();
	bool rc = ExpressionParser::getInstance()->isValid(eq, type);
	if (!rc)
		lineEdit->setStyleSheet("QLineEdit{background: red;}");
	else
		lineEdit->setStyleSheet("QLineEdit{background: white;}"); //TODO: assign the default color for the current style/theme
}

void XYEquationCurveDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	if (QObject::sender()==uiGeneralTab.tbConstants1)
		connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insert1(QString)));
	else
		connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insert2(QString)));

	connect(&constants, SIGNAL(constantSelected(QString)), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	if (QObject::sender()==uiGeneralTab.tbConstants1) {
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
	if (QObject::sender()==uiGeneralTab.tbFunctions1)
		connect(&functions, SIGNAL(functionSelected(QString)), this, SLOT(insert1(QString)));
	else
		connect(&functions, SIGNAL(functionSelected(QString)), this, SLOT(insert2(QString)));

	connect(&functions, SIGNAL(functionSelected(QString)), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	if (QObject::sender()==uiGeneralTab.tbFunctions1) {
		QPoint pos(-menu.sizeHint().width()+uiGeneralTab.tbFunctions1->width(),-menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbFunctions1->mapToGlobal(pos));
	} else {
		QPoint pos(-menu.sizeHint().width()+uiGeneralTab.tbFunctions2->width(),-menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbFunctions2->mapToGlobal(pos));
	}

}

void XYEquationCurveDock::insert1(const QString& str) {
	uiGeneralTab.teEquation1->insertPlainText(str);
}

void XYEquationCurveDock::insert2(const QString& str) {
	uiGeneralTab.teEquation2->insertPlainText(str);
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
	uiGeneralTab.leMin->setText(data.min);
	uiGeneralTab.leMax->setText(data.max);
	uiGeneralTab.sbCount->setValue(data.count);
	m_initializing = false;
}
