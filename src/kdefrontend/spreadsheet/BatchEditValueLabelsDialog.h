/***************************************************************************
	File                 : BatchEditValueLabelsDialog.h
    Project              : LabPlot
	Description          : Dialog to modify multiply value labels in a batch mode
    --------------------------------------------------------------------
    Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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
#ifndef BATCHEDITVALUELABELSDIALOG_H
#define BATCHEDITVALUELABELSDIALOG_H

#include <QDialog>

class Column;
class QTextEdit;

class BatchEditValueLabelsDialog : public QDialog {
	Q_OBJECT

public:
	explicit BatchEditValueLabelsDialog(QWidget* parent = nullptr);
	~BatchEditValueLabelsDialog() override;

	void setColumns(QList<Column*>);

private:
	QTextEdit* teValueLabels;
	QList<Column*> m_columns;
	Column* m_column;
	QString m_dateTimeFormat;

private slots:
	void save() const;
};

#endif
