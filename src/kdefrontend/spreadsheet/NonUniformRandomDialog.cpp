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

	ui.cbDistribution->addItem(i18n("Gaussian"));
	//TODO
// 	ui.cbDistribution->addItem(i18n("Gaussian Tail"));
// 	ui.cbDistribution->addItem(i18n("Exponential"));
// 	ui.cbDistribution->addItem(i18n("Laplace"));
// 	ui.cbDistribution->addItem(i18n("Exponential Power"));
// 	ui.cbDistribution->addItem(i18n("Cauchy"));
// 	ui.cbDistribution->addItem(i18n("Rayleigh"));
// 	ui.cbDistribution->addItem(i18n("Rayleigh Tail"));
// 	ui.cbDistribution->addItem(i18n("Landau"));
// 	ui.cbDistribution->addItem(i18n("Levy alpha-stable"));
// 	ui.cbDistribution->addItem(i18n("Levy skew alpha-stable"));

	//Gaussian distribution as default
	this->distributionChanged(0);

	connect( ui.cbDistribution, SIGNAL(currentIndexChanged(int)), SLOT(distributionChanged(int)) );
	connect(this, SIGNAL(okClicked()), this, SLOT(generate()));

	resize( QSize(400,0).expandedTo(minimumSize()) );
}

void NonUniformRandomDialog::setColumns(QList<Column*> list) {
	m_columns = list;
}

void NonUniformRandomDialog::distributionChanged(int index) {
	if (index == 0) {
		ui.lParameter1->setText(QString::fromUtf8("μ"));
		ui.lParameter2->setText(QString::fromUtf8("σ"));
	} else if (index == 1) {

	}
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
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		double mu = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter1->text().toDouble();
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
