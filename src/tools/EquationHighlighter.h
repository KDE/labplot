/***************************************************************************
    File             : EquationHighlighter.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
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
