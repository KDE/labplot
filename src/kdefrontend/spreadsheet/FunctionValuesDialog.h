/***************************************************************************
    File                 : FunctionValuesDialog.h
    Project              : LabPlot
    Description          : Dialog for generating values from a mathematical function
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2015 by Alexander Semke (alexander.semke@web.de)

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
#ifndef FUNCTIONVALUESDIALOG_H
#define FUNCTIONVALUESDIALOG_H

#include "ui_functionvalueswidget.h"
#include <QDialog>

#include <memory>

class Column;
class Spreadsheet;
class TreeViewComboBox;
class AspectTreeModel;
class QPushButton;

enum class AspectType : quint64;

class FunctionValuesDialog : public QDialog {
	Q_OBJECT

public:
	explicit FunctionValuesDialog(Spreadsheet*, QWidget* parent = nullptr);
	~FunctionValuesDialog() override;
	void setColumns(const QVector<Column*>&);
	bool validVariableName(QLineEdit*);	// check if variable is already defined as constant or function

private:
	Ui::FunctionValuesWidget ui;
	QVector<Column*> m_columns;	// columns to fill with values
	Spreadsheet* m_spreadsheet;	// current spreadsheet
#if __cplusplus < 201103L
	std::auto_ptr<AspectTreeModel> m_aspectTreeModel;
#else
	std::unique_ptr<AspectTreeModel> m_aspectTreeModel;
#endif
	QList<AspectType> m_topLevelClasses;
	QList<AspectType> m_selectableClasses;

	// variable widgets
	QList<QLineEdit*> m_variableLineEdits;
	QList<QLabel*> m_variableLabels;	// '=' labels
	QList<TreeViewComboBox*> m_variableDataColumns;
	QList<QToolButton*> m_variableDeleteButtons;

	QPushButton* m_okButton;

private slots:
	void generate();	// calculate and set values from function
	void checkValues();	// check user input and enable/diable Ok-button accordingly
	void showConstants();	// select predefined constant
	void showFunctions();	// select predefined function
	void insertFunction(const QString&);
	void insertConstant(const QString&);
	void addVariable();
	void deleteVariable();
	void variableNameChanged();
	void variableColumnChanged(const QModelIndex&);	// called when a new column is selected
};

#endif
