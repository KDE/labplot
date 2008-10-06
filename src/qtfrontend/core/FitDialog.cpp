/***************************************************************************
    File                 : FitDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Fit Wizard

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
#include "FitDialog.h"
#include "MyParser.h"
#include "ApplicationWindow.h"
#include "lib/ColorBox.h"
#include "Fit.h"
#include "analysis/MultiPeakFit.h"
#include "analysis/ExponentialFit.h"
#include "analysis/PolynomialFit.h"
#include "analysis/PluginFit.h"
#include "analysis/UserFunctionFit.h"
#include "analysis/SigmoidalFit.h"
#include "matrix/Matrix.h"
#include "table/Table.h"

#include <muParserError.h>

#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QWidget>
#include <QMessageBox>
#include <QComboBox>
#include <QWidgetList>
#include <QRadioButton>
#include <QFileDialog>
#include <QGroupBox>
#include <QLibrary>
#include <QLocale>
#include <stdio.h>

#define CONFS(string) QString::number(QLocale().toDouble(string),'g',boxPrecision->value())

FitDialog::FitDialog( QWidget* parent, Qt::WFlags fl )
: QDialog( parent, fl )
{
	setWindowTitle(tr("Fit Wizard"));
	setSizeGripEnabled( true );

    m_user_function_names = QStringList();
    m_user_functions = QStringList();
    m_user_function_params = QStringList();

	m_fitter = 0;

	tw = new QStackedWidget();

	initEditPage();
	initFitPage();
	initAdvancedPage();

	QVBoxLayout* vl = new QVBoxLayout();
	vl->addWidget(tw);
    setLayout(vl);
    resize(minimumSize());

	setBuiltInFunctionNames();
	setBuiltInFunctions();

	categoryBox->setCurrentRow (2);
	funcBox->setCurrentRow (0);

	loadPlugins();
}

void FitDialog::initFitPage()
{
    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("Curve")), 0, 0);
	boxCurve = new QComboBox();
    gl1->addWidget(boxCurve, 0, 1);
    gl1->addWidget(new QLabel(tr("Function")), 1, 0);
	lblFunction = new QLabel();
    gl1->addWidget(lblFunction, 1, 1);
	boxFunction = new QTextEdit();
	boxFunction->setReadOnly(true);
    QPalette palette = boxFunction->palette();
    palette.setColor(QPalette::Active, QPalette::Base, Qt::lightGray);
    boxFunction->setPalette(palette);
	boxFunction->setMaximumHeight(50);
    gl1->addWidget(boxFunction, 2, 1);
	gl1->addWidget(new QLabel( tr("Initial guesses")), 3, 0 );

	boxParams = new QTableWidget();
    boxParams->setColumnCount(3);
    boxParams->horizontalHeader()->setClickable(false);
    boxParams->horizontalHeader()->setResizeMode (0, QHeaderView::ResizeToContents);
    boxParams->horizontalHeader()->setResizeMode (1, QHeaderView::Stretch);
    boxParams->horizontalHeader()->setResizeMode (2, QHeaderView::ResizeToContents);
    QStringList header = QStringList() << tr("Parameter") << tr("Value") << tr("Constant");
    boxParams->setHorizontalHeaderLabels(header);
    boxParams->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    boxParams->verticalHeader()->hide();
    gl1->addWidget(boxParams, 3, 1);

	gl1->addWidget(new QLabel( tr("Algorithm")), 4, 0 );
	boxAlgorithm = new QComboBox();
	boxAlgorithm->addItem(tr("Scaled Levenberg-Marquardt"));
	boxAlgorithm->addItem(tr("Unscaled Levenberg-Marquardt"));
	boxAlgorithm->addItem(tr("Nelder-Mead Simplex"));
    gl1->addWidget(boxAlgorithm, 4, 1);

	gl1->addWidget(new QLabel( tr("Color")), 5, 0);
	boxColor = new ColorBox( false );
	boxColor->setColor(QColor(Qt::red));
    gl1->addWidget(boxColor, 5, 1);

    QGroupBox *gb1 = new QGroupBox();
    gb1->setLayout(gl1);

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr("From x=")), 0, 0);
	boxFrom = new QLineEdit();
    gl2->addWidget(boxFrom, 0, 1);
	gl2->addWidget(new QLabel( tr("To x=")), 1, 0);
	boxTo = new QLineEdit();
    gl2->addWidget(boxTo, 1, 1);
    QGroupBox *gb2 = new QGroupBox();
    gb2->setLayout(gl2);

    QGridLayout *gl3 = new QGridLayout();
    gl3->addWidget(new QLabel(tr("Iterations")), 0, 0);
	boxPoints = new QSpinBox();
    boxPoints->setRange(10, 10000);
	boxPoints->setSingleStep(50);
	boxPoints->setValue(1000);
    gl3->addWidget(boxPoints, 0, 1);
	gl3->addWidget(new QLabel( tr("Tolerance")), 1, 0);
	boxTolerance = new QLineEdit("1e-4");
	gl3->addWidget(boxTolerance, 1, 1);
    QGroupBox *gb3 = new QGroupBox();
    gb3->setLayout(gl3);

    QHBoxLayout *hbox1 = new QHBoxLayout();
    hbox1->addWidget(gb2);
    hbox1->addWidget(gb3);

    QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addWidget(new QLabel(tr( "Weighting Method" )));
	boxWeighting = new QComboBox();
	boxWeighting->addItem(tr("No weighting"));
	boxWeighting->addItem(tr("Instrumental"));
	boxWeighting->addItem(tr("Statistical"));
	boxWeighting->addItem(tr("Arbitrary Dataset"));
    hbox2->addWidget(boxWeighting);
    QGroupBox *gb4 = new QGroupBox();
    gb4->setLayout(hbox2);

	tableNamesBox = new QComboBox();
	tableNamesBox->setEnabled(false);
    hbox2->addWidget(tableNamesBox);
	colNamesBox = new QComboBox();
	colNamesBox->setEnabled(false);
    hbox2->addWidget(colNamesBox);

    QHBoxLayout *hbox3 = new QHBoxLayout();
	buttonEdit = new QPushButton(tr( "<< &Edit function" ) );
    hbox3->addWidget(buttonEdit);
	btnDeleteFitCurves = new QPushButton(tr( "&Delete Fit Curves" ));
    hbox3->addWidget(btnDeleteFitCurves);
	buttonOk = new QPushButton(tr( "&Fit" ) );
	buttonOk->setDefault( true );
    hbox3->addWidget(buttonOk);
	buttonCancel1 = new QPushButton(tr( "&Close" ));
    hbox3->addWidget(buttonCancel1);
	buttonAdvanced = new QPushButton(tr( "Custom &Output >>" ));
    hbox3->addWidget(buttonAdvanced);
    hbox3->addStretch();

    QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addWidget(gb1);
    vbox1->addLayout(hbox1);
    vbox1->addWidget(gb4);
    vbox1->addLayout(hbox3);

    fitPage = new QWidget();
    fitPage->setLayout(vbox1);
    tw->addWidget(fitPage);

	connect( boxCurve, SIGNAL( activated(const QString&) ), this, SLOT( activateCurve(const QString&) ) );
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT(accept()));
	connect( buttonCancel1, SIGNAL( clicked() ), this, SLOT(close()));
	connect( buttonEdit, SIGNAL( clicked() ), this, SLOT(showEditPage()));
	connect( btnDeleteFitCurves, SIGNAL( clicked() ), this, SLOT(deleteFitCurves()));
	connect( boxWeighting, SIGNAL( activated(int) ), this, SLOT( enableWeightingParameters(int) ) );
	connect( buttonAdvanced, SIGNAL(clicked()), this, SLOT(showAdvancedPage() ) );
    connect( tableNamesBox, SIGNAL( activated(int) ), this, SLOT( selectSrcTable(int) ) );

	setFocusProxy(boxFunction);
}

void FitDialog::initEditPage()
{
    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("Category")), 0, 0);
    gl1->addWidget(new QLabel(tr("Function")), 0, 1);
    gl1->addWidget(new QLabel(tr("Expression")), 0, 2);

	categoryBox = new QListWidget();
	categoryBox->addItem(tr("User defined"));
	categoryBox->addItem(tr("Built-in"));
	categoryBox->addItem(tr("Basic"));
	categoryBox->addItem(tr("Plugins"));

    gl1->addWidget(categoryBox, 1, 0);
	funcBox = new QListWidget();
    gl1->addWidget(funcBox, 1, 1);
	explainBox = new QTextEdit();
	explainBox->setReadOnly(true);
    gl1->addWidget(explainBox, 1, 2);

	boxUseBuiltIn = new QCheckBox();
	boxUseBuiltIn->setText(tr("Fit with &built-in function"));
	boxUseBuiltIn->hide();

    QHBoxLayout *hbox1 = new QHBoxLayout();
	hbox1->addWidget(boxUseBuiltIn);
    hbox1->addStretch();

	polynomOrderLabel = new QLabel( tr("Polynomial Order"));
	polynomOrderLabel->hide();
    hbox1->addWidget(polynomOrderLabel);

	polynomOrderBox = new QSpinBox();
    polynomOrderBox->setMinimum(1);
	polynomOrderBox->setValue(2);
	polynomOrderBox->hide();
	connect(polynomOrderBox, SIGNAL(valueChanged(int)), this, SLOT(showExpression(int)));
    hbox1->addWidget(polynomOrderBox);

	buttonPlugins = new QPushButton(tr( "&Choose plugins folder..." ) );
    hbox1->addWidget(buttonPlugins);
	buttonPlugins->hide();

    buttonClearUsrList = new QPushButton(tr( "Clear user &list" ) );
    hbox1->addWidget(buttonClearUsrList);
	buttonClearUsrList->hide();

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr("Name")), 0, 0);
	boxName = new QLineEdit(tr("user1"));
    gl2->addWidget(boxName, 0, 1);
	btnAddFunc = new QPushButton(tr( "&Save" ));
    gl2->addWidget(btnAddFunc, 0, 2);
    gl2->addWidget(new QLabel(tr("Parameters")), 1, 0);
	boxParam = new QLineEdit("a, b");
    gl2->addWidget(boxParam, 1, 1);
	btnDelFunc = new QPushButton( tr( "&Remove" ));
    gl2->addWidget(btnDelFunc, 1, 2);

    QGroupBox *gb = new QGroupBox();
    gb->setLayout(gl2);

	editBox = new QTextEdit();
	editBox->setTextFormat(Qt::PlainText);
	editBox->setFocus();

    QVBoxLayout *vbox1 = new QVBoxLayout();
	btnAddTxt = new QPushButton(tr( "Add &expression" ) );
    vbox1->addWidget(btnAddTxt);
	btnAddName = new QPushButton(tr( "Add &name" ));
    vbox1->addWidget(btnAddName);
	buttonClear = new QPushButton(tr( "Rese&t" ));
    vbox1->addWidget(buttonClear);
	buttonCancel2 = new QPushButton(tr( "&Close" ));
    vbox1->addWidget(buttonCancel2);
    btnContinue = new QPushButton(tr( "&Fit >>" ));
    vbox1->addWidget(btnContinue);
    vbox1->addStretch();

    QHBoxLayout *hbox2 = new QHBoxLayout();
	hbox2->addWidget(editBox);
    hbox2->addLayout(vbox1);

    QVBoxLayout *vbox2 = new QVBoxLayout();
    vbox2->addLayout(gl1);
    vbox2->addLayout(hbox1);
    vbox2->addWidget(gb);
    vbox2->addLayout(hbox2);

    editPage = new QWidget();
    editPage->setLayout(vbox2);
    tw->addWidget(editPage);

	connect( buttonPlugins, SIGNAL( clicked() ), this, SLOT(choosePluginsFolder()));
    connect( buttonClear, SIGNAL( clicked() ), this, SLOT(resetFunction()));
	connect( buttonClearUsrList, SIGNAL( clicked() ), this, SLOT(clearUserList()));
	connect( categoryBox, SIGNAL(currentRowChanged (int)), this, SLOT(showFunctionsList(int) ) );
	connect( funcBox, SIGNAL(currentRowChanged(int)), this, SLOT(showExpression(int)));
	connect( boxUseBuiltIn, SIGNAL(toggled(bool)), this, SLOT(setFunction(bool) ) );
	connect( btnAddName, SIGNAL(clicked()), this, SLOT(pasteFunctionName() ) );
	connect( btnAddTxt, SIGNAL(clicked()), this, SLOT(pasteExpression() ) );
	connect( btnContinue, SIGNAL(clicked()), this, SLOT(showFitPage() ) );
	connect( btnAddFunc, SIGNAL(clicked()), this, SLOT(saveUserFunction()));
	connect( btnDelFunc, SIGNAL(clicked()), this, SLOT(removeUserFunction()));
	connect( buttonCancel2, SIGNAL(clicked()), this, SLOT(close()) );
}


void FitDialog::initAdvancedPage()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();

	generatePointsBtn = new QRadioButton (tr("&Uniform X Function"));
	generatePointsBtn->setChecked(app->generateUniformFitPoints);
	connect( generatePointsBtn, SIGNAL(clicked()), this, SLOT(enableApplyChanges()));


    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(generatePointsBtn, 0, 0);

	lblPoints = new QLabel( tr("Points"));

	generatePointsBox = new QSpinBox ();
    generatePointsBox->setRange(0, 1000000);
	generatePointsBox->setSingleStep(10);
	generatePointsBox->setValue(app->fitPoints);
	connect( generatePointsBox, SIGNAL(valueChanged(int)), this, SLOT(enableApplyChanges(int)));
    showPointsBox(!app->generateUniformFitPoints);

    QHBoxLayout *hb = new QHBoxLayout();
    hb->addStretch();
    hb->addWidget(lblPoints);
    hb->addWidget(generatePointsBox);
	gl1->addLayout(hb, 0, 1);

	samePointsBtn = new QRadioButton(tr( "Same X as Fitting &Data" ));
    gl1->addWidget(samePointsBtn, 1, 0);
	samePointsBtn->setChecked(!app->generateUniformFitPoints);
	connect( samePointsBtn, SIGNAL(clicked()), this, SLOT(enableApplyChanges()));

    QGroupBox *gb1 = new QGroupBox(tr("Generated Fit Curve"));
    gb1->setLayout(gl1);

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel( tr("Significant Digits")), 0, 1);
	boxPrecision = new QSpinBox ();
    boxPrecision->setRange(0, 15);
	boxPrecision->setValue (app->fit_output_precision);
	connect( boxPrecision, SIGNAL(valueChanged (int)), this, SLOT(enableApplyChanges(int)));
    gl2->addWidget(boxPrecision, 0, 2);
	btnParamTable = new QPushButton(tr( "Parameters &Table" ));
    gl2->addWidget(btnParamTable, 1, 0);
	gl2->addWidget(new QLabel( tr("Name: ")), 1, 1);
	paramTableName = new QLineEdit(tr( "Parameters" ));
    gl2->addWidget(paramTableName, 1, 2);
	btnCovMatrix = new QPushButton(tr( "Covariance &Matrix" ));
    gl2->addWidget(btnCovMatrix, 2, 0);
    gl2->addWidget(new QLabel( tr("Name: ")), 2, 1);
	covMatrixName = new QLineEdit( tr( "CovMatrix" ) );
    gl2->addWidget(covMatrixName, 2, 2);

	scaleErrorsBox = new QCheckBox(tr("Scale Errors with sqrt(Chi^2/doF)"));
	scaleErrorsBox->setChecked(app->fit_scale_errors);
	connect( scaleErrorsBox, SIGNAL(stateChanged (int)), this, SLOT(enableApplyChanges(int)));

    QGroupBox *gb2 = new QGroupBox(tr("Parameters Output"));
    gb2->setLayout(gl2);

	logBox = new QCheckBox (tr("&Write Parameters to Result Log"));
	logBox->setChecked(app->writeFitResultsToLog);
	connect( logBox, SIGNAL(stateChanged(int)), this, SLOT(enableApplyChanges(int)));

	plotLabelBox = new QCheckBox (tr("&Paste Parameters to Plot"));
	plotLabelBox->setChecked(app->pasteFitResultsToPlot);
	connect( plotLabelBox, SIGNAL(stateChanged (int)), this, SLOT(enableApplyChanges(int)));

    QHBoxLayout *hbox1 = new QHBoxLayout();

	btnBack = new QPushButton(tr( "<< &Fit" ));
	connect( btnBack, SIGNAL(clicked()), this, SLOT(showFitPage()));
	connect( btnBack, SIGNAL(clicked()), this, SLOT(applyChanges()));
    hbox1->addWidget(btnBack);

	btnApply = new QPushButton(tr( "&Apply" ));
	btnApply->setEnabled(false);
	connect( btnApply, SIGNAL(clicked()), this, SLOT(applyChanges()));
    hbox1->addWidget(btnApply);

	buttonCancel3 = new QPushButton(tr( "&Close" ));
    hbox1->addWidget(buttonCancel3);
    hbox1->addStretch();

    QVBoxLayout *vbox1 = new QVBoxLayout();
    vbox1->addWidget(gb1);
    vbox1->addWidget(gb2);
	vbox1->addWidget(scaleErrorsBox);
    vbox1->addWidget(logBox);
    vbox1->addWidget(plotLabelBox);
    vbox1->addStretch();
    vbox1->addLayout(hbox1);

    advancedPage = new QWidget();
	advancedPage->setLayout(vbox1);
    tw->addWidget(advancedPage);

	connect(btnParamTable, SIGNAL(clicked()), this, SLOT(showParametersTable()));
	connect(btnCovMatrix, SIGNAL(clicked()), this, SLOT(showCovarianceMatrix()));
	connect(samePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
	connect(generatePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
	connect(buttonCancel3, SIGNAL(clicked()), this, SLOT(close()));
}

void FitDialog::applyChanges()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	app->fit_output_precision = boxPrecision->value();
	app->pasteFitResultsToPlot = plotLabelBox->isChecked();
	app->writeFitResultsToLog = logBox->isChecked();
	app->fitPoints = generatePointsBox->value();
	app->generateUniformFitPoints = generatePointsBtn->isChecked();
	app->fit_scale_errors = scaleErrorsBox->isChecked();
	app->saveSettings();
	btnApply->setEnabled(false);
}

void FitDialog::showParametersTable()
{
	QString tableName = paramTableName->text();
	if (tableName.isEmpty())
	{
		QMessageBox::critical(this, tr("Error"),
				tr("Please enter a valid name for the parameters table."));
		return;
	}

	if (!m_fitter)
	{
		QMessageBox::critical(this, tr("Error"),
				tr("Please perform a fit first and try again."));
		return;
	}

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	tableName = app->generateUniqueName(tableName, false);
	m_fitter->parametersTable(tableName);
}

void FitDialog::showCovarianceMatrix()
{
	QString matrixName = covMatrixName->text();
	if (matrixName.isEmpty())
	{
		QMessageBox::critical(this, tr("Error"),
				tr("Please enter a valid name for the covariance matrix."));
		return;
	}

	if (!m_fitter)
	{
		QMessageBox::critical(this, tr("Error"),
				tr("Please perform a fit first and try again."));
		return;
	}

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	matrixName = app->generateUniqueName(matrixName, false);
	m_fitter->covarianceMatrix(matrixName);
}

void FitDialog::showPointsBox(bool)
{
	if (generatePointsBtn->isChecked())
	{
		lblPoints->show();
		generatePointsBox->show();
	}
	else
	{
		lblPoints->hide();
		generatePointsBox->hide();
	}
}

void FitDialog::setLayer(Layer *g)
{
	if (!g)
		return;

	m_layer = g;
	boxCurve->clear();
	boxCurve->addItems(m_layer->analysableCurvesList());

    QString selectedCurve = g->selectedCurveTitle();
	if (!selectedCurve.isEmpty())
	{
	    int index = boxCurve->findText (selectedCurve);
		boxCurve->setCurrentItem(index);
	}
    activateCurve(boxCurve->currentText());

	connect (m_layer, SIGNAL(closed()), this, SLOT(close()));
	connect (m_layer, SIGNAL(dataRangeChanged()), this, SLOT(changeDataRange()));
};

void FitDialog::activateCurve(const QString& curveName)
{
	QwtPlotCurve *c = m_layer->curve(curveName);
	if (!c)
		return;

	double start, end;
    m_layer->range(m_layer->curveIndex(curveName), &start, &end);
    boxFrom->setText(QLocale().toString(QMIN(start, end), 'g', 15));
    boxTo->setText(QLocale().toString(QMAX(start, end), 'g', 15));
};

void FitDialog::saveUserFunction()
{
	if (editBox->text().isEmpty())
	{
		QMessageBox::critical(this, tr("Input function error"), tr("Please enter a valid function!"));
		editBox->setFocus();
		return;
	}
	else if (boxName->text().isEmpty())
	{
		QMessageBox::critical(this, tr("Input function error"),
				tr("Please enter a function name!"));
		boxName->setFocus();
		return;
	}
	else if (boxParam->text().remove(QRegExp("[,;\\s]")).isEmpty())
	{
		QMessageBox::critical(this, tr("Input function error"),
				tr("Please enter at least one parameter name!"));
		boxParam->setFocus();
		return;
	}

	if (m_built_in_function_names.contains(boxName->text()))
	{
		QMessageBox::critical(this, tr("Error: function name"),
				"<p><b>" + boxName->text() + "</b>" + tr(" is a built-in function name"
					"<p>You must choose another name for your function!"));
		editBox->setFocus();
		return;
	}
	if (editBox->text().contains(boxName->text()))
	{
		QMessageBox::critical(this, tr("Input function error"),
				tr("You can't define functions recursevely!"));
		editBox->setFocus();
		return;
	}

	QString name = boxName->text();
	QString f = name + "(x, " + boxParam->text() + ")=" + editBox->text().remove("\n");

	if (m_user_function_names.contains(name))
	{
		int index = m_user_function_names.indexOf(name);
		m_user_functions[index] = f;
		m_user_function_params[index] = boxParam->text();

		if (funcBox->currentItem()->text() == name)
			showExpression(index);
	}
	else
	{
		m_user_function_names << name;
		m_user_functions << f;
		m_user_function_params << boxParam->text();

		if (categoryBox->currentRow() == 0)
		{
			funcBox->addItem(name);
			funcBox->setCurrentRow (funcBox->count()-1);
		}

		if (m_user_function_names.count()>0 && !boxUseBuiltIn->isEnabled() && categoryBox->currentRow() == 0)
			boxUseBuiltIn->setEnabled(true);
	}
    buttonClearUsrList->setEnabled(true);
	emit saveFunctionsList(m_user_functions);
}

void FitDialog::removeUserFunction()
{
	QString name = funcBox->currentItem()->text();
	if (m_user_function_names.contains(name))
	{
		explainBox->setText(QString());

		int index = m_user_function_names.indexOf(name);
		m_user_function_names.remove(name);

		QString f = m_user_functions[index];
		m_user_functions.remove(f);

		f = m_user_function_params[index];
		m_user_function_params.remove(f);

		funcBox->clear();
		funcBox->addItems (m_user_function_names);
		funcBox->setCurrentRow (0);

		if (!m_user_function_names.count())
            {
			boxUseBuiltIn->setEnabled(false);
            buttonClearUsrList->setEnabled(false);
            }

		emit saveFunctionsList(m_user_functions);
	}
}

void FitDialog::showFitPage()
{
    int param_table_rows = boxParams->rowCount();

	QString par = boxParam->text().simplified();
	QStringList paramList = par.split(QRegExp("[,;]+[\\s]*"), QString::SkipEmptyParts);
	int parameters = paramList.count();
	boxParams->setRowCount(parameters);
    boxParams->hideColumn(2);

	if (parameters > 7)
		parameters = 7;
	boxParams->setMinimumHeight(4+(parameters+1)*boxParams->horizontalHeader()->height());

    for (int i = param_table_rows; i<paramList.count(); i++)
	{
        QTableWidgetItem *it = new QTableWidgetItem(paramList[i]);
        it->setFlags(!Qt::ItemIsEditable);
        it->setBackground(QBrush(Qt::lightGray));
        it->setForeground(QBrush(Qt::darkRed));
        QFont font = it->font();
        font.setBold(true);
        it->setFont(font);
        boxParams->setItem(i, 0, it);

        it = new QTableWidgetItem(QLocale().toString(1.0, 'f', boxPrecision->value()));
        it->setTextAlignment(Qt::AlignRight);
        boxParams->setItem(i, 1, it);
	}
    for (int i = 0; i<paramList.count(); i++)
        boxParams->item (i, 0)->setText(paramList[i]);

	// FIXME: this check is pretty ugly, should be changed to a more elegant way some time
	if (!boxUseBuiltIn->isChecked() ||
		(boxUseBuiltIn->isChecked()&& categoryBox->currentRow()!=3 && categoryBox->currentRow()!=1))
	{
        boxParams->showColumn(2);

		for (int i = 0; i<boxParams->rowCount(); i++ )
		{
            QTableWidgetItem *it = new QTableWidgetItem();
            it->setFlags(!Qt::ItemIsEditable);
            it->setBackground(QBrush(Qt::lightGray));
            boxParams->setItem(i, 2, it);

			QCheckBox *cb = new QCheckBox();
            boxParams->setCellWidget(i, 2, cb);
		}
	}

	boxFunction->setText(editBox->text().simplified());
	lblFunction->setText(boxName->text() +" (x, " + par + ")");

	tw->setCurrentWidget (fitPage);
}

void FitDialog::showEditPage()
{
	tw->setCurrentWidget (editPage);
}

void FitDialog::showAdvancedPage()
{
	tw->setCurrentWidget (advancedPage);
}

void FitDialog::setFunction(bool ok)
{
	editBox->setEnabled(!ok);
	boxParam->setEnabled(!ok);
	boxName->setEnabled(!ok);
	btnAddFunc->setEnabled(!ok);
	btnAddName->setEnabled(!ok);
	btnAddTxt->setEnabled(!ok);
	buttonClear->setEnabled(!ok);

	if (ok)
	{
		boxName->setText(funcBox->currentItem()->text());
		editBox->setText(explainBox->text());

		if (categoryBox->currentRow() == 0 && m_user_function_params.size() > 0)
			boxParam->setText(m_user_function_params[funcBox->currentRow ()]);
		else if (categoryBox->currentRow() == 1)
		{
			QStringList lst;
			switch(funcBox->currentRow ())
			{
				case 0:
					lst << "A1" << "A2" << "x0" << "dx";
					break;
				case 1:
					lst << "A" << "t" << "y0";
					break;
				case 2:
					lst << "A" << "t" << "y0";
					break;
				case 3:
					lst << "A1" << "t1" << "A2" << "t2" << "y0";
					break;
				case 4:
					lst << "A1" << "t1" << "A2" << "t2" << "A3" << "t3" << "y0";
					break;
				case 5:
					lst << "y0" << "A" << "xc" << "w";
					break;
				case 6:
					lst = MultiPeakFit::generateParameterList(polynomOrderBox->value());
					break;
				case 7:
					lst = MultiPeakFit::generateParameterList(polynomOrderBox->value());
					break;
				case 8:
					lst = PolynomialFit::generateParameterList(polynomOrderBox->value());
					break;
			}
			boxParam->setText(lst.join(", "));
		}
		else if (categoryBox->currentRow() == 3 && m_plugin_params.size() > 0 )
			boxParam->setText(m_plugin_params[funcBox->currentRow()]);
	}
}

void FitDialog::clearUserFunctions()
{
	m_user_functions.clear();
	m_user_function_names.clear();
	if (categoryBox->currentRow() == 0)
	{
		funcBox->clear();
		explainBox->clear();
        boxUseBuiltIn->setEnabled(false);
        buttonClearUsrList->setEnabled(false);
	}
	emit clearFunctionsList();
}

void FitDialog::addUserFunctions(const QStringList& list)
{
	if (!list.count())
	{
		boxUseBuiltIn->setEnabled(false);
		return;
	}

	for (int i = 0; i<list.count(); i++)
	{
		QString s = list.at(i).simplified();
		if (!s.isEmpty())
		{
            m_user_functions << s;

            int pos1 = s.find("(", 0);
            m_user_function_names << s.left(pos1);

            int pos2 = s.find(")", pos1);
            m_user_function_params << s.mid(pos1+4, pos2-pos1-4);
		}
	}
}

void FitDialog::showFunctionsList(int category)
{
	boxUseBuiltIn->setChecked(false);
	boxUseBuiltIn->setEnabled(false);
	boxUseBuiltIn->hide();
	buttonPlugins->hide();
    buttonClearUsrList->hide();
    buttonClearUsrList->setEnabled(false);
	btnDelFunc->setEnabled(false);
    funcBox->blockSignals(true);
	funcBox->clear();
    explainBox->clear();
	polynomOrderLabel->hide();
	polynomOrderBox->hide();

	switch (category)
	{
		case 0:
			if (m_user_function_names.count() > 0)
			{
				showUserFunctions();
                buttonClearUsrList->show();
				boxUseBuiltIn->setEnabled(true);
                buttonClearUsrList->setEnabled(true);
			}

			boxUseBuiltIn->setText(tr("Fit with selected &user function"));
			boxUseBuiltIn->show();
            buttonClearUsrList->show();
			btnDelFunc->setEnabled(true);
			break;

		case 1:
			boxUseBuiltIn->setText(tr("Fit using &built-in function"));
			boxUseBuiltIn->show();
			boxUseBuiltIn->setEnabled(true);
			funcBox->addItems(m_built_in_function_names);
			break;

		case 2:
			showParseFunctions();
			break;

		case 3:
			buttonPlugins->show();
			boxUseBuiltIn->setText(tr("Fit using &plugin function"));
			boxUseBuiltIn->show();
			if (m_plugin_function_names.size() > 0)
			{
				funcBox->addItems(m_plugin_function_names);
				boxUseBuiltIn->setEnabled(true);
			}
			break;
	}
    funcBox->blockSignals(false);
	funcBox->setCurrentRow (0);
}

void FitDialog::choosePluginsFolder()
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QString dir = QFileDialog::getExistingDirectory(QDir::currentDirPath(), this, "get directory",
			tr("Choose the plugins folder"), true, true);
	if (!dir.isEmpty())
	{
		m_plugin_files_list.clear();
		m_plugin_function_names.clear();
		m_plugin_functions.clear();
		m_plugin_params.clear();
		funcBox->clear();
		explainBox->clear();

		app->fitPluginsPath = dir;
		loadPlugins();
		if (m_plugin_function_names.size() > 0)
		{
			funcBox->addItems(m_plugin_function_names);
			if (!boxUseBuiltIn->isEnabled())
				boxUseBuiltIn->setEnabled(true);

			funcBox->setCurrentRow(0);
		}
		else
			boxUseBuiltIn->setEnabled(false);
	}
}

void FitDialog::loadPlugins()
{
	typedef char* (*fitFunc)();

	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	QString path = app->fitPluginsPath + "/";
	QDir dir(path);
	QStringList lst = dir.entryList(QDir::Files|QDir::NoSymLinks);

	for (int i=0; i<lst.count(); i++)
	{
		QLibrary lib(path + lst[i]);

		fitFunc name = (fitFunc) lib.resolve( "name" );
		fitFunc function = (fitFunc) lib.resolve("function");
		fitFunc params = (fitFunc) lib.resolve("parameters");

		if ( name && function && params )
		{
			m_plugin_function_names << QString(name());
			m_plugin_functions << QString(function());
			m_plugin_params << QString(params());
			m_plugin_files_list << lib.library();
		}
	}
}

void FitDialog::showUserFunctions()
{
	funcBox->addItems(m_user_function_names);
}

void FitDialog::setBuiltInFunctionNames()
{
	m_built_in_function_names << "Boltzmann" << "ExpGrowth" << "ExpDecay1" << "ExpDecay2" << "ExpDecay3"
		<< "GaussAmp" << "Gauss" << "Lorentz" << "Polynomial";
}

void FitDialog::setBuiltInFunctions()
{
	m_built_in_functions << "(A1-A2)/(1+exp((x-x0)/dx))+A2";
	m_built_in_functions << "y0+A*exp(x/t)";
	m_built_in_functions << "y0+A*exp(-x/t)";
	m_built_in_functions << "y0+A1*exp(-x/t1)+A2*exp(-x/t2)";
	m_built_in_functions << "y0+A1*exp(-x/t1)+A2*exp(-x/t2)+A3*exp(-x/t3)";
	m_built_in_functions << "y0+A*exp(-(x-xc)*(x-xc)/(2*w*w))";
}

void FitDialog::showParseFunctions()
{
	funcBox->addItems(MyParser::functionsList());
}

void FitDialog::showExpression(int function)
{
    if (function < 0)
        return;

	if (categoryBox->currentRow() == 2)
	{
		explainBox->setText(MyParser::explainFunction(function));
	}
	else if (categoryBox->currentRow() == 1)
	{
		polynomOrderLabel->show();
		polynomOrderBox->show();

		if (funcBox->currentItem()->text() == tr("Gauss"))
		{
			polynomOrderLabel->setText(tr("Peaks"));
			explainBox->setText(MultiPeakFit::generateFormula(polynomOrderBox->value(), MultiPeakFit::Gauss));
		}
		else if (funcBox->currentItem()->text() == tr("Lorentz"))
		{
			polynomOrderLabel->setText(tr("Peaks"));
			explainBox->setText(MultiPeakFit::generateFormula(polynomOrderBox->value(), MultiPeakFit::Lorentz));
		}
		else if (funcBox->currentItem()->text() == tr("Polynomial"))
		{
			polynomOrderLabel->setText(tr("Polynomial Order"));
			explainBox->setText(PolynomialFit::generateFormula(polynomOrderBox->value()));
		}
		else
		{
			polynomOrderLabel->hide();
			polynomOrderBox->hide();
			polynomOrderBox->setValue(1);
			explainBox->setText(m_built_in_functions[function]);
		}
		setFunction(boxUseBuiltIn->isChecked());
	}
	else if (categoryBox->currentRow() == 0)
	{
		if (m_user_functions.size() > function) {
			QStringList l = m_user_functions[function].split("=");
			explainBox->setText(l[1]);
		} else
			explainBox->clear();
		setFunction(boxUseBuiltIn->isChecked());
	}
	else if (categoryBox->currentRow() == 3)
	{
		if (m_plugin_functions.size() > 0)
		{
			explainBox->setText(m_plugin_functions[function]);
			setFunction(boxUseBuiltIn->isChecked());
		}
		else
			explainBox->clear();
	}
}

void FitDialog::pasteExpression()
{
	QString f = explainBox->text();
	if (categoryBox->currentRow() == 2)
	{//basic parser function
		f = f.left(f.find("(", 0)+1);
		if (editBox->hasSelectedText())
		{
			QString markedText=editBox->selectedText();
			editBox->insert(f+markedText+")");
		}
		else
			editBox->insert(f+")");
	}
	else
		editBox->insert(f);

	editBox->setFocus();
}

void FitDialog::pasteFunctionName()
{
	editBox->insert(funcBox->currentItem()->text());
	editBox->setFocus();
}

void FitDialog::accept()
{
	QString curve = boxCurve->currentText();
	QStringList curvesList = m_layer->curvesList();
	if (curvesList.contains(curve) <= 0)
	{
		QMessageBox::critical(this,tr("Warning"),
				tr("The curve <b> %1 </b> doesn't exist anymore! Operation aborted!").arg(curve));
		boxCurve->clear();
		boxCurve->addItems(curvesList);
		return;
	}

	if (!validInitialValues())
		return;

	QString from=boxFrom->text().toLower();
	QString to=boxTo->text().toLower();
	QString tolerance=boxTolerance->text().toLower();
	double start, end, eps;
	try
	{
		MyParser parser;
		parser.SetExpr(CONFS(from).toAscii().constData());
		start=parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("Start limit error"),QString::fromStdString(e.GetMsg()));
		boxFrom->setFocus();
		return;
	}

	try
	{
		MyParser parser;
		parser.SetExpr(CONFS(to).toAscii().constData());
		end=parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(this, tr("End limit error"),QString::fromStdString(e.GetMsg()));
		boxTo->setFocus();
		return;
	}

	if (start>=end)
	{
		QMessageBox::critical(0, tr("Input error"),
				tr("Please enter x limits that satisfy: from < end!"));
		boxTo->setFocus();
		return;
	}

	try
	{
		MyParser parser;
		parser.SetExpr(CONFS(tolerance).toAscii().constData());
		eps=parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QMessageBox::critical(0, tr("Tolerance input error"),QString::fromStdString(e.GetMsg()));
		boxTolerance->setFocus();
		return;
	}

	if (eps<0 || eps>=1)
	{
		QMessageBox::critical(0, tr("Tolerance input error"),
				tr("The tolerance value must be positive and less than 1!"));
		boxTolerance->setFocus();
		return;
	}

	int i, n=0, rows=boxParams->rowCount();
	if (!boxParams->isColumnHidden(2))
	{
		for (i=0;i<rows;i++)
		{//count the non-constant parameters
            QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 2);
			if (!cb->isChecked())
				n++;
		}
	}
	else
		n=rows;

	QStringList parameters;
	MyParser parser;
	bool error=FALSE;

	double *paramsInit = new double[n];
	QString formula = boxFunction->text();
	try
	{
		bool withNames = containsUserFunctionName(formula);
		while(withNames)
		{
			for (i=0; i<m_user_function_names.count(); i++)
			{
				if (formula.contains(m_user_function_names[i]))
				{
					QStringList l = m_user_functions[i].split("=");
					formula.replace(m_user_function_names[i], "(" + l[1] + ")");
				}
			}
			withNames = containsUserFunctionName(formula);
		}

		for (i=0; i<m_built_in_function_names.count(); i++)
		{
			if (formula.contains(m_built_in_function_names[i]))
				formula.replace(m_built_in_function_names[i], "(" + m_built_in_functions[i] + ")");
		}

		if (!boxParams->isColumnHidden(2))
		{
			int j = 0;
			for (i=0;i<rows;i++)
			{
                QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 2);
				if (!cb->isChecked())
				{
					paramsInit[j] = QLocale().toDouble(boxParams->item(i,1)->text());
					parser.DefineVar(boxParams->item(i,0)->text().toAscii().constData(), &paramsInit[j]);
					parameters << boxParams->item(i,0)->text();
					j++;
				}
				else
					formula.replace(boxParams->item(i,0)->text(), 
						CONFS(boxParams->item(i,1)->text()) );
			}
		}
		else
		{
			for (i=0;i<n;i++)
			{
				paramsInit[i] = QLocale().toDouble(boxParams->item(i,1)->text());
				parser.DefineVar(boxParams->item(i,0)->text().toAscii().constData(), &paramsInit[i]);
				parameters << boxParams->item(i,0)->text();
			}
		}

		parser.SetExpr(formula.toAscii().constData());
		double x=start;
		parser.DefineVar("x", &x);
		parser.Eval();
	}
	catch(mu::ParserError &e)
	{
		QString errorMsg = boxFunction->text() + " = " + formula + "\n" + QString::fromStdString(e.GetMsg()) + "\n";
		
#if 0 // muParser 1.30 does not use ecUNEXPECTED_COMMA anymore
		if(e.GetCode() == ecUNEXPECTED_COMMA)
			errorMsg += tr("You have to use a dot as decimal separator in formulas.");
		else
#endif
			errorMsg += tr("Please verify that you have initialized all the parameters!");

		QMessageBox::critical(0, tr("Input function error"), errorMsg);
		boxFunction->setFocus();
		error = true;
	}

	if (!error)
	{
		ApplicationWindow *app = (ApplicationWindow *)this->parent();

		if (m_fitter)
		{
			delete m_fitter;
			m_fitter  = 0;
		}

		if (boxUseBuiltIn->isChecked() && categoryBox->currentRow() == 1)
			fitBuiltInFunction(funcBox->currentItem()->text(), paramsInit);
		else if (boxUseBuiltIn->isChecked() && categoryBox->currentRow() == 3)
		{
			m_fitter = new PluginFit(app, m_layer);
			if (!((PluginFit*)m_fitter)->load(m_plugin_files_list[funcBox->currentRow()])){
				m_fitter  = 0;
				return;}
				m_fitter->setInitialGuesses(paramsInit);
		}
		else
		{
			m_fitter = new UserFunctionFit(app, m_layer);
			((UserFunctionFit*)m_fitter)->setParametersList(parameters);
			((UserFunctionFit*)m_fitter)->setFormula(formula);
			m_fitter->setInitialGuesses(paramsInit);
		}
		delete[] paramsInit;
				
		if (!m_fitter->setDataFromCurve(curve, start, end) ||
			!m_fitter->setWeightingData ((Fit::WeightingMethod)boxWeighting->currentIndex(),
					       tableNamesBox->currentText()+"_"+colNamesBox->currentText()))
		{
			delete m_fitter;
			m_fitter  = 0;
			return;
		}
				
		m_fitter->setTolerance (eps);
		m_fitter->setAlgorithm((Fit::Algorithm)boxAlgorithm->currentIndex());
		m_fitter->setColor(boxColor->currentIndex());
		m_fitter->generateFunction(generatePointsBtn->isChecked(), generatePointsBox->value());
		m_fitter->setMaximumIterations(boxPoints->value());
		m_fitter->scaleErrors(scaleErrorsBox->isChecked());

		if (m_fitter->name() == tr("MultiPeak") && ((MultiPeakFit *)m_fitter)->peaks() > 1)
		{
			((MultiPeakFit *)m_fitter)->enablePeakCurves(app->generatePeakCurves);
			((MultiPeakFit *)m_fitter)->setPeakCurvesColor(app->peakCurvesColor);
		}

		m_fitter->fit();
		double *res = m_fitter->results();
		if (!boxParams->isColumnHidden(2))
		{
			int j = 0;
			for (i=0;i<rows;i++)
			{
                QCheckBox *cb = (QCheckBox*)boxParams->cellWidget(i, 2);
				if (!cb->isChecked())
					boxParams->item(i, 1)->setText(QLocale().toString(res[j++], 'g', boxPrecision->value()));
			}
		}
		else
		{
			for (i=0;i<rows;i++)
				boxParams->item(i, 1)->setText(QLocale().toString(res[i], 'g', boxPrecision->value()));
		}
	}
}

void FitDialog::fitBuiltInFunction(const QString& function, double* initVal)
{
	ApplicationWindow *app = (ApplicationWindow *)this->parent();
	if (function == "ExpDecay1")
	{
		initVal[1] = 1/initVal[1];
		m_fitter = new ExponentialFit(app, m_layer);
	}
	else if (function == "ExpGrowth")
	{
		initVal[1] = -1/initVal[1];
		m_fitter = new ExponentialFit(app, m_layer, true);
	}
	else if (function == "ExpDecay2")
	{
		initVal[1] = 1/initVal[1];
		initVal[3] = 1/initVal[3];
		m_fitter = new TwoExpFit(app, m_layer);
	}
	else if (function == "ExpDecay3")
	{
		initVal[1] = 1/initVal[1];
		initVal[3] = 1/initVal[3];
		initVal[5] = 1/initVal[5];
		m_fitter = new ThreeExpFit(app, m_layer);
	}
	else if (function == "Boltzmann")
		m_fitter = new SigmoidalFit(app, m_layer);
	else if (function == "GaussAmp")
		m_fitter = new GaussAmpFit(app, m_layer);
	else if (function == "Gauss")
		m_fitter = new MultiPeakFit(app, m_layer, MultiPeakFit::Gauss, polynomOrderBox->value());
	else if (function == "Lorentz")
		m_fitter = new MultiPeakFit(app, m_layer, MultiPeakFit::Lorentz, polynomOrderBox->value());
	else if (function == tr("Polynomial"))
		m_fitter = new PolynomialFit(app, m_layer, polynomOrderBox->value());

	if (function != tr("Polynomial"))
		m_fitter->setInitialGuesses(initVal);
}

bool FitDialog::containsUserFunctionName(const QString& function)
{
	foreach(QString fn, m_user_function_names)
		if (!fn.isEmpty() && function.contains(fn))
			return true;

	return false;
}

bool FitDialog::validInitialValues()
{
	for (int i=0; i<boxParams->rowCount(); i++)
	{
		if(boxParams->item(i,1)->text().isEmpty())
		{
			QMessageBox::critical(0, tr("Input error"),
					tr("Please enter initial guesses for your parameters!"));
			boxParams->setCurrentCell (i,1);
			return false;
		}

		try
		{
			MyParser parser;
			parser.SetExpr(CONFS(boxParams->item(i,1)->text()).toAscii().constData());
			parser.Eval();
		}
		catch (mu::ParserError &e)
		{
			QMessageBox::critical(0, tr("Start limit error"), QString::fromStdString(e.GetMsg()));
			boxParams->setCurrentCell (i,1);
			return false;
		}
	}
	return true;
}

void FitDialog::changeDataRange()
{
	double start = m_layer->selectedXStartValue();
	double end = m_layer->selectedXEndValue();
	boxFrom->setText(QString::number(QMIN(start, end), 'g', 15));
	boxTo->setText(QString::number(QMAX(start, end), 'g', 15));
}

void FitDialog::setSrcTables(QWidgetList* tables)
{
	if (tables->isEmpty())
	{
		tableNamesBox->addItem(tr("No data tables"));
		colNamesBox->addItem(tr("No data tables"));
		return;
	}
	
	m_src_table = tables;
	tableNamesBox->clear();
	foreach(QWidget *i, *m_src_table)
		tableNamesBox->addItem(i->name());

	tableNamesBox->setCurrentIndex(tableNamesBox->findText(boxCurve->currentText().split("_", QString::SkipEmptyParts)[0]));
	selectSrcTable(tableNamesBox->currentIndex());
}

void FitDialog::selectSrcTable(int tabnr)
{
	colNamesBox->clear();
	
	if (tabnr >= 0 && tabnr < m_src_table->count())
	{
		Table *t = (Table*)m_src_table->at(tabnr);
		if (t)
			colNamesBox->addItems(t->colNames());
	}
}

void FitDialog::enableWeightingParameters(int index)
{
	if (index == Fit::Dataset)
	{
		tableNamesBox->setEnabled(true);
		colNamesBox->setEnabled(true);
	}
	else
	{
		tableNamesBox->setEnabled(false);
		colNamesBox->setEnabled(false);
	}
}

void FitDialog::closeEvent (QCloseEvent * e )
{
	if(m_fitter && plotLabelBox->isChecked())
		m_fitter->showLegend();

	e->accept();
}

void FitDialog::enableApplyChanges(int)
{
	btnApply->setEnabled(true);
}

void FitDialog::deleteFitCurves()
{
	m_layer->deleteFitCurves();
	boxCurve->clear();
	boxCurve->addItems(m_layer->curvesList());
}

void FitDialog::resetFunction()
{
	boxName->clear();
	boxParam->clear();
	editBox->clear();
}

