/***************************************************************************
    File                 : FunctionValuesDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating values from a mathematical function
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2018 by Alexander Semke (alexander.semke@web.de)

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
#include "FunctionValuesDialog.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/gsl/ExpressionParser.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"

#include <QMenu>
#include <QWidgetAction>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

#include <cmath>

/*!
	\class FunctionValuesDialog
	\brief Dialog for generating values from a mathematical function.

	\ingroup kdefrontend
 */
FunctionValuesDialog::FunctionValuesDialog(Spreadsheet* s, QWidget* parent) : QDialog(parent), m_spreadsheet(s) {
	Q_ASSERT(s != nullptr);
	setWindowTitle(i18nc("@title:window", "Function Values"));

	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	ui.tbConstants->setIcon( QIcon::fromTheme("labplot-format-text-symbol") );

	ui.tbConstants->setIcon( QIcon::fromTheme("format-text-symbol") );
	ui.tbFunctions->setIcon( QIcon::fromTheme("preferences-desktop-font") );

	ui.teEquation->setMaximumHeight(QLineEdit().sizeHint().height()*2);
	ui.teEquation->setFocus();

	m_topLevelClasses = {AspectType::Folder, AspectType::Workbook,
	                     AspectType::Spreadsheet, AspectType::Column
	                    };
	m_selectableClasses = {AspectType::Column};

// needed for buggy compiler
#if __cplusplus < 201103L
	m_aspectTreeModel = std::auto_ptr<AspectTreeModel>(new AspectTreeModel(m_spreadsheet->project()));
#else
	m_aspectTreeModel = std::unique_ptr<AspectTreeModel>(new AspectTreeModel(m_spreadsheet->project()));
#endif
	m_aspectTreeModel->setSelectableAspects(m_selectableClasses);
	m_aspectTreeModel->enableNumericColumnsOnly(true);
	m_aspectTreeModel->enableNonEmptyNumericColumnsOnly(true);

	ui.bAddVariable->setIcon(QIcon::fromTheme("list-add"));
	ui.bAddVariable->setToolTip(i18n("Add new variable"));

	ui.chkAutoUpdate->setToolTip(i18n("Automaticaly update the calculated values on changes in the variable columns"));

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.verticalLayout->addWidget(btnBox);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox, &QDialogButtonBox::accepted, this, &FunctionValuesDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &FunctionValuesDialog::reject);
	m_okButton->setText(i18n("&Generate"));
	m_okButton->setToolTip(i18n("Generate function values"));

	connect(ui.bAddVariable, &QPushButton::pressed, this, &FunctionValuesDialog::addVariable);
	connect(ui.teEquation, &ExpressionTextEdit::expressionChanged, this, &FunctionValuesDialog::checkValues);
	connect(ui.tbConstants, &QToolButton::clicked, this, &FunctionValuesDialog::showConstants);
	connect(ui.tbFunctions, &QToolButton::clicked, this, &FunctionValuesDialog::showFunctions);
	connect(m_okButton, &QPushButton::clicked, this, &FunctionValuesDialog::generate);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "FunctionValuesDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));
}

FunctionValuesDialog::~FunctionValuesDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "FunctionValuesDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void FunctionValuesDialog::setColumns(QVector<Column*> columns) {
	m_columns = columns;

	//formula expression
	ui.teEquation->setPlainText(m_columns.first()->formula());

	//variables
	const QStringList& variableNames = m_columns.first()->formulaVariableNames();
	if (!variableNames.size()) {
		//no formula was used for this column -> add the first variable "x"
		addVariable();
		m_variableNames[0]->setText("x");
	} else {
		//formula and variables are available
		const QVector<Column*>& variableColumns = m_columns.first()->formulaVariableColumns();
		const QStringList columnPaths = m_columns.first()->formulaVariableColumnPaths();

		//add all available variables and select the corresponding columns
		const QVector<AbstractAspect*> cols = m_spreadsheet->project()->children(AspectType::Column, AbstractAspect::Recursive);
		for (int i = 0; i < variableNames.size(); ++i) {
			addVariable();
			m_variableNames[i]->setText(variableNames.at(i));

			bool found = false;
			for (const auto* col : cols) {
				if (col != variableColumns.at(i))
					continue;

				const auto* column = dynamic_cast<const AbstractColumn*>(col);
				if (column)
					m_variableDataColumns[i]->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(column));
				else
					m_variableDataColumns[i]->setCurrentModelIndex(QModelIndex());

				m_variableDataColumns[i]->useCurrentIndexText(true);
				m_variableDataColumns[i]->setInvalid(false);

				found = true;
				break;
			}

			//for the current variable name no column is existing anymore (was deleted)
			//->highlight the combobox red
			if (!found) {
				m_variableDataColumns[i]->setCurrentModelIndex(QModelIndex());
				m_variableDataColumns[i]->useCurrentIndexText(false);
				m_variableDataColumns[i]->setInvalid(true, i18n("The column \"%1\"\nis not available anymore. It will be automatically used once it is created again.", columnPaths[i]));
				m_variableDataColumns[i]->setText(columnPaths[i].split('/').last());
			}
		}
	}

	//auto update
	ui.chkAutoUpdate->setChecked(m_columns.first()->formulaAutoUpdate());

	checkValues();
}

bool FunctionValuesDialog::validVariableName(QLineEdit* le) {
	if (ExpressionParser::getInstance()->constants().indexOf(le->text()) != -1) {
		le->setStyleSheet("QLineEdit{background: red;}");
		le->setToolTip(i18n("Provided variable name is already reserved for a name of a constant. Please use another name."));
		return false;
	}

	if (ExpressionParser::getInstance()->functions().indexOf(le->text()) != -1) {
		le->setStyleSheet("QLineEdit{background: red;}");
		le->setToolTip(i18n("Provided variable name is already reserved for a name of a function. Please use another name."));
		return false;
	}

	le->setStyleSheet(QString());
	le->setToolTip("");
	return true;
}

/*!
	check the user input and enables/disables the Ok-button depending on the correctness of the input
 */
void FunctionValuesDialog::checkValues() {
	//check whether the formula syntax is correct
	if (!ui.teEquation->isValid()) {
		m_okButton->setEnabled(false);
		return;
	}

	//check whether for the variables where a name was provided also a column was selected.
	for (int i = 0; i < m_variableDataColumns.size(); ++i) {
		if (m_variableNames.at(i)->text().simplified().isEmpty())
			continue;

		TreeViewComboBox* cb = m_variableDataColumns.at(i);
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cb->currentModelIndex().internalPointer());
		if (!aspect) {
			m_okButton->setEnabled(false);
			return;
		}

		if (!validVariableName(m_variableNames[i])) {
			m_okButton->setEnabled(false);
			return;
		}
/*		Column* column = dynamic_cast<Column*>(aspect);
		DEBUG("row count = " << (static_cast<QVector<double>* >(column->data()))->size());
		if (!column || column->rowCount() < 1) {
			m_okButton->setEnabled(false);
			//Warning: x column is empty
			return;
		}
*/
	}

	m_okButton->setEnabled(true);
}

void FunctionValuesDialog::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);
	connect(&constants, &ConstantsWidget::constantSelected, this, &FunctionValuesDialog::insertConstant);
	connect(&constants, &ConstantsWidget::constantSelected, &menu, &QMenu::close);
	connect(&constants, &ConstantsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+ui.tbConstants->width(),-menu.sizeHint().height());
	menu.exec(ui.tbConstants->mapToGlobal(pos));
}

void FunctionValuesDialog::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, &FunctionsWidget::functionSelected, this, &FunctionValuesDialog::insertFunction);
	connect(&functions, &FunctionsWidget::functionSelected, &menu, &QMenu::close);
	connect(&functions, &FunctionsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+ui.tbFunctions->width(),-menu.sizeHint().height());
	menu.exec(ui.tbFunctions->mapToGlobal(pos));
}

void FunctionValuesDialog::insertFunction(const QString& str) {
	//TODO: not all functions have only one argument
	ui.teEquation->insertPlainText(str + "(x)");
}

void FunctionValuesDialog::insertConstant(const QString& str) {
	ui.teEquation->insertPlainText(str);
}

void FunctionValuesDialog::addVariable() {
	auto* layout = ui.gridLayoutVariables;
	int row = m_variableNames.size();

	//text field for the variable name
	auto* le = new QLineEdit();
	le->setMaximumWidth(30);
	connect(le, &QLineEdit::textChanged, this, &FunctionValuesDialog::variableNameChanged);
	layout->addWidget(le, row, 0, 1, 1);
	m_variableNames << le;

	//label for the "="-sign
	auto* l = new QLabel("=");
	layout->addWidget(l, row, 1, 1, 1);
	m_variableLabels << l;

	//combo box for the data column
	auto* cb = new TreeViewComboBox();
	cb->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &FunctionValuesDialog::variableColumnChanged);
	layout->addWidget(cb, row, 2, 1, 1);
	m_variableDataColumns << cb;

	cb->setTopLevelClasses(m_topLevelClasses);
	cb->setModel(m_aspectTreeModel.get());

	//don't allow to select columns to be calculated as variable columns (avoid circular dependencies)
	QList<const AbstractAspect*> aspects;
	for (auto* col : m_columns)
		aspects << col;
	cb->setHiddenAspects(aspects);

	//for the variable column select the first non-selected column in the spreadsheet
	for (auto* col : m_spreadsheet->children<Column>()) {
		if (m_columns.indexOf(col) == -1) {
			cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(col));
			break;
		}
	}

	//move the add-button to the next row
	layout->removeWidget(ui.bAddVariable);
	layout->addWidget(ui.bAddVariable, row+1,3, 1, 1);

	//add delete-button for the just added variable
	if (row != 0) {
		auto* b = new QToolButton();
		b->setIcon(QIcon::fromTheme("list-remove"));
		b->setToolTip(i18n("Delete variable"));
		layout->addWidget(b, row, 3, 1, 1);
		m_variableDeleteButtons<<b;
		connect(b, &QToolButton::pressed, this, &FunctionValuesDialog::deleteVariable);
	}

	ui.lVariable->setText(i18n("Variables:"));

	//TODO: adjust the tab-ordering after new widgets were added
}

void FunctionValuesDialog::deleteVariable() {
	QObject* ob = QObject::sender();
	int index = m_variableDeleteButtons.indexOf(qobject_cast<QToolButton*>(ob)) ;

	delete m_variableNames.takeAt(index+1);
	delete m_variableLabels.takeAt(index+1);
	delete m_variableDataColumns.takeAt(index+1);
	delete m_variableDeleteButtons.takeAt(index);

	variableNameChanged();
	checkValues();

	//adjust the layout
	resize( QSize(width(),0).expandedTo(minimumSize()) );

	m_variableNames.size() > 1 ? ui.lVariable->setText(i18n("Variables:")) : ui.lVariable->setText(i18n("Variable:"));

	//TODO: adjust the tab-ordering after some widgets were deleted
}

void FunctionValuesDialog::variableNameChanged() {
	QStringList vars;
	QString text;
	for (auto* varName : m_variableNames) {
		QString name = varName->text().simplified();
		if (!name.isEmpty()) {
			vars << name;

			if (text.isEmpty()) {
				text += name;
			} else {
				text += ", " + name;
			}
		}
	}

	if (!text.isEmpty())
		text = "f(" + text + ") = ";
	else
		text = "f = ";

	ui.lFunction->setText(text);
	ui.teEquation->setVariables(vars);
	checkValues();
}

/*!
 * called if a new column was selected in the comboboxes for the variable columns.
 */
void FunctionValuesDialog::variableColumnChanged(const QModelIndex& index) {
	//combobox was potentially red-highlighted because of a missing column
	//remove the highlighting if we have a valid selection now
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	if (aspect) {
		auto* cb = dynamic_cast<TreeViewComboBox*>(QObject::sender());
		if (cb)
			cb->setStyleSheet("");
	}

	checkValues();
}

void FunctionValuesDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: fill column with function values",
					"%1: fill columns with function values",
					m_spreadsheet->name(), m_columns.size()));

	//determine variable names and the data vectors of the specified columns
	QStringList variableNames;
	QVector<Column*> variableColumns;
	for (int i = 0; i < m_variableNames.size(); ++i) {
		variableNames << m_variableNames.at(i)->text().simplified();

		AbstractAspect* aspect = static_cast<AbstractAspect*>(m_variableDataColumns.at(i)->currentModelIndex().internalPointer());
		Q_ASSERT(aspect);
		auto* column = dynamic_cast<Column*>(aspect);
		Q_ASSERT(column);
		variableColumns << column;
	}

	//set the new values and store the expression, variable names and the used data columns
	const QString& expression = ui.teEquation->toPlainText();
	bool autoUpdate = (ui.chkAutoUpdate->checkState() == Qt::Checked);
	for (auto* col : m_columns) {
		if (col->columnMode() != AbstractColumn::Numeric)
			col->setColumnMode(AbstractColumn::Numeric);

		col->setFormula(expression, variableNames, variableColumns, autoUpdate);
		col->updateFormula();
	}

	m_spreadsheet->endMacro();

	RESET_CURSOR;
}
