/*
    File             : ExpressionTextEdit.h
    Project          : LabPlot
    Description      : widget for defining mathematical expressions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014-2017 Alexander Semke <alexander.semke@web.de>
    modified version of https://doc.qt.io/qt-5/qtwidgets-tools-customcompleter-example.html
    SPDX-FileCopyrightText: 2013 Digia Plc and /or its subsidiary(-ies) <http://www.qt-project.org/legal>

    SPDX-License-Identifier: GPL-2.0-or-later AND BSD-3-Clause
*/

#ifndef EXPRESSIONTEXTEDIT_H
#define EXPRESSIONTEXTEDIT_H

#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include <KTextWidgets/KTextEdit>

class QCompleter;
class EquationHighlighter;

class ExpressionTextEdit : public KTextEdit {
	Q_OBJECT

public:
	explicit ExpressionTextEdit(QWidget *parent = nullptr);
	EquationHighlighter* highlighter();
	void setExpressionType(XYEquationCurve::EquationType);
	void setVariables(const QStringList&);
	bool isValid() const;

protected:
	void keyPressEvent(QKeyEvent*) override;
	void focusInEvent(QFocusEvent*) override;
	void focusOutEvent(QFocusEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;

private slots:
	void insertCompletion(const QString&);
	void validateExpression(bool force = false);

private:
	EquationHighlighter* m_highlighter;
	XYEquationCurve::EquationType m_expressionType{XYEquationCurve::EquationType::Neutral};
	QStringList m_variables;
	QCompleter* m_completer;
	bool m_isValid{false};
	QString m_currentExpression;

signals:
	void expressionChanged();
};

#endif
