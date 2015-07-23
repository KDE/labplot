/***************************************************************************
    File                 : FunctionValuesDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating values from a mathematical function
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)

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

#include <math.h>

/*!
	\class FunctionValuesDialog
	\brief Dialog for generating values from a mathematical function.

	\ingroup kdefrontend
 */

FunctionValuesDialog::FunctionValuesDialog(Spreadsheet* s, QWidget* parent, Qt::WFlags fl) : KDialog(parent, fl), m_spreadsheet(s) {

	setWindowTitle(i18n("Function values"));

	QFrame* mainWidget = new QFrame(this);
	ui.setupUi(mainWidget);
	setMainWidget( mainWidget );

	cbXDataColumn = new TreeViewComboBox(mainWidget);
	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(mainWidget->layout());
	Q_ASSERT(gridLayout);
	gridLayout->addWidget(cbXDataColumn, 0, 2, 1, 1);

	ui.tbConstants->setIcon( KIcon("format-text-symbol") );
	ui.tbFunctions->setIcon( KIcon("preferences-desktop-font") );

	QStringList vars;
	vars<<"x";
	ui.teEquation->setVariables(vars);

	ui.teEquation->setFocus();

	if (m_spreadsheet) {
		m_aspectTreeModel = std::auto_ptr<AspectTreeModel>(new AspectTreeModel(m_spreadsheet->project()));

		QList<const char *>  list;
		list<<"Folder"<<"Workbook"<<"Spreadsheet"<<"FileDataSource"<<"Column";
		cbXDataColumn->setTopLevelClasses(list);

		list.clear();
		list<<"Column";
		m_aspectTreeModel->setSelectableAspects(list);
		cbXDataColumn->setSelectableClasses(list);
		cbXDataColumn->setModel(m_aspectTreeModel.get());

		//select the first available column in the spreadsheet
		cbXDataColumn->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(m_spreadsheet->column(0)));
	}

	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonText(KDialog::Ok, i18n("&Generate"));
	setButtonToolTip(KDialog::Ok, i18n("Generate function values"));

	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(checkValues()) );
	connect( ui.teEquation, SIGNAL(expressionChanged()), this, SLOT(checkValues()) );
	connect( ui.tbConstants, SIGNAL(clicked()), this, SLOT(showConstants()) );
	connect( ui.tbFunctions, SIGNAL(clicked()), this, SLOT(showFunctions()) );
	connect(this, SIGNAL(okClicked()), this, SLOT(generate()));

	resize( QSize(300,0).expandedTo(minimumSize()) );
}

void FunctionValuesDialog::setColumns(QList<Column*> list) {
	m_columns = list;
	ui.teEquation->setPlainText(m_columns.first()->formula());
}

void FunctionValuesDialog::checkValues() {
	if (!ui.teEquation->isValid()) {
		enableButton(KDialog::Ok, false);
		return;
	}

	AbstractAspect* aspect = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	if (!aspect) {
		enableButton(KDialog::Ok, false);
		return;
	}

	enableButton(KDialog::Ok, true);
}

void FunctionValuesDialog::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);
	connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insertConstant(QString)));
	connect(&constants, SIGNAL(constantSelected(QString)), &menu, SLOT(close()));
	connect(&constants, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
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

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+ui.tbFunctions->width(),-menu.sizeHint().height());
	menu.exec(ui.tbFunctions->mapToGlobal(pos));
}

void FunctionValuesDialog::insertFunction(const QString& str) {
	ui.teEquation->insertPlainText(str + "(x)");
}

void FunctionValuesDialog::insertConstant(const QString& str) {
	ui.teEquation->insertPlainText(str);
}

void FunctionValuesDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: fill column with function values",
									"%1: fill columns with function values",
									m_spreadsheet->name(),
									m_columns.size()));


	AbstractAspect* aspect = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	if (!aspect)
		return;

	Column* xColumn = dynamic_cast<Column*>(aspect);
	Q_ASSERT(xColumn);

	if (m_spreadsheet->rowCount()<xColumn->rowCount())
		m_spreadsheet->setRowCount(xColumn->rowCount());

	QVector<double>* xVector = static_cast<QVector<double>* >(xColumn->data());
	ExpressionParser* parser = ExpressionParser::getInstance();
	const QString& expression = ui.teEquation->toPlainText();

	QVector<double> new_data(m_spreadsheet->rowCount());
	for (int i=0; i<new_data.size(); ++i)
		new_data[i] = NAN;

	parser->evaluateCartesian(expression, xVector, &new_data);

	foreach(Column* col, m_columns) {
		col->setFormula(expression);
		col->replaceValues(0, new_data);
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
