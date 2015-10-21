/***************************************************************************
    File                 : MatrixFunctionDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating matrix values from a mathematical function
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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
#include "MatrixFunctionDialog.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/gsl/parser_extern.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"

#include <QMenu>
#include <QWidgetAction>


/*!
	\class MatrixFunctionDialog
	\brief Dialog for generating matrix values from a mathematical function.

	\ingroup kdefrontend
 */

MatrixFunctionDialog::MatrixFunctionDialog(Matrix* m, QWidget* parent, Qt::WFlags fl) : KDialog(parent, fl), m_matrix(m) {
	Q_ASSERT(m_matrix);
	setWindowTitle(i18n("Function values"));

	QFrame* mainWidget = new QFrame(this);
	ui.setupUi(mainWidget);
	setMainWidget( mainWidget );

	ui.tbConstants->setIcon( KIcon("labplot-format-text-symbol") );
	ui.tbFunctions->setIcon( KIcon("preferences-desktop-font") );

	QStringList vars;
	vars<<"x"<<"y";
	ui.teEquation->setVariables(vars);
	ui.teEquation->setFocus();

	QString info = "[" + QString::number(m_matrix->xStart()) + ", " + QString::number(m_matrix->xEnd()) + "], " + QString::number(m_matrix->columnCount()) + " " + i18n("values");
	ui.lXInfo->setText(info);
	info = "[" + QString::number(m_matrix->yStart()) + ", " + QString::number(m_matrix->yEnd()) + "], " + QString::number(m_matrix->rowCount()) + " " + i18n("values");
	ui.lYInfo->setText(info);

	ui.teEquation->setPlainText(m_matrix->formula());

	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonText(KDialog::Ok, i18n("&Generate"));
	setButtonToolTip(KDialog::Ok, i18n("Generate function values"));

	connect( ui.teEquation, SIGNAL(expressionChanged()), this, SLOT(checkValues()) );
	connect( ui.tbConstants, SIGNAL(clicked()), this, SLOT(showConstants()) );
	connect( ui.tbFunctions, SIGNAL(clicked()), this, SLOT(showFunctions()) );
	connect(this, SIGNAL(okClicked()), this, SLOT(generate()));

	resize( QSize(300,0).expandedTo(minimumSize()) );
}

void MatrixFunctionDialog::checkValues() {
	if (!ui.teEquation->isValid()) {
		enableButton(KDialog::Ok, false);
		return;
	}

	enableButton(KDialog::Ok, true);
}

void MatrixFunctionDialog::showConstants() {
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

void MatrixFunctionDialog::showFunctions() {
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

void MatrixFunctionDialog::insertFunction(const QString& str) {
	ui.teEquation->insertPlainText(str + "(x)");
}

void MatrixFunctionDialog::insertConstant(const QString& str) {
	ui.teEquation->insertPlainText(str);
}

void MatrixFunctionDialog::generate() {
	WAIT_CURSOR;

	m_matrix->beginMacro(i18n("%1: fill matrix with function values", m_matrix->name()) );


	QVector<QVector<double> > new_data = m_matrix->data();

	QByteArray funcba = ui.teEquation->toPlainText().toLocal8Bit();
	char* func = funcba.data();
	char varX[] = "x";
	char varY[] = "y";

	double diff = m_matrix->xEnd() - m_matrix->xStart();
	double xStep = 0.0;
	if (m_matrix->columnCount() > 1)
		xStep = diff/double(m_matrix->columnCount()-1);

	diff = m_matrix->yEnd() - m_matrix->yStart();
	double yStep = 0.0;
	if (m_matrix->rowCount() > 1)
		yStep = diff/double(m_matrix->rowCount()-1);

	double x = m_matrix->xStart();
	double y = m_matrix->yStart();
	for (int col=0; col<m_matrix->columnCount(); ++col) {
		for (int row=0; row<m_matrix->rowCount(); row++) {
			assign_variable(varX, x);
			assign_variable(varY, y);
			double z = parse(func);
			new_data[col][row] = z;
			y += yStep;
		}
		y = m_matrix->yStart();
		x += xStep;
	}

	m_matrix->setFormula(ui.teEquation->toPlainText());
	m_matrix->setData(new_data);

	m_matrix->endMacro();
	RESET_CURSOR;
}
