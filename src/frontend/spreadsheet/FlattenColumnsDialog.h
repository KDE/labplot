/*
	File                 : FlattenColumnsDialog.h
	Project              : LabPlot
	Description          : Dialog for flattening of spreadsheet columns
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FLATTENCOLUMNSDIALOG_H
#define FLATTENCOLUMNSDIALOG_H

#include "ui_flattencolumnswidget.h"
#include <QDialog>

class Column;
class Spreadsheet;
class QComboBox;

class FlattenColumnsDialog : public QDialog {
	Q_OBJECT

public:
	explicit FlattenColumnsDialog(Spreadsheet* s, QWidget* parent = nullptr);
	~FlattenColumnsDialog() override;
	void setColumns(const QVector<Column*>&);
	void flatten(const Spreadsheet* source, const QVector<Column*>& valuesColumns, const QVector<Column*>& referenceColumns) const;

private:
	Ui::FlattenColumnsWidget ui;
	Spreadsheet* m_spreadsheet;
	QVector<Column*> m_columns;
	QStringList m_referenceColumnNames;

	// widgets to handle the reference columns
	QPushButton* m_okButton;
	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<QComboBox*> m_columnComboBoxes;
	QVector<QPushButton*> m_removeButtons;

private Q_SLOTS:
	void flattenColumns() const;
	void addReferenceColumn();
	void removeReferenceColumn();
};

#endif
