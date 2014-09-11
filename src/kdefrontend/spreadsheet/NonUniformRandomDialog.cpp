/***************************************************************************
    File                 : NonUniformRandomDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating non-uniformly distributed random numbers
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
#include "NonUniformRandomDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include <KLocale>

#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

/*!
	\class NonUniformRandomDialog
	\brief Dialog for generating non-uniform random numbers.

	\ingroup kdefrontend
 */

NonUniformRandomDialog::NonUniformRandomDialog(Spreadsheet* s, QWidget* parent, Qt::WFlags fl) : KDialog(parent, fl), m_spreadsheet(s) {

	setWindowTitle(i18n("Non-uniform random numbers"));
// 	setSizeGripEnabled(true);

	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	setMainWidget( mainWidget );

	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonText(KDialog::Ok, i18n("&Generate"));
	setButtonToolTip(KDialog::Ok, i18n("Generate random numbers according to the selected distribution"));

	ui.cbDistribution->addItem(i18n("Gaussian Distribution"));
	ui.cbDistribution->addItem(i18n("Gaussian Tail Distribution"));
	ui.cbDistribution->addItem(i18n("Exponential Distribution"));
	ui.cbDistribution->addItem(i18n("Laplace Distribution"));
	ui.cbDistribution->addItem(i18n("Exponential Power Distribution"));
	ui.cbDistribution->addItem(i18n("Cauchy Distribution"));
	ui.cbDistribution->addItem(i18n("Rayleigh Distribution"));
	ui.cbDistribution->addItem(i18n("Rayleigh Tail Distribution"));
	ui.cbDistribution->addItem(i18n("Landau Distribution"));
	ui.cbDistribution->addItem(i18n("Levy alpha-stable Distribution"));
	ui.cbDistribution->addItem(i18n("Levy skew alpha-stable Distribution"));
	ui.cbDistribution->addItem(i18n("Gamma Distribution"));
	ui.cbDistribution->addItem(i18n("Flat (Uniform) Distribution"));
	ui.cbDistribution->addItem(i18n("Lognormal Distribution"));
	ui.cbDistribution->addItem(i18n("Chi-squared Distribution"));
	ui.cbDistribution->addItem(i18n("F-distribution"));
	ui.cbDistribution->addItem(i18n("t-distribution"));
	ui.cbDistribution->addItem(i18n("Beta Distribution"));
	ui.cbDistribution->addItem(i18n("Logistic Distribution"));
	ui.cbDistribution->addItem(i18n("Pareto Distribution"));
	ui.cbDistribution->addItem(i18n("Spherical Vector Distributions"));
	ui.cbDistribution->addItem(i18n("Weibull Distribution"));
	ui.cbDistribution->addItem(i18n("Type-1 Gumbel Distribution"));
	ui.cbDistribution->addItem(i18n("Type-2 Gumbel Distribution"));
	ui.cbDistribution->addItem(i18n("Dirichlet Distribution"));
// 	ui.cbDistribution->addItem(i18n("General Discrete Distributions"));
	ui.cbDistribution->addItem(i18n("Poisson Distribution"));
	ui.cbDistribution->addItem(i18n("Bernoulli Distribution"));
	ui.cbDistribution->addItem(i18n("Binomial Distribution"));
	ui.cbDistribution->addItem(i18n("Multinomial Distribution"));
	ui.cbDistribution->addItem(i18n("Negative Binomial Distribution"));
	ui.cbDistribution->addItem(i18n("Pascal Distribution"));
	ui.cbDistribution->addItem(i18n("Geometric Distribution"));
	ui.cbDistribution->addItem(i18n("Hypergeometric Distribution"));
	ui.cbDistribution->addItem(i18n("Logarithmic Distribution"));

	ui.kleParameter1->setClearButtonShown(true);
	ui.kleParameter2->setClearButtonShown(true);
	ui.kleParameter3->setClearButtonShown(true);

	ui.kleParameter1->setValidator( new QDoubleValidator(ui.kleParameter1) );
	ui.kleParameter2->setValidator( new QDoubleValidator(ui.kleParameter2) );
	ui.kleParameter3->setValidator( new QDoubleValidator(ui.kleParameter3) );

	connect( ui.cbDistribution, SIGNAL(currentIndexChanged(int)), SLOT(distributionChanged(int)) );
	connect( ui.kleParameter1, SIGNAL(textChanged(QString)), this, SLOT(checkValues()) );
	connect( ui.kleParameter2, SIGNAL(textChanged(QString)), this, SLOT(checkValues()) );
	connect( ui.kleParameter3, SIGNAL(textChanged(QString)), this, SLOT(checkValues()) );
	connect(this, SIGNAL(okClicked()), this, SLOT(generate()));

	//Gaussian distribution as default
	this->distributionChanged(0);

	resize( QSize(400,0).expandedTo(minimumSize()) );
}

void NonUniformRandomDialog::setColumns(QList<Column*> list) {
	m_columns = list;
}

void NonUniformRandomDialog::distributionChanged(int index) {
	if (index == 0) {
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("μ"));
		ui.lParameter2->setText(QString::fromUtf8("σ"));
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
	} else if (index == 1) {

	}
}

void NonUniformRandomDialog::checkValues() {
	if (ui.kleParameter1->text().simplified().isEmpty()) {
		enableButton(KDialog::Ok, false);
		return;
	}

	if (ui.kleParameter2->isVisible() && ui.kleParameter2->text().simplified().isEmpty()) {
		enableButton(KDialog::Ok, false);
		return;
	}

	if (ui.kleParameter3->isVisible() && ui.kleParameter3->text().simplified().isEmpty()) {
		enableButton(KDialog::Ok, false);
		return;
	}

	enableButton(KDialog::Ok, true);
}

void NonUniformRandomDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	//create a generator chosen by the environment variable GSL_RNG_TYPE
	gsl_rng_env_setup();
	const gsl_rng_type* T = gsl_rng_default;
	gsl_rng* r = gsl_rng_alloc (T);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: fill column with non-uniform random numbers", m_spreadsheet->name()));

	int index = ui.cbDistribution->currentIndex();
	if (index == 0) {
		double mu = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_gaussian(r, sigma) + mu;
				col->setValueAt(i, value);
			}
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;

	gsl_rng_free (r);
}
