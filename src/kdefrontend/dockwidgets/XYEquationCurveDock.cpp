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
#include "tools/EquationHighlighter.h"

#include <QCompleter>
#include <QKeyEvent>
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
	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	uiGeneralTab.tbConstants1->setIcon( KIcon("applications-education-mathematics") );
	uiGeneralTab.tbConstants2->setIcon( KIcon("applications-education-mathematics") );

	uiGeneralTab.cbType->addItem(i18n("cartesian"));
	uiGeneralTab.cbType->addItem(i18n("polar"));
	uiGeneralTab.cbType->addItem(i18n("parametric"));
	uiGeneralTab.cbType->addItem(i18n("implicit"));

	uiGeneralTab.pbRecalculate->setIcon(KIcon("run-build"));

	//highlighting
	m_highlighter1 = new EquationHighlighter(uiGeneralTab.teEquation1);
	m_highlighter2 = new EquationHighlighter(uiGeneralTab.teEquation2);

	//currently, highlighter2 is for implicit equations only, with two parameters - x and y
	QStringList vars;
	vars<<"x"<<"y";
	m_highlighter2->setVariables(vars);

	//completion
	QStringList list = ExpressionParser::getInstance()->functionsList();
	list.append(ExpressionParser::getInstance()->constantsList());
	m_completer1 = new QCompleter(list);
	m_completer1->setWidget(uiGeneralTab.teEquation1);
	m_completer1->setCompletionMode(QCompleter::PopupCompletion);
	m_completer1->setCaseSensitivity(Qt::CaseInsensitive);
	connect(m_completer1, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));

	m_completer2 = new QCompleter(list);
	m_completer2->setWidget(uiGeneralTab.teEquation2);
	m_completer2->setCompletionMode(QCompleter::PopupCompletion);
	m_completer2->setCaseSensitivity(Qt::CaseInsensitive);
	connect(m_completer2, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));

	uiGeneralTab.teEquation1->installEventFilter(this);
	uiGeneralTab.teEquation2->installEventFilter(this);

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)) );
	connect( uiGeneralTab.teEquation1, SIGNAL(textChanged()), this, SLOT(validateExpression()) );
	connect( uiGeneralTab.teEquation1, SIGNAL(cursorPositionChanged()), m_highlighter1, SLOT(rehighlight()) );
	connect( uiGeneralTab.teEquation2, SIGNAL(textChanged()), this, SLOT(validateExpression()) );
	connect( uiGeneralTab.teEquation2, SIGNAL(cursorPositionChanged()), m_highlighter2, SLOT(rehighlight()) );
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

/*!
 * Code for the completion adopted from http://qt-project.org/doc/qt-4.8/tools-customcompleter.html
 */
bool XYEquationCurveDock::eventFilter(QObject* object, QEvent* event){
	if (event->type()!=QEvent::KeyPress)
		return false;

	QCompleter* c;
	QTextEdit* te;
	if (object==uiGeneralTab.teEquation1) {
		c = m_completer1;
		te = uiGeneralTab.teEquation1;
	} else if (object==uiGeneralTab.teEquation2) {
		c = m_completer2;
		te = uiGeneralTab.teEquation2;
	} else {
		return false;
	}

	QKeyEvent* e = dynamic_cast<QKeyEvent*>(event);
	if (c && c->popup()->isVisible()) {
         // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
             e->ignore();
             return false; // let the completer do default behavior
        default:
            break;
        }
     }

     bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+SPACE
     if (!c || !isShortcut) // do not process the shortcut when we have a completer
		 return false;

     const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
     if (!c || (ctrlOrShift && e->text().isEmpty()))
         return false;

     static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
     bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;

	 QTextCursor tc = te->textCursor();
     tc.select(QTextCursor::WordUnderCursor);
     QString completionPrefix = tc.selectedText();

     if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
                       || eow.contains(e->text().right(1)))) {
         c->popup()->hide();
         return false;
     }

     if (completionPrefix != c->completionPrefix()) {
         c->setCompletionPrefix(completionPrefix);
         c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
     }
     QRect cr = te->cursorRect();
     cr.setWidth(c->popup()->sizeHintForColumn(0)
                 + c->popup()->verticalScrollBar()->sizeHint().width());
     c->complete(cr); // popup it up!

	 return true;
}

void XYEquationCurveDock::insertCompletion(const QString& completion) {
	QTextEdit* te;
	QCompleter* c;
	if (QObject::sender() == m_completer1) {
		te = uiGeneralTab.teEquation1;
		c = m_completer1;
	} else {
		te = uiGeneralTab.teEquation2;
		c = m_completer2;
	}

	QTextCursor tc = te->textCursor();
	int extra = completion.length() - c->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);
	tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(completion.right(extra));
	te->setTextCursor(tc);
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
	QStringList vars;
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
		vars<<"x";
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
		vars<<"phi";
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
		vars<<"t";
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
		vars<<"x"<<"y";
	}
	m_highlighter1->setVariables(vars);
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

void XYEquationCurveDock::validateExpression() {
	QTextEdit* textEdit = dynamic_cast<QTextEdit*>(QObject::sender());
	Q_ASSERT(textEdit);
	XYEquationCurve::EquationType type = (XYEquationCurve::EquationType)uiGeneralTab.cbType->currentIndex();
	bool rc = ExpressionParser::getInstance()->isValid(textEdit->document()->toPlainText(), type);
	if (!rc)
		textEdit->setStyleSheet("QTextEdit{background: red;}");
	else
		textEdit->setStyleSheet("QTextEdit{background: white;}"); //TODO: assign the default color for the current style/theme
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
