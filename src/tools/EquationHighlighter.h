/*
	File             : EquationHighlighter.h
	Project          : LabPlot
	Description      : syntax highlighter for mathematical equations
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EQUATIONHIGHLIGHTER_H
#define EQUATIONHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class KTextEdit;

class EquationHighlighter : public QSyntaxHighlighter {
	Q_OBJECT
public:
	explicit EquationHighlighter(KTextEdit* parent);
	void setVariables(const QStringList&);
	// 	void setErrorPosition(int position);

public Q_SLOTS:
	void rehighlight();

protected:
	void highlightBlock(const QString& text) override;

	//	int m_errorPosition;
	KTextEdit* m_parent;
	QStringList m_variables;
};

#endif
