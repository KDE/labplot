/*
	File                 : BatchEditValueLabelsDialog.h
    Project              : LabPlot
	Description          : Dialog to modify multiply value labels in a batch mode
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
