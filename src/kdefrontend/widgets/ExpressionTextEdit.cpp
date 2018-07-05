/***************************************************************************
    File             : ExpressionTextEdit.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)
    Description      : widget for defining mathematical expressions
					   modified version of
					   http://qt-project.org/doc/qt-4.8/tools-customcompleter.html
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

/*!
  \class ExpressionTextEdit
  \brief  Provides a widget for defining mathematical expressions
		  Supports syntax-highlighting and completion.

		  Modified version of http://qt-project.org/doc/qt-4.8/tools-customcompleter.html

  \ingroup kdefrontend
*/
ExpressionTextEdit::ExpressionTextEdit(QWidget* parent) : KTextEdit(parent),
	m_highlighter(new EquationHighlighter(this)),
	m_expressionType(XYEquationCurve::Neutral),
	m_isValid(false) {

	QStringList list = ExpressionParser::getInstance()->functions();
	list.append(ExpressionParser::getInstance()->constants());

	setTabChangesFocus(true);

	m_completer = new QCompleter(list, this);
	m_completer->setWidget(this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);

	connect(m_completer, static_cast<void (QCompleter::*) (const QString&)>(&QCompleter::activated),this, &ExpressionTextEdit::insertCompletion);
	connect(this, &ExpressionTextEdit::textChanged, this, [=](){ validateExpression();});
	connect(this, &ExpressionTextEdit::cursorPositionChanged, m_highlighter, &EquationHighlighter::rehighlight);
}

EquationHighlighter* ExpressionTextEdit::highlighter() {
	return m_highlighter;
}

bool ExpressionTextEdit::isValid() const {
	return (!document()->toPlainText().trimmed().isEmpty() && m_isValid);
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
	validateExpression(true);
}

void ExpressionTextEdit::insertCompletion(const QString& completion) {
	QTextCursor tc = textCursor();
	int extra = completion.length() - m_completer->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);
	tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(completion.right(extra));
	setTextCursor(tc);
}


/*!
 * \brief Validates the current expression if the text was changed and highlights the text field red if the expression is invalid.
 * \param force forces the validation and highlighting when no text changes were made, used when new parameters/variables were provided
 */
void ExpressionTextEdit::validateExpression(bool force) {
	//check whether the expression was changed or only the formating
	QString text = toPlainText();
	bool textChanged = (text != m_currentExpression) ? true : false;

	if (textChanged || force) {
		m_isValid = ExpressionParser::getInstance()->isValid(text, m_variables);
		if (!m_isValid)
			setStyleSheet("QTextEdit{background: red;}");
		else
			setStyleSheet("");

		m_currentExpression = text;
	}
	if (textChanged)
		emit expressionChanged();
}

//##############################################################################
//####################################  Events   ###############################
//##############################################################################
void ExpressionTextEdit::focusInEvent(QFocusEvent* e) {
	m_completer->setWidget(this);
	QTextEdit::focusInEvent(e);
}

void ExpressionTextEdit::focusOutEvent(QFocusEvent* e) {
	//when loosing focus, rehighlight to remove potential highlighting of opening and closing brackets
	m_highlighter->rehighlight();
	QTextEdit::focusOutEvent(e);
}

void ExpressionTextEdit::keyPressEvent(QKeyEvent* e) {
	switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			e->ignore();
			return;
		default:
			break;
	}

	bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
	if (!isShortcut) // do not process the shortcut when we have a completer
		QTextEdit::keyPressEvent(e);

	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if ((ctrlOrShift && e->text().isEmpty()))
		return;

	static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	const QString& completionPrefix = tc.selectedText();

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

void ExpressionTextEdit::mouseMoveEvent(QMouseEvent* e) {
	QTextCursor tc = cursorForPosition(e->pos());
	tc.select(QTextCursor::WordUnderCursor);

	const QString& token = tc.selectedText();
	if (token.isEmpty()) {
		setToolTip("");
		return;
	}

	//try to find the token under the mouse cursor in the list of constants first
	static const QStringList& constants = ExpressionParser::getInstance()->constants();
	int index = constants.indexOf(token);

	if (index != -1) {
		static const QStringList& names = ExpressionParser::getInstance()->constantsNames();
		static const QStringList& values = ExpressionParser::getInstance()->constantsValues();
		static const QStringList& units = ExpressionParser::getInstance()->constantsUnits();
		setToolTip(names.at(index) + ": " + constants.at(index) + " = " + values.at(index) + ' ' + units.at(index));
	} else {
		//text token was not found in the list of constants -> check functions as next
		static const QStringList& functions = ExpressionParser::getInstance()->functions();
		index = functions.indexOf(token);
		if (index != -1) {
			static const QStringList& names = ExpressionParser::getInstance()->functionsNames();
			setToolTip(functions.at(index) + " - " + names.at(index));
		} else
			setToolTip("");
	}

	KTextEdit::mouseMoveEvent(e);
}
