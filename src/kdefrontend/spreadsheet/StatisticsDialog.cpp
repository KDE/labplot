/***************************************************************************
    File                 : StatisticsDialog.cpp
    Project              : LabPlot
    Description          : Dialog showing statistics for column values
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
    Copyright            : (C) 2016 by Alexander Semke (alexander.semke@web.de)

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

#include "StatisticsDialog.h"
#include "backend/core/column/Column.h"

#include <QTextEdit>
#include <QTabWidget>
#include <KLocale>

#include <cmath>

StatisticsDialog::StatisticsDialog(const QString& title, QWidget* parent) :
	KDialog(parent) {

	twStatistics = new QTabWidget(this);
	setMainWidget(twStatistics);

	setWindowTitle(title);
	setButtons(KDialog::Ok);
	setButtonText(KDialog::Ok, i18n("&Ok"));

	m_htmlText = QString("<table border=0 width=100%>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>"
	                     + i18n("Location measures")+
	                     "</big><b></td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td width=70%><b>"
	                     + i18n("Minimum")+
	                     "<b></td>"
	                     "<td>%1</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Maximum")+
	                     "<b></td>"
	                     "<td>%2</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Arithmetic mean")+
	                     "<b></td>"
	                     "<td>%3</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Geometric mean")+
	                     "<b></td>"
	                     "<td>%4</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Harmonic mean")+
	                     "<b></td>"
	                     "<td>%5</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Contraharmonic mean")+
	                     "<b></td>"
	                     "<td>%6</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Median")+
	                     "<b></td>"
	                     "<td>%7</td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>"
	                     + i18n("Dispersion measures")+
	                     "</big></b></td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Variance")+
	                     "<b></td>"
	                     "<td>%8</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Standard deviation")+
	                     "<b></td>"
	                     "<td>%9</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Mean absolute deviation around mean")+
	                     "<b></td>"
	                     "<td>%10</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Mean absolute deviation around median")+
	                     "<b></td>"
	                     "<td>%11</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Median absolute deviation")+
	                     "<b></td>"
	                     "<td>%12</td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>"
	                     + i18n("Shape measures")+
	                     "</big></b></td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Skewness")+
	                     "<b></td>"
	                     "<td>%13</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Kurtosis")+
	                     "<b></td>"
	                     "<td>%14</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Entropy")+
	                     "<b></td>"
	                     "<td>%15</td>"
	                     "</tr>"
	                     "</table>");

    connect(twStatistics, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
	connect(this, SIGNAL(okClicked()), this, SLOT(close()));
}

void StatisticsDialog::setColumns(const QList<Column*>& columns) {
	if (!columns.size())
		return;

	m_columns = columns;

	for (int i = 0; i < m_columns.size(); ++i) {
		QTextEdit* textEdit = new QTextEdit;
		textEdit->setReadOnly(true);
		twStatistics->addTab(textEdit, m_columns[i]->name());
	}
    currentTabChanged(0);
}

const QString StatisticsDialog::isNanValue(const double value) {
	return (std::isnan(value) ? i18n("The value couldn't be calculated.") : QString::number(value,'g', 10));
}

QSize StatisticsDialog::sizeHint() const {
	return QSize(490, 520);
}

void StatisticsDialog::currentTabChanged(int index) {
	WAIT_CURSOR;
	const Column::ColumnStatistics& statistics = m_columns[index]->statistics();
	RESET_CURSOR;

	QTextEdit* textEdit = static_cast<QTextEdit*>(twStatistics->currentWidget());
	textEdit->setHtml(m_htmlText.arg(isNanValue(statistics.minimum)).
	                  arg(isNanValue(statistics.maximum)).
	                  arg(isNanValue(statistics.arithmeticMean)).
	                  arg(isNanValue(statistics.geometricMean)).
	                  arg(isNanValue(statistics.harmonicMean)).
	                  arg(isNanValue(statistics.contraharmonicMean)).
	                  arg(isNanValue(statistics.median)).
	                  arg(isNanValue(statistics.variance)).
	                  arg(isNanValue(statistics.standardDeviation)).
	                  arg(isNanValue(statistics.meanDeviation)).
	                  arg(isNanValue(statistics.meanDeviationAroundMedian)).
	                  arg(isNanValue(statistics.medianDeviation)).
	                  arg(isNanValue(statistics.skewness)).
	                  arg(isNanValue(statistics.kurtosis)).
	                  arg(isNanValue(statistics.entropy)));
}
