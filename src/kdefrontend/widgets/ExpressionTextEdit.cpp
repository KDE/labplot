/****************************************************************************
 **
 ** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 **     of its contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "ExpressionTextEdit.h"
#include "backend/gsl/ExpressionParser.h"
#include "tools/EquationHighlighter.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QScrollBar>

// modified version of http://qt-project.org/doc/qt-4.8/tools-customcompleter.html
ExpressionTextEdit::ExpressionTextEdit(QWidget *parent) : KTextEdit(parent),
	m_highlighter(new EquationHighlighter(this)),
	m_expressionType(XYEquationCurve::Cartesian),
	m_isValid(false) {

	QStringList list = ExpressionParser::getInstance()->functions();
	list.append(ExpressionParser::getInstance()->constants());

	m_completer = new QCompleter(list);
	m_completer->setWidget(this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);

	connect(m_completer, SIGNAL(activated(QString)),this, SLOT(insertCompletion(QString)));
	connect(this, SIGNAL(textChanged()), this, SLOT(validateExpression()) );
	connect(this, SIGNAL(cursorPositionChanged()), m_highlighter, SLOT(rehighlight()) );
}

EquationHighlighter* ExpressionTextEdit::highlighter() {
	return m_highlighter;
}

bool ExpressionTextEdit::isValid() const {
	return (!document()->toPlainText().isEmpty() && m_isValid);
}

void ExpressionTextEdit::setExpressionType(XYEquationCurve::EquationType type) {
	m_expressionType = type;
	m_variables.clear();
	if (type==XYEquationCurve::Cartesian)
		m_variables<<"x";
	else if (type==XYEquationCurve::Polar)
		m_variables<<"phi";
	else if (type==XYEquationCurve::Parametric)
		m_variables<<"t";
	else if (type==XYEquationCurve::Implicit)
		m_variables<<"x"<<"y";

	m_highlighter->setVariables(m_variables);
}

void ExpressionTextEdit::setVariables(const QStringList& vars) {
	m_variables = vars;
	m_highlighter->setVariables(m_variables);
}

void ExpressionTextEdit::insertCompletion(const QString& completion) {
	QTextCursor tc = textCursor();
	int extra = completion.length() - m_completer->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);
	tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(completion.right(extra));
	setTextCursor(tc);
}

QString ExpressionTextEdit::textUnderCursor() const {
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}

void ExpressionTextEdit::focusInEvent(QFocusEvent *e) {
	m_completer->setWidget(this);
	QTextEdit::focusInEvent(e);
}

void ExpressionTextEdit::keyPressEvent(QKeyEvent *e) {
	if (m_completer->popup()->isVisible()) {
		// The following keys are forwarded by the completer to the widget
		switch (e->key()) {
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
			case Qt::Key_Backtab:
				e->ignore();
				return; // let the completer do default behavior
			default:
				break;
		}
	}

	bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
	if (!isShortcut) // do not process the shortcut when we have a completer
		QTextEdit::keyPressEvent(e);

	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if ((ctrlOrShift && e->text().isEmpty()))
		return;

	static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor();

	if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 1
					|| eow.contains(e->text().right(1)))) {
		m_completer->popup()->hide();
		return;
	}

	if (completionPrefix != m_completer->completionPrefix()) {
		m_completer->setCompletionPrefix(completionPrefix);
		m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
	}
	QRect cr = cursorRect();
	cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
				+ m_completer->popup()->verticalScrollBar()->sizeHint().width());
	m_completer->complete(cr); // popup it up!
}

void ExpressionTextEdit::validateExpression() {
	m_isValid = ExpressionParser::getInstance()->isValid(document()->toPlainText(), m_variables);
	if (!m_isValid)
		setStyleSheet("QTextEdit{background: red;}");
	else
		setStyleSheet("QTextEdit{background: white;}"); //TODO: assign the default color for the current style/theme
}
