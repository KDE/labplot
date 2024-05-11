/*
	File             : ExpressionTextEdit.cpp
	Project          : LabPlot
	Description      : widget for defining mathematical expressions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2017 Alexander Semke <alexander.semke@web.de>
	modified version of https://doc.qt.io/qt-5/qtwidgets-tools-customcompleter-example.html
	SPDX-FileCopyrightText: 2013 Digia Plc and /or its subsidiary(-ies) <http://www.qt-project.org/legal>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later AND BSD-3-Clause
*/

#include "ExpressionTextEdit.h"
#include "backend/gsl/ExpressionParser.h"
#include "tools/EquationHighlighter.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>

/*!
  \class ExpressionTextEdit
  \brief  Provides a widget for defining mathematical expressions
		  Supports syntax-highlighting and completion.

		  Modified version of https://doc.qt.io/qt-5/qtwidgets-tools-customcompleter-example.html

  \ingroup kdefrontend
*/
ExpressionTextEdit::ExpressionTextEdit(QWidget* parent)
	: KTextEdit(parent)
	, m_highlighter(new EquationHighlighter(this)) {
	QStringList list = ExpressionParser::getInstance()->functions();
	// append description
	for (auto& s : list)
		s.append(ExpressionParser::functionArgumentString(s, XYEquationCurve::EquationType::Cartesian) + QStringLiteral(" - ")
				 + ExpressionParser::getInstance()->functionDescription(s));
	QStringList constants = ExpressionParser::getInstance()->constants();
	for (auto& s : constants) {
		if (s != QLatin1String("..."))
			s.append(QStringLiteral(" - ") + ExpressionParser::getInstance()->constantDescription(s));
	}
	list.append(constants);

	setTabChangesFocus(true);

	m_completer = new QCompleter(list, this);
	m_completer->setWidget(this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);

	connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated), this, &ExpressionTextEdit::insertCompletion);
	connect(this, &ExpressionTextEdit::textChanged, this, [=]() {
		validateExpression();
	});
	connect(this, &ExpressionTextEdit::cursorPositionChanged, m_highlighter, &EquationHighlighter::rehighlight);
}

EquationHighlighter* ExpressionTextEdit::highlighter() {
	return m_highlighter;
}

bool ExpressionTextEdit::isValid() const {
	return (!document()->toPlainText().simplified().isEmpty() && m_isValid);
}

QString ExpressionTextEdit::errorMessage() const {
	const auto parser = ExpressionParser::getInstance();
	return parser->errorMessage();
}

void ExpressionTextEdit::setExpressionType(XYEquationCurve::EquationType type) {
	m_expressionType = type;
	m_variables.clear();
	if (type == XYEquationCurve::EquationType::Cartesian)
		m_variables << QStringLiteral("x");
	else if (type == XYEquationCurve::EquationType::Polar)
		m_variables << QStringLiteral("phi");
	else if (type == XYEquationCurve::EquationType::Parametric)
		m_variables << QStringLiteral("t");
	else if (type == XYEquationCurve::EquationType::Implicit)
		m_variables << QStringLiteral("x") << QStringLiteral("y");

	m_highlighter->setVariables(m_variables);
}

void ExpressionTextEdit::setVariables(const QStringList& vars) {
	m_variables = vars;
	m_highlighter->setVariables(m_variables);
	validateExpression(true);
}

void ExpressionTextEdit::insertCompletion(const QString& completion) {
	QTextCursor tc{textCursor()};

	// remove description
	int nameLength = completion.indexOf(QLatin1String(" - "));
	QString name = completion;
	name.truncate(nameLength);

	int extra = name.length() - m_completer->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);
	tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(name.right(extra));
	setTextCursor(tc);
}

/*!
 * \brief Validates the current expression if the text was changed and highlights the text field if the expression is invalid.
 * \param force forces the validation and highlighting when no text changes were made, used when new parameters/variables were provided
 */
void ExpressionTextEdit::validateExpression(bool force) {
	// check whether the expression was changed or only the formatting
	QString text = toPlainText().simplified();
	bool textChanged{(text != m_currentExpression) ? true : false};

	if (textChanged || force) {
		auto parser = ExpressionParser::getInstance();
		m_isValid = parser->isValid(text, m_variables);
		if (!m_isValid) {
			setToolTip(parser->errorMessage());
			SET_WARNING_STYLE(this)
		} else {
			setToolTip(QStringLiteral(""));
			setStyleSheet(QString());
		}

		m_currentExpression = std::move(text);
	}
	if (textChanged)
		Q_EMIT expressionChanged();
}

// ##############################################################################
// ####################################  Events   ###############################
// ##############################################################################
void ExpressionTextEdit::focusInEvent(QFocusEvent* e) {
	m_completer->setWidget(this);
	QTextEdit::focusInEvent(e);
}

void ExpressionTextEdit::focusOutEvent(QFocusEvent* e) {
	// when loosing focus, rehighlight to remove potential highlighting of opening and closing brackets
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

	const bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
	if (!isShortcut) // do not process the shortcut when we have a completer
		QTextEdit::keyPressEvent(e);

	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if ((ctrlOrShift && e->text().isEmpty()))
		return;

	static QString eow(QStringLiteral("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=")); // end of word
	const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	const QString& completionPrefix = tc.selectedText();

	if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 1 || eow.contains(e->text().right(1)))) {
		m_completer->popup()->hide();
		return;
	}

	if (completionPrefix != m_completer->completionPrefix()) {
		m_completer->setCompletionPrefix(completionPrefix);
		m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
	}
	QRect cr{cursorRect()};
	cr.setWidth(m_completer->popup()->sizeHintForColumn(0) + m_completer->popup()->verticalScrollBar()->sizeHint().width());
	m_completer->complete(cr); // popup it up!
}

void ExpressionTextEdit::mouseMoveEvent(QMouseEvent* e) {
	QTextCursor tc = cursorForPosition(e->pos());
	tc.select(QTextCursor::WordUnderCursor);

	const QString& token = tc.selectedText();
	if (token.isEmpty()) {
		setToolTip(QString());
		return;
	}

	// try to find the token under the mouse cursor in the list of constants first
	static const QStringList& constants = ExpressionParser::getInstance()->constants();
	int index = constants.indexOf(token);

	if (index != -1) {
		static const QStringList& names = ExpressionParser::getInstance()->constantsNames();
		static const QStringList& values = ExpressionParser::getInstance()->constantsValues();
		static const QStringList& units = ExpressionParser::getInstance()->constantsUnits();
		setToolTip(names.at(index) + QStringLiteral(": ") + constants.at(index) + QStringLiteral(" = ") + values.at(index) + QStringLiteral(" ")
				   + units.at(index));
	} else {
		// text token was not found in the list of constants -> check functions as next
		static const QStringList& functions = ExpressionParser::getInstance()->functions();
		index = functions.indexOf(token);
		if (index != -1) {
			static const QStringList& names = ExpressionParser::getInstance()->functionsDescriptions();
			setToolTip(functions.at(index) + QStringLiteral(" - ") + names.at(index));
		} else
			setToolTip(QString());
	}

	KTextEdit::mouseMoveEvent(e);
}
