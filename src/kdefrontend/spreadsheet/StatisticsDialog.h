/***************************************************************************
    File                 : StatisticsDialog.h
    Project              : LabPlot
    Description          : Dialog showing statistics for column values
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>

class Column;
class QTabWidget;

class StatisticsDialog : public QDialog {
	Q_OBJECT

public:
	explicit StatisticsDialog(const QString&, QWidget *parent = nullptr);
	~StatisticsDialog() override;
	void setColumns(const QVector<Column*>&);

private:
	const QString isNanValue(const double);

	QTabWidget* m_twStatistics;
	QString m_htmlText;
	QVector<Column*> m_columns;

private slots:
	void currentTabChanged(int);
	void loadSettings();
};

#endif
