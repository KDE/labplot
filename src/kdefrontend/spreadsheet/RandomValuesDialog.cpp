/***************************************************************************
    File                 : RandomValuesDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating non-uniformly distributed random numbers
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016-2017 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include <QStandardPaths>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>
#include <QFileInfo>

extern "C" {
#include <stdio.h>
#include "backend/nsl/nsl_sf_stats.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
}

/*!
	\class RandomValuesDialog
	\brief Dialog for generating non-uniform random numbers.

	\ingroup kdefrontend
 */

RandomValuesDialog::RandomValuesDialog(Spreadsheet* s, QWidget* parent, Qt::WFlags fl) : QDialog(parent, fl), m_spreadsheet(s) {
	setWindowTitle(i18n("Random values"));

	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	QVBoxLayout *layout = new QVBoxLayout(this);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setDefault(true);
    m_okButton->setToolTip(i18n("Generate random values according to the selected distribution"));
    m_okButton->setText(i18n("&Generate"));
	layout->addWidget(mainWidget);
	layout->addWidget(buttonBox);
	setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);
	for (int i = 0; i < NSL_SF_STATS_DISTRIBUTION_RNG_COUNT; i++)
                ui.cbDistribution->addItem(i18n(nsl_sf_stats_distribution_name[i]), i);

	//use white background in the preview label
	QPalette p;
	p.setColor(QPalette::Window, Qt::white);
	ui.lFuncPic->setAutoFillBackground(true);
	ui.lFuncPic->setPalette(p);

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
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(generate()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

	//restore saved settings if available
	const KConfigGroup conf(KSharedConfig::openConfig(), "RandomValuesDialog");
	if (conf.exists()) {
		ui.cbDistribution->setCurrentIndex(conf.readEntry("Distribution", 0));
		this->distributionChanged(ui.cbDistribution->currentIndex()); //if index=0 no signal is emmited above, call this slot directly here
		ui.kleParameter1->setText(conf.readEntry("Parameter1"));
		ui.kleParameter2->setText(conf.readEntry("Parameter2"));
		ui.kleParameter3->setText(conf.readEntry("Parameter3"));

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
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

	KWindowConfig::saveWindowSize(windowHandle(), conf);
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
		ui.lParameter1->setText(QString::fromUtf8("\u03c3 ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("0.0");
		break;
	case nsl_sf_stats_gaussian_tail:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("\u03bc ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03c3 ="));
		ui.lParameter3->setText("a =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("0.0");
		break;
	case nsl_sf_stats_exponential:
		ui.lParameter1->setText(QString::fromUtf8("\u03bb ="));
		ui.kleParameter1->setText("1.0");
		ui.lParameter2->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter2->setText("0.0");
		break;
	case nsl_sf_stats_laplace:
		ui.lParameter1->setText(QString::fromUtf8("\u03c3 ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("0.0");
		break;
	case nsl_sf_stats_exponential_power:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("\u03bc ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03c3 ="));
		ui.lParameter3->setText("b =");
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("1.0");
		break;
	case nsl_sf_stats_cauchy_lorentz:
		ui.lParameter1->setText(QString::fromUtf8("\u03b3 ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("0.0");
		break;
	case nsl_sf_stats_rayleigh:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText(QString::fromUtf8("\u03c3 ="));
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_rayleigh_tail:
		ui.lParameter1->setText(QString::fromUtf8("\u03bc ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03c3 ="));
		ui.kleParameter1->setText("0.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_landau:
		ui.lParameter1->hide();
		ui.kleParameter1->hide();
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		break;
	case nsl_sf_stats_levy_alpha_stable:
		ui.lParameter1->setText("c =");
		ui.lParameter2->setText(QString::fromUtf8("\u03b1 ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_levy_skew_alpha_stable:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("c ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03b1 ="));
		ui.lParameter3->setText(QString::fromUtf8("\u03b2 ="));
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
		ui.lParameter1->setText(QString::fromUtf8("\u03b8 ="));
		ui.lParameter2->setText("k =");
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_weibull:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText("k =");
		ui.lParameter2->setText(QString::fromUtf8("\u03bb ="));
		ui.lParameter3->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("1.0");
		break;
	case nsl_sf_stats_beta:
		ui.lParameter1->setText("a =");
		ui.lParameter2->setText("b =");
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_gumbel1:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText(QString::fromUtf8("\u03c3 ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03b2 ="));
		ui.lParameter3->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("0.0");
		break;
	case nsl_sf_stats_gumbel2:
		ui.lParameter3->show();
		ui.kleParameter3->show();
		ui.lParameter1->setText("a =");
		ui.lParameter2->setText("b =");
		ui.lParameter3->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		ui.kleParameter3->setText("0.0");
		break;
	case nsl_sf_stats_pareto:
		ui.lParameter1->setText("a =");
		ui.lParameter2->setText("b =");
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("0.0");
		break;
	case nsl_sf_stats_lognormal:
		ui.lParameter1->setText(QString::fromUtf8("\u03c3 ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_chi_squared:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText("n =");
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_fdist:
		ui.lParameter1->setText(QString::fromUtf8("\u03bd\u2081 ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03bd\u2082 ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("1.0");
		break;
	case nsl_sf_stats_tdist:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lParameter1->setText(QString::fromUtf8("\u03bd ="));
		ui.kleParameter1->setText("1.0");
		break;
	case nsl_sf_stats_logistic:
		ui.lParameter1->setText(QString::fromUtf8("\u03c3 ="));
		ui.lParameter2->setText(QString::fromUtf8("\u03bc ="));
		ui.kleParameter1->setText("1.0");
		ui.kleParameter2->setText("0.0");
		break;
	case nsl_sf_stats_poisson:
		ui.lParameter2->hide();
		ui.kleParameter2->hide();
		ui.lFunc->setText("p(k) =");
		ui.lParameter1->setText(QString::fromUtf8("\u03bb ="));
		ui.kleParameter1->setText("1.0");
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
	case nsl_sf_stats_negative_binomial:
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
		break;
	case nsl_sf_stats_maxwell_boltzmann:	// additional non-GSL distros
	case nsl_sf_stats_sech:
	case nsl_sf_stats_levy:
	case nsl_sf_stats_frechet:
		break;
	}

	QString file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/gsl_distributions/" + QString(nsl_sf_stats_distribution_pic_name[dist]) + ".jpg");
	ui.lFuncPic->setPixmap(QPixmap(file));
}

void RandomValuesDialog::checkValues() {
	if (ui.kleParameter1->text().simplified().isEmpty()) {
        m_okButton->setEnabled(false);
		return;
	}

	if (ui.kleParameter2->isVisible() && ui.kleParameter2->text().simplified().isEmpty()) {
        m_okButton->setEnabled(false);
		return;
	}

	if (ui.kleParameter3->isVisible() && ui.kleParameter3->text().simplified().isEmpty()) {
        m_okButton->setEnabled(false);
		return;
	}

    m_okButton->setEnabled(true);
	return;
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

	const int index = ui.cbDistribution->currentIndex();
	const nsl_sf_stats_distribution dist = (nsl_sf_stats_distribution)ui.cbDistribution->itemData(index).toInt();

	const int rows = m_spreadsheet->rowCount();
	QVector<double> new_data(rows);

	switch (dist) {
	case nsl_sf_stats_gaussian: {
		double mu = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_gaussian(r, sigma) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_gaussian_tail: {
		double mu = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		double a = ui.kleParameter3->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_gaussian_tail(r, a, sigma) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_exponential: {
		double l = ui.kleParameter1->text().toDouble();
		double mu = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			//GSL uses the inverse for exp. distrib.
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_exponential(r, 1./l) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_laplace: {
		double s = ui.kleParameter1->text().toDouble();
		double mu = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_laplace(r, s) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_exponential_power: {
		double mu = ui.kleParameter1->text().toDouble();
		double a = ui.kleParameter2->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_exppow(r, a, b) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_cauchy_lorentz: {
		double gamma = ui.kleParameter1->text().toDouble();
		double mu = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_cauchy(r, gamma) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_rayleigh: {
		double s = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_rayleigh(r, s);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_rayleigh_tail: {
		double mu = ui.kleParameter1->text().toDouble();
		double sigma = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_rayleigh_tail(r, mu, sigma);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_landau:
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_landau(r);
			col->replaceValues(0, new_data);
		}
		break;
	case nsl_sf_stats_levy_alpha_stable: {
		double c = ui.kleParameter1->text().toDouble();
		double alpha = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_levy(r, c, alpha);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_levy_skew_alpha_stable: {
		double c = ui.kleParameter1->text().toDouble();
		double alpha = ui.kleParameter2->text().toDouble();
		double beta = ui.kleParameter3->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_levy_skew(r, c, alpha, beta);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_gamma: {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_gamma(r, a, b);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_flat: {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_flat(r, a, b);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_lognormal: {
		double s = ui.kleParameter1->text().toDouble();
		double mu = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_lognormal(r, mu, s);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_chi_squared: {
		double n = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_chisq(r, n);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_fdist: {
		double nu1 = ui.kleParameter1->text().toDouble();
		double nu2 = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_fdist(r, nu1, nu2);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_tdist: {
		double nu = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_tdist(r, nu);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_beta: {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_beta(r, a, b);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_logistic: {
		double s = ui.kleParameter1->text().toDouble();
		double mu = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_logistic(r, s) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_pareto: {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_pareto(r, a, b);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_weibull: {
		double k = ui.kleParameter1->text().toDouble();
		double l = ui.kleParameter2->text().toDouble();
		double mu = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_weibull(r, l, k) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_gumbel1: {
		double s = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		double mu = ui.kleParameter3->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_gumbel1(r, 1./s, b) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_gumbel2: {
		double a = ui.kleParameter1->text().toDouble();
		double b = ui.kleParameter2->text().toDouble();
		double mu = ui.kleParameter3->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_gumbel2(r, a, b) + mu;
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_poisson: {
		double l = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_poisson(r, l);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_bernoulli: {
		double p = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_bernoulli(r, p);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_binomial: {
		double p = ui.kleParameter1->text().toDouble();
		double n = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_binomial(r, p, n);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_negative_binomial: {
		double p = ui.kleParameter1->text().toDouble();
		double n = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_negative_binomial(r, p, n);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_pascal: {
		double p = ui.kleParameter1->text().toDouble();
		double n = ui.kleParameter2->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_pascal(r, p, n);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_geometric: {
		double p = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_geometric(r, p);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_hypergeometric: {
		double n1 = ui.kleParameter1->text().toDouble();
		double n2 = ui.kleParameter2->text().toDouble();
		double t = ui.kleParameter3->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_hypergeometric(r, n1, n2, t);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_logarithmic: {
		double p = ui.kleParameter1->text().toDouble();
		foreach (Column* col, m_columns) {
			for (int i = 0; i < rows; ++i)
				new_data[i] = gsl_ran_logarithmic(r, p);
			col->replaceValues(0, new_data);
		}
		break;
	}
	case nsl_sf_stats_maxwell_boltzmann:	// additional non-GSL distros
	case nsl_sf_stats_sech:
	case nsl_sf_stats_levy:
	case nsl_sf_stats_frechet:
		break;
	}

	foreach (Column* col, m_columns) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;

	gsl_rng_free(r);
}
