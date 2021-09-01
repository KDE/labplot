/*
    File             : EquationHighlighter.h
    Project          : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
    Description      : syntax highligher for mathematical equations
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EQUATIONHIGHLIGHTER_H
#define EQUATIONHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class QStringList;
class KTextEdit;

class EquationHighlighter : public QSyntaxHighlighter {
	Q_OBJECT
public:
	explicit EquationHighlighter(KTextEdit* parent);
	void setVariables(const QStringList&);
// 	void setErrorPosition(int position);

public slots:
	void rehighlight();

protected:
	void highlightBlock(const QString& text) override;

//	int m_errorPosition;
	KTextEdit* m_parent;
	QStringList m_variables;
};

#endif
