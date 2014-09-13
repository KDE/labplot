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

enum distribution {
	Gaussian, Exponential, Laplace, ExponentialPower, Cauchy, Rayleigh, RayleighTail, Landau,
	LevyAlphaStable, LevySkewAlphaStable, Gamma, Flat, Lognormal, ChiSquared, F, t, Beta,
	Logistic, Pareto, Weibull, Type1Gumbel, Type2Gumbel, Poisson, Bernoulli, Binomial,
	NegativeBinomial, Pascal, Geometric, Hypergeometric, Logarithmic
};

NonUniformRandomDialog::NonUniformRandomDialog(Spreadsheet* s, QWidget* parent, Qt::WFlags fl) : KDialog(parent, fl), m_spreadsheet(s) {

	setWindowTitle(i18n("Non-uniform random numbers"));

	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	setMainWidget( mainWidget );

	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonText(KDialog::Ok, i18n("&Generate"));
	setButtonToolTip(KDialog::Ok, i18n("Generate random numbers according to the selected distribution"));

	ui.cbDistribution->addItem(i18n("Gaussian Distribution"), Gaussian);
// 	ui.cbDistribution->addItem(i18n("Gaussian Tail Distribution"));
	ui.cbDistribution->addItem(i18n("Exponential Distribution"), Exponential);
	ui.cbDistribution->addItem(i18n("Laplace Distribution"), Laplace);
	ui.cbDistribution->addItem(i18n("Exponential Power Distribution"), ExponentialPower);
	ui.cbDistribution->addItem(i18n("Cauchy Distribution"), Cauchy);
	ui.cbDistribution->addItem(i18n("Rayleigh Distribution"), Rayleigh);
	ui.cbDistribution->addItem(i18n("Rayleigh Tail Distribution"), RayleighTail);
	ui.cbDistribution->addItem(i18n("Landau Distribution"), Landau);
	ui.cbDistribution->addItem(i18n("Levy alpha-stable Distribution"), LevyAlphaStable);
	ui.cbDistribution->addItem(i18n("Levy skew alpha-stable Distribution"), LevySkewAlphaStable);
	ui.cbDistribution->addItem(i18n("Gamma Distribution"), Gamma);
	ui.cbDistribution->addItem(i18n("Flat (Uniform) Distribution"), Flat);
	ui.cbDistribution->addItem(i18n("Lognormal Distribution"), Lognormal);
	ui.cbDistribution->addItem(i18n("Chi-squared Distribution"), ChiSquared);
	ui.cbDistribution->addItem(i18n("F-distribution"), F);
	ui.cbDistribution->addItem(i18n("t-distribution"), t);
	ui.cbDistribution->addItem(i18n("Beta Distribution"), Beta);
	ui.cbDistribution->addItem(i18n("Logistic Distribution"), Logistic);
	ui.cbDistribution->addItem(i18n("Pareto Distribution"), Pareto);
// 	ui.cbDistribution->addItem(i18n("Spherical Vector Distributions"));
	ui.cbDistribution->addItem(i18n("Weibull Distribution"), Weibull);
	ui.cbDistribution->addItem(i18n("Type-1 Gumbel Distribution"), Type1Gumbel);
	ui.cbDistribution->addItem(i18n("Type-2 Gumbel Distribution"), Type2Gumbel);
// 	ui.cbDistribution->addItem(i18n("Dirichlet Distribution"));
// 	ui.cbDistribution->addItem(i18n("General Discrete Distributions"));
	ui.cbDistribution->addItem(i18n("Poisson Distribution"), Poisson);
	ui.cbDistribution->addItem(i18n("Bernoulli Distribution"), Bernoulli);
	ui.cbDistribution->addItem(i18n("Binomial Distribution"), Binomial);
// 	ui.cbDistribution->addItem(i18n("Multinomial Distribution"));
	ui.cbDistribution->addItem(i18n("Negative Binomial Distribution"), NegativeBinomial);
	ui.cbDistribution->addItem(i18n("Pascal Distribution"), Pascal);
	ui.cbDistribution->addItem(i18n("Geometric Distribution"), Geometric);
	ui.cbDistribution->addItem(i18n("Hypergeometric Distribution"), Hypergeometric);
	ui.cbDistribution->addItem(i18n("Logarithmic Distribution"), Logarithmic);

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
	distribution distr = (distribution)ui.cbDistribution->itemData(index).toInt();
	if (distr == Gaussian) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("μ"));
		ui.lParameter2->setText(QString::fromUtf8("σ"));
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == Exponential) {
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("λ"));
		ui.kleParameter1->setText("1.0");
	} else if (distr == Laplace) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("μ"));
		ui.lParameter2->setText(QString::fromUtf8("a"));
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == ExponentialPower) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("μ"));
		ui.lParameter2->setText(QString::fromUtf8("a"));
		ui.lParameter2->setText(QString::fromUtf8("b"));
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("1.0");
	} else if (distr == Cauchy) {
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("a"));
		ui.kleParameter1->setText("1.0");
	} else if (distr == Rayleigh) {
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("σ"));
		ui.kleParameter1->setText("1.0");
	} else if (distr == RayleighTail) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("σ"));
		ui.lParameter2->setText(QString::fromUtf8("a"));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("0.0");
	} else if (distr == Landau) {
		ui.lParameter1->hide();
		ui.kleParameter1->hide();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
	} else if (distr == LevyAlphaStable) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("c"));
		ui.lParameter2->setText(QString::fromUtf8("α"));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
	}else if (distr == LevySkewAlphaStable) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("c"));
		ui.lParameter2->setText(QString::fromUtf8("α"));
		ui.lParameter3->setText(QString::fromUtf8("β"));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("1.0");
	} else if (distr == Gamma) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("a"));
		ui.lParameter2->setText(QString::fromUtf8("b"));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == Flat) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("a"));
		ui.lParameter2->setText(QString::fromUtf8("b"));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
	}  else if (distr == Lognormal) {
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("σ"));
		ui.lParameter2->setText(QString::fromUtf8("ζ"));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == ChiSquared) {
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lParameter1->setText(QString::fromUtf8("ν"));
		ui.kleParameter1->setText("1.0");
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
	distribution distr = (distribution)ui.cbDistribution->itemData(index).toInt();
	if (distr == Gaussian) {
		double mu = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_gaussian(r, sigma) + mu;
				col->setValueAt(i, value);
			}
		}
	} else if (distr == Exponential) {
		double mu = ui.kleParameter1->text().toDouble();
		mu = 1/mu; //GSL uses the inverse for exp. distrib.
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_exponential(r, mu);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == Laplace) {
		double mu = ui.kleParameter1->text().toDouble();
		double a = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_laplace(r, a) + mu;
				col->setValueAt(i, value);
			}
		}
	} else if (distr == ExponentialPower) {
		double mu = ui.kleParameter1->text().toDouble();
		double a = ui.kleParameter2->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_exppow(r, a, b) + mu;
				col->setValueAt(i, value);
			}
		}
	} else if (distr == Cauchy) {
		double a = ui.kleParameter1->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_cauchy(r, a);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == Rayleigh) {
		double sigma = ui.kleParameter1->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_rayleigh(r, sigma);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == RayleighTail) {
		double sigma = ui.kleParameter1->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_rayleigh(r, sigma);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == Landau) {
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_landau(r);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == LevyAlphaStable) {
		double c = ui.kleParameter1->text().toDouble();
		double alpha = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_levy(r, c, alpha);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == LevySkewAlphaStable) {
		double c = ui.kleParameter1->text().toDouble();
		double alpha = ui.kleParameter2->text().toDouble();
		double beta = ui.kleParameter3->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_levy_skew(r, c, alpha, beta);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == Gamma) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_gamma(r, a, b);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == Flat) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_flat(r, a, b);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == Lognormal) {
		double zeta = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_lognormal(r, zeta, sigma);
				col->setValueAt(i, value);
			}
		}
	} else if (distr == ChiSquared) {
		double nu = ui.kleParameter1->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i=0; i<col->rowCount(); ++i) {
				double value = gsl_ran_chisq(r, nu);
				col->setValueAt(i, value);
			}
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;

	gsl_rng_free (r);
}
