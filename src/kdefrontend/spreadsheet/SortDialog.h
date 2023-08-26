/*
	File                 : SortDialog.h
	Project              : LabPlot
	Description          : Sorting options dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SORTDIALOG_H
#define SORTDIALOG_H

#include <QDialog>
#include <ui_sortdialogwidget.h>
class Column;

class SortDialog : public QDialog {
	Q_OBJECT

public:
	explicit SortDialog(QWidget* parent = nullptr);
	~SortDialog() override;

	void setColumns(const QVector<Column*>&, const Column* leadingColumn = nullptr);

	enum { Separately = 0, Together = 1 };

private Q_SLOTS:
	void sortColumns();
	void changeType(int index);

Q_SIGNALS:
	void sort(Column*, QVector<Column*>, bool ascending);

private:
	Ui::SortDialogWidget ui;
	QVector<Column*> m_columns;
};

#endif
