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
	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	uiGeneralTab.tbConstants->setIcon( KIcon("applications-education-mathematics") );
	uiGeneralTab.tbConstants2->setIcon( KIcon("applications-education-mathematics") );

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
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYEquationCurveDock::setCurves(QList<XYCurve*> list){
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
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
	uiGeneralTab.cbType->setCurrentIndex(equationCurve->equationType());
	this->typeChanged(equationCurve->equationType());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
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
		uiGeneralTab.lParamFuncX->setText("y=f(x)");
		uiGeneralTab.lParamFuncY->hide();
		uiGeneralTab.leParamFuncY->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lParameterMin->show();
		uiGeneralTab.lParameterMax->show();
		uiGeneralTab.leParameterMin->show();
		uiGeneralTab.leParameterMax->show();
		uiGeneralTab.lParameterMin->setText(i18n("x, min"));
		uiGeneralTab.lParameterMax->setText(i18n("x, max"));
	} else if (type==XYEquationCurve::Polar) {
		uiGeneralTab.lParamFuncX->setText(QString::fromUtf8("r(φ)"));
		uiGeneralTab.lParamFuncY->hide();
		uiGeneralTab.leParamFuncY->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lParameterMin->show();
		uiGeneralTab.lParameterMax->show();
		uiGeneralTab.leParameterMin->show();
		uiGeneralTab.leParameterMax->show();
		uiGeneralTab.lParameterMin->setText(i18n("φ, min"));
		uiGeneralTab.lParameterMax->setText(i18n("φ, max"));
	} else if (type==XYEquationCurve::Parametric) {
		uiGeneralTab.lParamFuncX->setText("x=f(t)");
		uiGeneralTab.lParamFuncY->setText("y=f(t)");
		uiGeneralTab.lParamFuncY->show();
		uiGeneralTab.leParamFuncY->show();
		uiGeneralTab.tbFunctions2->show();
		uiGeneralTab.tbConstants2->show();
		uiGeneralTab.lParameterMin->show();
		uiGeneralTab.lParameterMax->show();
		uiGeneralTab.leParameterMin->show();
		uiGeneralTab.leParameterMax->show();
		uiGeneralTab.lParameterMin->setText(i18n("t, min"));
		uiGeneralTab.lParameterMax->setText(i18n("t, max"));
	} else if (type==XYEquationCurve::Implicit) {
		uiGeneralTab.lParamFuncX->setText("f(x,y)");
		uiGeneralTab.lParamFuncY->hide();
		uiGeneralTab.leParamFuncY->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lParameterMin->hide();
		uiGeneralTab.lParameterMax->hide();
		uiGeneralTab.leParameterMin->hide();
		uiGeneralTab.leParameterMax->hide();
	}
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

