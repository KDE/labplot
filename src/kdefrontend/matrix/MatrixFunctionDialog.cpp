/***************************************************************************
    File                 : MatrixFunctionDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating matrix values from a mathematical function
    ------------------------------------------------------------------------
    Copyright            : (C) 2015-2016 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"

extern "C" {
#include "backend/gsl/parser.h"
}
#include <cmath>

#include <QMenu>
#include <QWidgetAction>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KLocalizedString>
#include <QThreadPool>
#ifndef NDEBUG
#include <QElapsedTimer>
#endif

/*!
	\class MatrixFunctionDialog
	\brief Dialog for generating matrix values from a mathematical function.

	\ingroup kdefrontend
 */

MatrixFunctionDialog::MatrixFunctionDialog(Matrix* m, QWidget* parent) : QDialog(parent), m_matrix(m) {
	Q_ASSERT(m_matrix);
	setWindowTitle(i18nc("@title:window", "Function Values"));

	ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
	ui.tbConstants->setIcon( QIcon::fromTheme("labplot-format-text-symbol") );
	ui.tbFunctions->setIcon( QIcon::fromTheme("preferences-desktop-font") );

	QStringList vars;
	vars << "x" << "y";
	ui.teEquation->setVariables(vars);
	ui.teEquation->setFocus();
	ui.teEquation->setMaximumHeight(QLineEdit().sizeHint().height()*2);

	QString info = '[' + QString::number(m_matrix->xStart()) + ", " + QString::number(m_matrix->xEnd()) + "], " + i18np("%1 value", "%1 values", m_matrix->columnCount());
	ui.lXInfo->setText(info);
	info = '[' + QString::number(m_matrix->yStart()) + ", " + QString::number(m_matrix->yEnd()) + "], " + i18np("%1 value", "%1 values", m_matrix->rowCount());
	ui.lYInfo->setText(info);

	ui.teEquation->setPlainText(m_matrix->formula());
	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.gridLayout_2->addWidget(btnBox);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &MatrixFunctionDialog::close);
	connect(btnBox, &QDialogButtonBox::accepted, this, &MatrixFunctionDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &MatrixFunctionDialog::reject);

	m_okButton->setText(i18n("&Generate"));
	m_okButton->setToolTip(i18n("Generate function values"));

	connect(ui.teEquation, &ExpressionTextEdit::expressionChanged, this, &MatrixFunctionDialog::checkValues);
	connect(ui.tbConstants, &QToolButton::clicked, this, &MatrixFunctionDialog::showConstants);
	connect(ui.tbFunctions, &QToolButton::clicked, this, &MatrixFunctionDialog::showFunctions);
	connect(m_okButton, &QPushButton::clicked, this, &MatrixFunctionDialog::generate);

	resize(QSize(300,0).expandedTo(minimumSize()));
}

void MatrixFunctionDialog::checkValues() {
	if (!ui.teEquation->isValid()) {
		m_okButton->setEnabled(false);
		return;
	}

	m_okButton->setEnabled(true);
}

void MatrixFunctionDialog::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);
	connect(&constants, &ConstantsWidget::constantSelected, this, &MatrixFunctionDialog::insertConstant);
	connect(&constants, &ConstantsWidget::constantSelected, &menu, &QMenu::close);
	connect(&constants, &ConstantsWidget::canceled, &menu, &QMenu::close);

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+ui.tbConstants->width(),-menu.sizeHint().height());
	menu.exec(ui.tbConstants->mapToGlobal(pos));
}

void MatrixFunctionDialog::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, &FunctionsWidget::functionSelected, this, &MatrixFunctionDialog::insertFunction);
	connect(&functions, &FunctionsWidget::functionSelected, &menu, &QMenu::close);
	connect(&functions, &FunctionsWidget::canceled, &menu, &QMenu::close);

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+ui.tbFunctions->width(),-menu.sizeHint().height());
	menu.exec(ui.tbFunctions->mapToGlobal(pos));
}

void MatrixFunctionDialog::insertFunction(const QString& str) {
	//TODO: not all functions have only one argument
	ui.teEquation->insertPlainText(str + "(x)");
}

void MatrixFunctionDialog::insertConstant(const QString& str) {
	ui.teEquation->insertPlainText(str);
}

/* task class for parallel fill (not used) */
class GenerateValueTask : public QRunnable {
public:
	GenerateValueTask(int startCol, int endCol, QVector<QVector<double>>& matrixData, double xStart, double yStart,
		double xStep, double yStep, char* func): m_startCol(startCol), m_endCol(endCol), m_matrixData(matrixData),
		m_xStart(xStart), m_yStart(yStart), m_xStep(xStep), m_yStep(yStep), m_func(func) {
	};

	void run() override {
		const int rows = m_matrixData[m_startCol].size();
		double x = m_xStart;
		double y = m_yStart;
		DEBUG("FILL col"<<m_startCol<<"-"<<m_endCol<<" x/y ="<<x<<'/'<<y<<" steps ="<<m_xStep<<'/'<<m_yStep<<" rows ="<<rows);

		parser_var vars[] = { {"x", x}, {"y", y}};
		for (int col = m_startCol; col < m_endCol; ++col) {
			vars[0].value = x;
			for (int row = 0; row < rows; ++row) {
				vars[1].value = y;
				double z = parse_with_vars(m_func, vars, 2);
				//qDebug()<<" z ="<<z;
				m_matrixData[col][row] = z;
				y += m_yStep;
			}

			y = m_yStart;
			x += m_xStep;
		}
	}

private:
	int m_startCol;
	int m_endCol;
	QVector<QVector<double>>& m_matrixData;
	double m_xStart;
	double m_yStart;
	double m_xStep;
	double m_yStep;
	char* m_func;
};

void MatrixFunctionDialog::generate() {
	WAIT_CURSOR;

	m_matrix->beginMacro(i18n("%1: fill matrix with function values", m_matrix->name()));

	//TODO: data types
	QVector<QVector<double>>* new_data = static_cast<QVector<QVector<double>>*>(m_matrix->data());

	QByteArray funcba = ui.teEquation->toPlainText().toLocal8Bit();
	char* func = funcba.data();

	// check if rows or cols == 1
	double diff = m_matrix->xEnd() - m_matrix->xStart();
	double xStep = 0.0;
	if (m_matrix->columnCount() > 1)
		xStep = diff/double(m_matrix->columnCount() - 1);

	diff = m_matrix->yEnd() - m_matrix->yStart();
	double yStep = 0.0;
	if (m_matrix->rowCount() > 1)
		yStep = diff/double(m_matrix->rowCount() - 1);

#ifndef NDEBUG
	QElapsedTimer timer;
	timer.start();
#endif

	//TODO: too slow because every parser thread needs an own symbol_table
	// idea: use pool->maxThreadCount() symbol tables and reuse them?
/*	double yStart = m_matrix->yStart();
	const int cols = m_matrix->columnCount();
	QThreadPool* pool = QThreadPool::globalInstance();
	int range = ceil(double(cols)/pool->maxThreadCount());
	DEBUG("Starting" << pool->maxThreadCount() << "threads. cols =" << cols << ": range =" << range);

	for (int i = 0; i < pool->maxThreadCount(); ++i) {
		const int start = i*range;
		int end = (i+1)*range;
		if (end > cols) end = cols;
		qDebug() << "start/end: " << start << end;
		const double xStart = m_matrix->xStart() + xStep*start;
		GenerateValueTask* task = new GenerateValueTask(start, end, new_data, xStart, yStart, xStep, yStep, func);
		task->setAutoDelete(false);
		pool->start(task);
	}
	pool->waitForDone();
*/
	double x = 0, y = 0;
	parser_var vars[] = {{"x", x}, {"y", y}};
	for (int col = 0; col < m_matrix->columnCount(); ++col) {
		vars[0].value = x;
		for (int row = 0; row < m_matrix->rowCount(); ++row) {
			vars[1].value = y;
			(new_data->operator[](col))[row] = parse_with_vars(func, vars, 2);
			y += yStep;
		}
		y = m_matrix->yStart();
		x += xStep;
	}

	// Timing
	DEBUG("elapsed time =" << timer.elapsed() << "ms");

	m_matrix->setFormula(ui.teEquation->toPlainText());
	m_matrix->setData(new_data);

	m_matrix->endMacro();
	RESET_CURSOR;
}
