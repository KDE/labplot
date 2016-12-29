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
#include "backend/nsl/nsl_sf_stats.h"
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
	setMainWidget(mainWidget);

	setButtons(KDialog::Ok | KDialog::Cancel);
	setButtonText(KDialog::Ok, i18n("&Generate"));
	setButtonToolTip(KDialog::Ok, i18n("Generate random values according to the selected distribution"));

	for (int i = 0; i < NSL_SF_STATS_DISTRIBUTION_RNG_COUNT; i++) {
                ui.cbDistribution->addItem(i18n(nsl_sf_stats_distribution_name[i]), i);
		m_formulaPixs[i] = nsl_sf_stats_distribution_pic_name[i];
	}

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
	nsl_sf_stats_distribution dist = (nsl_sf_stats_distribution)ui.cbDistribution->itemData(index).toInt();

	//  default settings (used by most distributions)
	ui.lParameter1->show();
	ui.kleParameter1->show();
	ui.lParameter2->show();
	ui.kleParameter2->show();
	ui.lParameter3->hide();
	ui.kleParameter3->hide();
	ui.lFunc->setText("p(x) =");

	switch (dist) {
	case nsl_sf_stats_gaussian:
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.lParameter2->setText(QString::fromUtf8("σ ="));
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_gaussian_tail:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.lParameter2->setText(QString::fromUtf8("σ ="));
		ui.lParameter3->setText("a =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("0.0");
		break;
	case nsl_sf_stats_exponential:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText(QString::fromUtf8("λ ="));
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_laplace:
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.lParameter2->setText("a =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_exponential_power:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.lParameter2->setText("a =");
		ui.lParameter3->setText("b =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("1.0");
		break;
	case nsl_sf_stats_cauchy_lorentz:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText("a =");
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_rayleigh:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText(QString::fromUtf8("σ ="));
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_rayleigh_tail:
		ui.lParameter1->setText(QString::fromUtf8("σ ="));
		ui.lParameter2->setText("a =");
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("0.0");
		break;
	case nsl_sf_stats_landau:
		ui.lParameter1->hide();
		ui.kleParameter1->hide();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		break;
	case nsl_sf_stats_levy_alpha_stable:
		ui.lParameter1->setText("c =");
		ui.lParameter2->setText(QString::fromUtf8("α ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_levy_skew_alpha_stable:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("c ="));
		ui.lParameter2->setText(QString::fromUtf8("α ="));
		ui.lParameter3->setText(QString::fromUtf8("β ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("1.0");
		break;
	case nsl_sf_stats_flat:
		ui.lParameter1->setText("a =");
		ui.lParameter2->setText("b =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_gamma:
	case nsl_sf_stats_beta:
	case nsl_sf_stats_pareto:
	case nsl_sf_stats_weibull:
	case nsl_sf_stats_gumbel1:
	case nsl_sf_stats_gumbel2:
		ui.lParameter1->setText("a =");
		ui.lParameter2->setText("b =");
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_lognormal:
		ui.lParameter1->setText(QString::fromUtf8("σ ="));
		ui.lParameter2->setText(QString::fromUtf8("ζ ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_chisquared:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText(QString::fromUtf8("ν ="));
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_fdist:
		ui.lParameter1->setText(QString::fromUtf8("ν1 ="));
		ui.lParameter2->setText(QString::fromUtf8("ν2 ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_tdist:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText(QString::fromUtf8("ν ="));
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_logistic:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText("a =");
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_poisson:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lFunc->setText("p(k) =");
		ui.lParameter1->setText(QString::fromUtf8("μ ="));
		ui.kleParameter1->setText("0.0");
		break;
	case nsl_sf_stats_bernoulli:
	case nsl_sf_stats_geometric:
	case nsl_sf_stats_logarithmic:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		if (dist == nsl_sf_stats_bernoulli)
			ui.lFunc->setText("");
		else
			ui.lFunc->setText("p(k) =");
		ui.lParameter1->setText("p =");
		ui.kleParameter1->setText("0.5");
		break;
	case nsl_sf_stats_binomial:
	case nsl_sf_stats_negative_bionomial:
	case nsl_sf_stats_pascal:
		ui.lFunc->setText("p(k) =");
		ui.lParameter1->setText("p =");
		ui.lParameter1->setText("n =");
		ui.kleParameter1->setText("0.5");
		ui.kleParameter2->setText("100");
		break;
	case nsl_sf_stats_hypergeometric:
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

	QString file = KStandardDirs::locate("data", "labplot2/pics/gsl_distributions/" + m_formulaPixs[dist] + ".jpg");
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
