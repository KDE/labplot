/***************************************************************************
    File             : EquationHighlighter.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright        : (C) 2006  David Saxton <david@bluehaze.org>
    Description      : syntax highligher for mathematical equations

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

#include "EquationHighlighter.h"
#include "backend/gsl/ExpressionParser.h"

#include <QPalette>
#include <KTextEdit>

EquationHighlighter::EquationHighlighter(KTextEdit* parent)
	: QSyntaxHighlighter(parent),
	  m_parent(parent)
// 	m_errorPosition = -1
{}

void EquationHighlighter::setVariables(const QStringList& variables) {
	m_variables = variables;
	rehighlight();
}

void EquationHighlighter::highlightBlock(const QString& text) {
	//TODO: m_parent->checkTextValidity();

	if (text.isEmpty())
		return;

	QTextCharFormat number;
	QTextCharFormat function;
	QTextCharFormat variable;
	QTextCharFormat constant;
	QTextCharFormat matchedParenthesis;

	QPalette palette;
	if (qGray(palette.color(QPalette::Base).rgb()) > 160) {
		number.setForeground(QColor(0, 0, 127));
		function.setForeground(QColor(85, 0, 0));
		function.setFontWeight(QFont::Bold);
		variable.setForeground(QColor(0, 85, 0));
		constant.setForeground(QColor(85, 0, 0));
		matchedParenthesis.setBackground(QColor(255, 255, 183));
	} else {
		number.setForeground(QColor(160, 160, 255));
		function.setForeground(QColor(255, 160, 160));
		function.setFontWeight(QFont::Bold);
		variable.setForeground(QColor(160, 255, 160));
		constant.setForeground(QColor(255, 160, 160));
		matchedParenthesis.setBackground(QColor(85, 85, 0));
	}

	QTextCharFormat other;

	static const QStringList& functions = ExpressionParser::getInstance()->functions();
	static const QStringList& constants = ExpressionParser::getInstance()->constants();

	for (int i = 0; i < text.length(); ++i) {
		QString remaining = text.right(text.length() - i);
		bool found = false;

		//variables
		for (const QString& var: m_variables) {
			if (remaining.startsWith(var)) {
				QString nextChar = remaining.mid(var.length(), 1);
				if (nextChar == " " || nextChar == ")" || nextChar == "+" || nextChar == "-"
					|| nextChar == "*" || nextChar == "/" || nextChar == "^") {
					setFormat(i, var.length(), variable);
					i += var.length() - 1;
					found = true;
					break;
				}
			}
		}
		if (found)
			continue;

		//functions
		for (const QString& f: functions) {
			if (remaining.startsWith(f)) {
				setFormat(i, f.length(), function);
				i += f.length() - 1;
				found = true;
				break;
			}
		}
		if (found)
			continue;

		//constants
		for (const QString& f: constants) {
			if (remaining.startsWith(f)) {
				setFormat(i, f.length(), function);
				i += f.length() - 1;
				found = true;
				break;
			}
		}
		if (found)
			continue;

		//TODO
		/*
		ushort u = text[i].unicode();
		bool isFraction = (u >= 0xbc && u <= 0xbe) || (u >= 0x2153 && u <= 0x215e);
		bool isPower = (u >= 0xb2 && u <= 0xb3) || (u == 0x2070) || (u >= 0x2074 && u <= 0x2079);
		bool isDigit = text[i].isDigit();
		bool isDecimalPoint = text[i] == QLocale().decimalPoint();

		if (isFraction || isPower || isDigit || isDecimalPoint)
			setFormat(i, 1, number);
		else
			setFormat(i, 1, other);
		*/
	}

	//highlight matched brackets
	int cursorPos = m_parent->textCursor().position();
	if (cursorPos < 0)
		cursorPos = 0;

	// Adjust cursorpos to allow for a bracket before the cursor position
	if (cursorPos >= text.size())
		cursorPos = text.size() - 1;
	else if (cursorPos > 0 && (text[cursorPos-1] == '(' || text[cursorPos-1] == ')'))
		cursorPos--;

	bool haveOpen =  text[cursorPos] == '(';
	bool haveClose = text[cursorPos] == ')';

	if ((haveOpen || haveClose) && m_parent->hasFocus()) {
		// Search for the other bracket

		int inc = haveOpen ? 1 : -1; // which direction to search in

		int level = 0;
		for (int i = cursorPos; i >= 0 && i < text.size(); i += inc) {
			if (text[i] == ')')
				level--;
			else if (text[i] == '(')
				level++;

			if (level == 0) {
				// Matched!
				setFormat(cursorPos, 1, matchedParenthesis);
				setFormat(i, 1, matchedParenthesis);
				break;
			}
		}
	}

	//TODO: highlight the position of the error
// 	if (m_errorPosition != -1) {
// 		QTextCharFormat error;
// 		error.setForeground(Qt::red);
//
// 		setFormat(m_errorPosition, 1, error);
// 	}
}

void EquationHighlighter::rehighlight() {
	setDocument(nullptr);
	setDocument(m_parent->document());
}

/**
* This is used to indicate the position where the error occurred.
* If \p position is negative, then no error will be shown.
*/
// void EquationHighlighter::setErrorPosition(int position) {
// 	m_errorPosition = position;
// }
