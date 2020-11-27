/***************************************************************************
    File                 : StatisticsDialog.cpp
    Project              : LabPlot
    Description          : Dialog showing statistics for column values
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 by Fabian Kristof (fkristofszabolcs@gmail.com))
    Copyright            : (C) 2016-2019 by Alexander Semke (alexander.semke@web.de)

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
#include "backend/lib/macros.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

#include <KLocalizedString>
#include <KWindowConfig>
#include <KSharedConfig>

#include <cmath>

StatisticsDialog::StatisticsDialog(const QString& title, const QVector<Column*>& columns, QWidget* parent) : QDialog(parent),
	m_twStatistics(new QTabWidget) {

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok);

	QPushButton* btnOk = btnBox->button(QDialogButtonBox::Ok);
	btnOk->setFocus();

	connect(btnOk, &QPushButton::clicked, this, &StatisticsDialog::close);
	connect(btnBox, &QDialogButtonBox::accepted, this, &StatisticsDialog::accept);

	auto* layout = new QVBoxLayout;
	layout->addWidget(m_twStatistics);
	layout->addWidget(btnBox);

	setLayout(layout);

	setWindowTitle(title);
	setWindowIcon(QIcon::fromTheme("view-statistics"));
	setAttribute(Qt::WA_DeleteOnClose);

	const QString htmlColor = (palette().color(QPalette::Base).lightness() < 128) ? QLatin1String("#5f5f5f") : QLatin1String("#D1D1D1");

	m_htmlText = QString("<table border=0 width=100%>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=" + htmlColor + "><b><big>"
	                     + i18n("Location measures")+
	                     "</big><b></td>"
	                     "</tr>"
// 	                     "<tr></tr>"
	                     "<tr>"
	                     "<td width=70%><b>"
	                     + i18n("Count")+
	                     "<b></td>"
	                     "<td>%1</td>"
	                     "</tr>"
						 "<tr>"
	                     "<td><b>"
	                     + i18n("Minimum")+
	                     "<b></td>"
	                     "<td>%2</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Maximum")+
	                     "<b></td>"
	                     "<td>%3</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Arithmetic mean")+
	                     "<b></td>"
	                     "<td>%4</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Geometric mean")+
	                     "<b></td>"
	                     "<td>%5</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Harmonic mean")+
	                     "<b></td>"
	                     "<td>%6</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Contraharmonic mean")+
	                     "<b></td>"
	                     "<td>%7</td>"
	                     "</tr>"
						"<tr>"
	                     "<td><b>"
	                     + i18n("Mode")+
	                     "<b></td>"
	                     "<td>%8</td>"
	                     "</tr>"
						 "<tr>"
	                     "<td><b>"
	                     + i18n("First Quartile")+
	                     "<b></td>"
	                     "<td>%9</td>"
	                     "</tr>"
						 "<tr>"
	                     "<td><b>"
	                     + i18n("Median")+
	                     "<b></td>"
	                     "<td>%10</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Third Quartile")+
	                     "<b></td>"
	                     "<td>%11</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Trimean")+
	                     "<b></td>"
	                     "<td>%12</td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=" + htmlColor + "><b><big>"
	                     + i18n("Dispersion measures")+
	                     "</big></b></td>"
	                     "</tr>"
// 	                     "<tr></tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Variance")+
	                     "<b></td>"
						 "<td>%13</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Standard deviation")+
	                     "<b></td>"
						 "<td>%14</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Mean absolute deviation around mean")+
	                     "<b></td>"
						 "<td>%15</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Mean absolute deviation around median")+
	                     "<b></td>"
						 "<td>%16</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Median absolute deviation")+
	                     "<b></td>"
						 "<td>%17</td>"
	                     "</tr>"
						 "<tr>"
						  "<td><b>"
						  + i18n("Interquartile Range")+
						  "<b></td>"
						  "<td>%18</td>"
						  "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=" + htmlColor + "><b><big>"
	                     + i18n("Shape measures")+
	                     "</big></b></td>"
	                     "</tr>"
// 	                     "<tr></tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Skewness")+
	                     "<b></td>"
	                     "<td>%19</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Kurtosis")+
	                     "<b></td>"
	                     "<td>%20</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Entropy")+
	                     "<b></td>"
	                     "<td>%21</td>"
	                     "</tr>"
	                     "</table>");


	m_columns = columns;

	//create tab widgets for every column and show the initial text with the placeholders
	if (!m_columns.isEmpty()) {
		for (auto* col : m_columns) {
			auto* textEdit = new QTextEdit(this);
			textEdit->setReadOnly(true);
			m_twStatistics->addTab(textEdit, col->name());
		}

		auto* const textEdit = static_cast<QTextEdit*>(m_twStatistics->currentWidget());
		textEdit->setHtml(m_htmlText.arg(QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-"),
									QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-")).
									arg(QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-"),
										QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-")).
									arg(QLatin1String("-"), QLatin1String("-"), QLatin1String("-")));
	}

	connect(m_twStatistics, &QTabWidget::currentChanged, this, &StatisticsDialog::currentTabChanged);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "StatisticsDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(490, 520));
}

StatisticsDialog::~StatisticsDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "StatisticsDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void StatisticsDialog::showStatistics() {
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	QTimer::singleShot(0, this, [=] () {currentTabChanged(0);});
}

const QString StatisticsDialog::isNanValue(const double value) {
	SET_NUMBER_LOCALE
	return (std::isnan(value) ? QLatin1String("-") : numberLocale.toString(value,'f'));
}

QString modeValue(Column* column, double value) {
	if (std::isnan(value))
		return QLatin1String("-");

	SET_NUMBER_LOCALE
	switch (column->columnMode()) {
	case AbstractColumn::ColumnMode::Integer:
		return numberLocale.toString((int)value);
	case AbstractColumn::ColumnMode::BigInt:
		return numberLocale.toString((qint64)value);
	case AbstractColumn::ColumnMode::Text:
		//TODO
	case AbstractColumn::ColumnMode::DateTime:
		//TODO
	case AbstractColumn::ColumnMode::Day:
		//TODO
	case AbstractColumn::ColumnMode::Month:
		//TODO
	case AbstractColumn::ColumnMode::Numeric:
		return numberLocale.toString(value, 'f');
	}

	return QString();
}

void StatisticsDialog::currentTabChanged(int index) {
	auto* const textEdit = static_cast<QTextEdit*>(m_twStatistics->currentWidget());
	if (!textEdit)
		return;

	WAIT_CURSOR;
	const Column::ColumnStatistics& statistics = m_columns[index]->statistics();

	textEdit->setHtml(m_htmlText.arg(QString::number(statistics.size),
									isNanValue(statistics.minimum == INFINITY ? NAN : statistics.minimum),
									isNanValue(statistics.maximum == -INFINITY ? NAN : statistics.maximum),
									isNanValue(statistics.arithmeticMean),
									isNanValue(statistics.geometricMean),
									isNanValue(statistics.harmonicMean),
									isNanValue(statistics.contraharmonicMean),
									modeValue(m_columns[index], statistics.mode),
									isNanValue(statistics.firstQuartile)).
						arg(isNanValue(statistics.median),
							isNanValue(statistics.thirdQuartile),
							isNanValue(statistics.trimean),
							isNanValue(statistics.variance),
							isNanValue(statistics.standardDeviation),
							isNanValue(statistics.meanDeviation),
							isNanValue(statistics.meanDeviationAroundMedian),
							isNanValue(statistics.medianDeviation),
							isNanValue(statistics.iqr)).
						arg(isNanValue(statistics.skewness),
							isNanValue(statistics.kurtosis),
							isNanValue(statistics.entropy)));
	RESET_CURSOR;
}
