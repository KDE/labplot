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
#include <QLineEdit>

#include <memory>

class Column;
class Spreadsheet;
class TreeViewComboBox;
class AspectTreeModel;
class QPushButton;
class QLineEdit;
class FunctionValuesDialog : public QDialog {
	Q_OBJECT

	public:
		explicit FunctionValuesDialog(Spreadsheet* s, QWidget* parent = 0);
		~FunctionValuesDialog();
		void setColumns(QVector<Column*>);

	private:
		Ui::FunctionValuesWidget ui;
		QVector<Column*> m_columns;
		Spreadsheet* m_spreadsheet;
#if __cplusplus < 201103L
		std::auto_ptr<AspectTreeModel> m_aspectTreeModel;
#else
		std::unique_ptr<AspectTreeModel> m_aspectTreeModel;
#endif
		QList<const char*>  m_topLevelClasses;
		QList<const char*>  m_selectableClasses;

		QList<QLineEdit*> m_variableNames;
		QList<QLabel*> m_variableLabels;
		QList<TreeViewComboBox*> m_variableDataColumns;
		QList<QToolButton*> m_variableDeleteButtons;

		QPushButton* m_okButton;
	private slots:
		void generate();
		void checkValues();
		void showConstants();
		void showFunctions();
		void insertFunction(const QString&);
		void insertConstant(const QString&);
		void addVariable();
		void deleteVariable();
		void variableNameChanged();
};

#endif
