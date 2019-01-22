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
#include "backend/gsl/ExpressionParser.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
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

	m_topLevelClasses<<"Folder"<<"Workbook"<<"Spreadsheet"<<"FileDataSource"<<"Column";
	m_selectableClasses<<"Column";

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

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.verticalLayout->addWidget(btnBox);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox, &QDialogButtonBox::accepted, this, &FunctionValuesDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &FunctionValuesDialog::reject);
	m_okButton->setText(i18n("&Generate"));
	m_okButton->setToolTip(i18n("Generate function values"));

	connect( ui.bAddVariable, SIGNAL(pressed()), this, SLOT(addVariable()) );
	connect( ui.teEquation, SIGNAL(expressionChanged()), this, SLOT(checkValues()) );
	connect( ui.tbConstants, SIGNAL(clicked()), this, SLOT(showConstants()) );
	connect( ui.tbFunctions, SIGNAL(clicked()), this, SLOT(showFunctions()) );
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
	ui.teEquation->setPlainText(m_columns.first()->formula());

	const QStringList& variableNames = m_columns.first()->formulaVariableNames();
	if (!variableNames.size()) {
		//no formula was used for this column -> add the first variable "x"
		addVariable();
		m_variableNames[0]->setText("x");
	} else {
		//formula and variables are available
		const QStringList& columnPathes = m_columns.first()->formulaVariableColumnPathes();

		//add all available variables and select the corresponding columns
		const QVector<AbstractAspect*> cols = m_spreadsheet->project()->children("Column", AbstractAspect::Recursive);
		for (int i = 0; i < variableNames.size(); ++i) {
			addVariable();
			m_variableNames[i]->setText(variableNames.at(i));

			for (const auto* aspect : cols) {
				if (aspect->path() == columnPathes.at(i)) {
					const auto* column = dynamic_cast<const AbstractColumn*>(aspect);
					if (column)
						m_variableDataColumns[i]->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(column));
					else
						m_variableDataColumns[i]->setCurrentModelIndex(QModelIndex());

					break;
				}
			}
		}
	}

	checkValues();
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
	connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insertConstant(QString)));
	connect(&constants, SIGNAL(constantSelected(QString)), &menu, SLOT(close()));
	connect(&constants, SIGNAL(canceled()), &menu, SLOT(close()));

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+ui.tbConstants->width(),-menu.sizeHint().height());
	menu.exec(ui.tbConstants->mapToGlobal(pos));
}

void FunctionValuesDialog::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, SIGNAL(functionSelected(QString)), this, SLOT(insertFunction(QString)));
	connect(&functions, SIGNAL(functionSelected(QString)), &menu, SLOT(close()));
	connect(&functions, SIGNAL(canceled()), &menu, SLOT(close()));

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
	auto* layout = dynamic_cast<QGridLayout*>(ui.frameVariables->layout());
	int row = m_variableNames.size();

	//text field for the variable name
	auto* le = new QLineEdit();
	le->setMaximumWidth(30);
	connect(le, SIGNAL(textChanged(QString)), this, SLOT(variableNameChanged()));
	layout->addWidget(le, row, 0, 1, 1);
	m_variableNames << le;

	//label for the "="-sign
	auto* l = new QLabel("=");
	layout->addWidget(l, row, 1, 1, 1);
	m_variableLabels << l;

	//combo box for the data column
	auto* cb = new TreeViewComboBox();
	cb->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	connect( cb, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(checkValues()) );
	layout->addWidget(cb, row, 2, 1, 1);
	m_variableDataColumns << cb;

	cb->setTopLevelClasses(m_topLevelClasses);
	cb->setModel(m_aspectTreeModel.get());
	cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(m_spreadsheet->column(0)));

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
		connect(b, SIGNAL(pressed()), this, SLOT(deleteVariable()));
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
}

void FunctionValuesDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: fill column with function values",
					"%1: fill columns with function values",
					m_spreadsheet->name(), m_columns.size()));

	//determine variable names and the data vectors of the specified columns
	QStringList variableNames;
	QStringList columnPathes;
	QVector<QVector<double>*> xVectors;
	QVector<QVector<double>*> xNewVectors;
	int maxRowCount = m_spreadsheet->rowCount();
	for (int i = 0; i < m_variableNames.size(); ++i) {
		variableNames << m_variableNames.at(i)->text().simplified();

		AbstractAspect* aspect = static_cast<AbstractAspect*>(m_variableDataColumns.at(i)->currentModelIndex().internalPointer());
		Q_ASSERT(aspect);
		auto* column = dynamic_cast<Column*>(aspect);
		Q_ASSERT(column);
		columnPathes << column->path();
		if (column->columnMode() == AbstractColumn::Integer) {
			//convert integers to doubles first
			auto* xVector = new QVector<double>(column->rowCount());
			for (int i = 0; i<column->rowCount(); ++i)
				xVector->operator[](i) = column->valueAt(i);

			xNewVectors << xVector;
			xVectors << xVector;
		} else
			xVectors << static_cast<QVector<double>* >(column->data());

		if (column->rowCount() > maxRowCount)
			maxRowCount = column->rowCount();
	}

	//resize the spreadsheet if one of the data vectors from other spreadsheet(s) has more elements then the current spreadsheet.
	if (m_spreadsheet->rowCount() < maxRowCount)
		m_spreadsheet->setRowCount(maxRowCount);

	//create new vector for storing the calculated values
	//the vectors with the variable data can be smaller then the result vector. So, not all values in the result vector might get initialized.
	//->"clean" the result vector first
	QVector<double> new_data(maxRowCount);
	for (auto& d : new_data)
		d = NAN;

	//evaluate the expression for f(x_1, x_2, ...) and write the calculated values into a new vector.
	ExpressionParser* parser = ExpressionParser::getInstance();
	const QString& expression = ui.teEquation->toPlainText();
	parser->evaluateCartesian(expression, variableNames, xVectors, &new_data);

	//set the new values and store the expression, variable names and the used data columns
	for (auto* col : m_columns) {
		if (col->columnMode() != AbstractColumn::Numeric)
			col->setColumnMode(AbstractColumn::Numeric);

		col->setFormula(expression, variableNames, columnPathes);
		col->replaceValues(0, new_data);
	}

	m_spreadsheet->endMacro();

	//delete help vectors created for the conversion from int to double
	for (auto* vector : xNewVectors)
		delete vector;

	RESET_CURSOR;
}
