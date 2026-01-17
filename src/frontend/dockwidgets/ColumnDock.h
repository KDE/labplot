/*
	File                 : ColumnDock.h
	Project              : LabPlot
	Description          : widget for column properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLUMNDOCK_H
#define COLUMNDOCK_H

#include "backend/core/AbstractColumn.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_columndock.h"

class Column;
class Spreadsheet;
class TreeViewComboBox;

class ColumnDock : public BaseDock {
	Q_OBJECT

public:
	explicit ColumnDock(QWidget*);
	void setColumns(QList<Column*>);
	void retranslateUi() override;

private:
	Ui::ColumnDock ui;
	QList<Column*> m_columns;
	Column* m_column{nullptr};
	Spreadsheet* m_spreadsheet{nullptr}; // parent spreadsheet, if available

	// formula
	QList<QLineEdit*> m_variableLineEdits;
	QList<QLabel*> m_variableLabels;
	QList<TreeViewComboBox*> m_variableDataColumns;
	QList<QToolButton*> m_variableDeleteButtons;

	void updateTypeWidgets(AbstractColumn::ColumnMode);
	void showValueLabels();
	bool validVariableName(QLineEdit*);

private Q_SLOTS:
	void typeChanged(int);
	void numericFormatChanged(int);
	void precisionChanged(int);
	void dateTimeFormatChanged(const QString&);
	void plotDesignationChanged(int);

	// formula
	void loadFormula();
	void applyFormula(); // calculate and set values from function
	void checkValues(); // check user input and enable/disable Ok-button accordingly
	void loadFunction();
	void saveFunction();
	void showConstants(); // select predefined constant
	void showFunctions(); // select predefined function
	void insertFunction(const QString&) const;
	void insertConstant(const QString&) const;
	void addVariable();
	void deleteVariable();
	void variableNameChanged();
	void variableColumnChanged(const QModelIndex&); // called when a new column is selected

	// value labels
	void addLabel();
	void removeLabel();
	void batchEditLabels();

	// SLOTs for changes triggered in Column
	void columnModeChanged(const AbstractAspect*);
	void columnFormatChanged();
	void columnPrecisionChanged();
	void columnPlotDesignationChanged(const AbstractColumn*);

Q_SIGNALS:
	void info(const QString&);
};

#endif // COLUMNDOCK_H
