/***************************************************************************
    File                 : RandomValuesDialog.cpp
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
#include "RandomValuesDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include <KStandardDirs>

extern "C" {
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
}

/*!
	\class RandomValuesDialog
	\brief Dialog for generating non-uniform random numbers.

	\ingroup kdefrontend
 */

//TODO: use nsl enum
enum distribution {
	Gaussian, GaussianTail, Exponential, Laplace, ExponentialPower, Cauchy, Rayleigh, RayleighTail, Landau,
	LevyAlphaStable, LevySkewAlphaStable, Gamma, Flat, Lognormal, ChiSquared, F, t, Beta,
	Logistic, Pareto, Weibull, Gumbel1, Gumbel2, Poisson, Bernoulli, Binomial,
	NegativeBinomial, Pascal, Geometric, Hypergeometric, Logarithmic
};

RandomValuesDialog::RandomValuesDialog(Spreadsheet* s, QWidget* parent, Qt::WFlags fl) : KDialog(parent, fl), m_spreadsheet(s) {

	setWindowTitle(i18n("Random values"));

	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	setMainWidget( mainWidget );

	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonText(KDialog::Ok, i18n("&Generate"));
	setButtonToolTip(KDialog::Ok, i18n("Generate random values according to the selected distribution"));

	ui.cbDistribution->addItem(i18n("Gaussian Distribution"), Gaussian);
 	ui.cbDistribution->addItem(i18n("Gaussian Tail Distribution"), GaussianTail);
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
	ui.cbDistribution->addItem(i18n("Type-1 Gumbel Distribution"), Gumbel1);
	ui.cbDistribution->addItem(i18n("Type-2 Gumbel Distribution"), Gumbel2);
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

	m_formulaPixs[Gaussian] = "gaussian";
	m_formulaPixs[GaussianTail] = "gaussian_tail";
	m_formulaPixs[Exponential] = "exponential";
	m_formulaPixs[Laplace] = "laplace";
	m_formulaPixs[ExponentialPower] = "exponential_power";
	m_formulaPixs[Cauchy] = "cauchy";
	m_formulaPixs[Rayleigh] = "rayleigh";
	m_formulaPixs[RayleighTail] = "rayleigh_tail";
	m_formulaPixs[Landau] = "landau";
	m_formulaPixs[LevyAlphaStable] = "levy_alpha_stable";
	m_formulaPixs[LevySkewAlphaStable] = "levy_skew_alpha_stable";
	m_formulaPixs[Gamma] = "gamma";
	m_formulaPixs[Flat] = "flat";
	m_formulaPixs[Lognormal] = "lognormal";
	m_formulaPixs[ChiSquared] = "chi_squared";
	m_formulaPixs[F] = 'F';
	m_formulaPixs[t] = 't';
	m_formulaPixs[Beta] = "beta";
	m_formulaPixs[Logistic] = "logistic";
	m_formulaPixs[Pareto] = "pareto";
	m_formulaPixs[Weibull] = "weibull";
	m_formulaPixs[Gumbel1] = "gumbel_type_1";
	m_formulaPixs[Gumbel2] = "gumbel_type_2";
	m_formulaPixs[Poisson] = "poisson";
	m_formulaPixs[Bernoulli] = "bernoulli";
	m_formulaPixs[Binomial] = "binomial";
	m_formulaPixs[NegativeBinomial] = "binomial_negative";
	m_formulaPixs[Pascal] = "pascal";
	m_formulaPixs[Geometric] = "geometric";
	m_formulaPixs[Hypergeometric] = "hypergeometric";
	m_formulaPixs[Logarithmic] = "logarithmic";

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

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), "RandomValuesDialog");
	if (conf.exists()) {
		ui.cbDistribution->setCurrentIndex(conf.readEntry("Distribution", 0));
		this->distributionChanged(ui.cbDistribution->currentIndex()); //if index=0 no signal is emmited above, call this slot directly here
		ui.kleParameter1->setText(conf.readEntry("Parameter1"));
		ui.kleParameter2->setText(conf.readEntry("Parameter2"));
		ui.kleParameter3->setText(conf.readEntry("Parameter3"));
		restoreDialogSize(conf);
	} else {
		//Gaussian distribution as default
		this->distributionChanged(0);

		resize( QSize(400,0).expandedTo(minimumSize()) );
	}
}

RandomValuesDialog::~RandomValuesDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "RandomValuesDialog");
	conf.writeEntry("Distribution", ui.cbDistribution->currentIndex());
	conf.writeEntry("Parameter1", ui.kleParameter1->text());
	conf.writeEntry("Parameter2", ui.kleParameter2->text());
	conf.writeEntry("Parameter3", ui.kleParameter3->text());
	saveDialogSize(conf);
}

void RandomValuesDialog::setColumns(QList<Column*> list) {
	m_columns = list;
}

void RandomValuesDialog::distributionChanged(int index) {
	distribution distr = (distribution)ui.cbDistribution->itemData(index).toInt();
	//TODO: use "switch"
	//TODO: use nsl enum
	if (distr == Gaussian) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.lParameter2->setText(QString::fromUtf8("σ ="));
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == GaussianTail) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.lParameter2->setText(QString::fromUtf8("σ ="));
		ui.lParameter3->setText("a =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("0.0");
	} else if (distr == Exponential) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("λ ="));
		ui.kleParameter1->setText("1.0");
	} else if (distr == Laplace) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.lParameter2->setText("a =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == ExponentialPower) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.lParameter2->setText("a =");
		ui.lParameter3->setText("b =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("1.0");
	} else if (distr == Cauchy) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText("a =");
		ui.kleParameter1->setText("1.0");
	} else if (distr == Rayleigh) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("σ ="));
		ui.kleParameter1->setText("1.0");
	} else if (distr == RayleighTail) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("σ ="));
		ui.lParameter2->setText("a =");
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("0.0");
	} else if (distr == Landau) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter1->hide();
		ui.kleParameter1->hide();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
	} else if (distr == LevyAlphaStable) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText("c =");
		ui.lParameter2->setText(QString::fromUtf8("α ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
	}else if (distr == LevySkewAlphaStable) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("c ="));
		ui.lParameter2->setText(QString::fromUtf8("α ="));
		ui.lParameter3->setText(QString::fromUtf8("β ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("1.0");
	} else if (distr == Flat) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText("a =");
		ui.lParameter2->setText("b =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == Gamma || distr == Flat || distr == Beta || distr == Pareto || distr == Weibull || distr == Gumbel1 || distr == Gumbel2) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText("a =");
		ui.lParameter2->setText("b =");
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
	}  else if (distr == Lognormal) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("σ ="));
		ui.lParameter2->setText(QString::fromUtf8("ζ ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == ChiSquared) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("ν ="));
		ui.kleParameter1->setText("1.0");
	} else if (distr == F) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("ν1 ="));
		ui.lParameter2->setText(QString::fromUtf8("ν2 ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
	} else if (distr == t) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText(QString::fromUtf8("ν ="));
		ui.kleParameter1->setText("1.0");
	} else if (distr == Logistic) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(x) =");
		ui.lParameter1->setText("a =");
		ui.kleParameter1->setText("1.0");
	} else if (distr == Poisson) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(k) =");
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.kleParameter1->setText("0.0");
	} else if (distr == Bernoulli || distr == Geometric || distr == Logarithmic) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		if (distr == Bernoulli) {
			ui.lFunc->setText("");
		} else {
			ui.lFunc->setText("p(k) =");
		}
		ui.lParameter1->setText("p =");
		ui.kleParameter1->setText("0.5");
	} else if (distr == Binomial || distr == NegativeBinomial || distr == Pascal) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->hide();
		ui.kleParameter3->hide();
		ui.lFunc->setText("p(k) =");
		ui.lParameter1->setText("p =");
		ui.lParameter1->setText("n =");
		ui.kleParameter1->setText("0.5");
		ui.kleParameter2->setText("100");
	} else if (distr == Hypergeometric) {
		ui.lParameter1->show();
		ui.kleParameter1->show();
		ui.lParameter2->show();
		ui.kleParameter2->show();
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lFunc->setText("p(k) =");
		ui.lParameter1->setText("n1 =");
		ui.lParameter2->setText("n2 =");
		ui.lParameter3->setText("t =");
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("2.0");
		ui.kleParameter3->setText("3.0");
	}

	QString file = KStandardDirs::locate("data", "labplot2/pics/gsl_distributions/" + m_formulaPixs[distr] + ".jpg");
	ui.lFuncPic->setPixmap(QPixmap(file));
}

void RandomValuesDialog::checkValues() {
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

void RandomValuesDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	//create a generator chosen by the environment variable GSL_RNG_TYPE
	gsl_rng_env_setup();
	const gsl_rng_type* T = gsl_rng_default;
	gsl_rng* r = gsl_rng_alloc(T);

	WAIT_CURSOR;
	foreach (Column* col, m_columns)
		col->setSuppressDataChangedSignal(true);

	m_spreadsheet->beginMacro(i18np("%1: fill column with non-uniform random numbers",
					"%1: fill columns with non-uniform random numbers",
					m_spreadsheet->name(), m_columns.size()));

	int index = ui.cbDistribution->currentIndex();
	distribution distr = (distribution)ui.cbDistribution->itemData(index).toInt();

	const int rows = m_spreadsheet->rowCount();
	QVector<double> new_data(rows);

	//TODO: use switch
	//TODO: double value
	if (distr == Gaussian) {
		double mu = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_gaussian(r, sigma) + mu;
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == GaussianTail) {
		double mu = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		double a = ui.kleParameter3->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_gaussian_tail(r, a, sigma) + mu;
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Exponential) {
		double mu = ui.kleParameter1->text().toDouble();
		mu = 1./mu; //GSL uses the inverse for exp. distrib.
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_exponential(r, mu);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Laplace) {
		double mu = ui.kleParameter1->text().toDouble();
		double a = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_laplace(r, a) + mu;
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == ExponentialPower) {
		double mu = ui.kleParameter1->text().toDouble();
		double a = ui.kleParameter2->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_exppow(r, a, b) + mu;
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Cauchy) {
		double a = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_cauchy(r, a);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Rayleigh) {
		double sigma = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_rayleigh(r, sigma);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == RayleighTail) {
		double sigma = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_rayleigh(r, sigma);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Landau) {
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_landau(r);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == LevyAlphaStable) {
		double c = ui.kleParameter1->text().toDouble();
		double alpha = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_levy(r, c, alpha);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == LevySkewAlphaStable) {
		double c = ui.kleParameter1->text().toDouble();
		double alpha = ui.kleParameter2->text().toDouble();
		double beta = ui.kleParameter3->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_levy_skew(r, c, alpha, beta);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Gamma) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_gamma(r, a, b);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Flat) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_flat(r, a, b);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Lognormal) {
		double zeta = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		foreach(Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_lognormal(r, zeta, sigma);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == ChiSquared) {
		double nu = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_chisq(r, nu);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == F) {
		double nu1 = ui.kleParameter1->text().toDouble();
		double nu2 = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_fdist(r, nu1, nu2);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == t) {
		double nu = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_tdist(r, nu);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Beta) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_beta(r, a, b);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Logistic) {
		double a = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_logistic(r, a);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Pareto) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_pareto(r, a, b);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Weibull) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_weibull(r, a, b);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Gumbel1) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_gumbel1(r, a, b);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Gumbel2) {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_gumbel2(r, a, b);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Poisson) {
		double mu = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_poisson(r, mu);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Bernoulli) {
		double p = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_bernoulli(r, p);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Binomial) {
		double p = ui.kleParameter1->text().toDouble();
		double n = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_binomial(r, p, n);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == NegativeBinomial) {
		double p = ui.kleParameter1->text().toDouble();
		double n = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_negative_binomial(r, p, n);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Pascal) {
		double p = ui.kleParameter1->text().toDouble();
		double n = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_pascal(r, p, n);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Geometric) {
		double p = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_geometric(r, p);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Hypergeometric) {
		double n1 = ui.kleParameter1->text().toDouble();
		double n2 = ui.kleParameter2->text().toDouble();
		double t = ui.kleParameter3->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_hypergeometric(r, n1, n2, t);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	} else if (distr == Logarithmic) {
		double p = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i) {
				double value = gsl_ran_logarithmic(r, p);
				new_data[i] = value;
			}
			col->replaceValues(0, new_data);
		}
	}

	foreach (Column* col, m_columns) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;

	gsl_rng_free(r);
}
