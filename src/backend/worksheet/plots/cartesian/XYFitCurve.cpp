
/***************************************************************************
    File                 : XYFitCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a fit model
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016-2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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

/*!
  \class XYFitCurve
  \brief A xy-curve defined by a fit model

  \ingroup worksheet
*/

#include "XYFitCurve.h"
#include "XYFitCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/gsl/ExpressionParser.h"

extern "C" {
#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_version.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_statistics_double.h>
#include "backend/gsl/parser.h"
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/nsl/nsl_stats.h"
}
#include <cmath>

#include <QElapsedTimer>
#include <QIcon>
#include <KLocalizedString>
#include <QThreadPool>

XYFitCurve::XYFitCurve(const QString& name)
		: XYCurve(name, new XYFitCurvePrivate(this)) {
	init();
}

XYFitCurve::XYFitCurve(const QString& name, XYFitCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}

XYFitCurve::~XYFitCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYFitCurve::init() {
	Q_D(XYFitCurve);

	//TODO: read from the saved settings for XYFitCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYFitCurve::recalculate() {
	Q_D(XYFitCurve);
	d->recalculate();
}

void XYFitCurve::initFitData(PlotDataDialog::AnalysisAction action) {
	if (!action)
		return;

	Q_D(XYFitCurve);
	XYFitCurve::FitData& fitData = d->fitData;
	if (action == PlotDataDialog::FitLinear) {
		//Linear
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = nsl_fit_model_polynomial;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::FitPower) {
		//Power
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = nsl_fit_model_power;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::FitExp1) {
		//Exponential (degree 1)
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = nsl_fit_model_exponential;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::FitExp2) {
		//Exponential (degree 2)
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = nsl_fit_model_exponential;
		fitData.degree = 2;
	} else if (action == PlotDataDialog::FitInvExp) {
		//Inverse exponential
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = nsl_fit_model_inverse_exponential;
	} else if (action == PlotDataDialog::FitGauss) {
		//Gauss
		fitData.modelCategory = nsl_fit_model_peak;
		fitData.modelType = nsl_fit_model_gaussian;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::FitCauchyLorentz) {
		//Cauchy-Lorentz
		fitData.modelCategory = nsl_fit_model_peak;
		fitData.modelType = nsl_fit_model_lorentz;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::FitTan) {
		//Arc tangent
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = nsl_fit_model_atan;
	} else if (action == PlotDataDialog::FitTanh) {
		//Hyperbolic tangent
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = nsl_fit_model_tanh;
	} else if (action == PlotDataDialog::FitErrFunc) {
		//Error function
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = nsl_fit_model_erf;
	} else {
		//Custom
		fitData.modelCategory = nsl_fit_model_custom;
		fitData.modelType = nsl_fit_model_custom;
	}

	XYFitCurve::initFitData(fitData);
}

/*!
 * sets the model expression and the parameter names for given model category, model type and degree in \c fitData
 */
void XYFitCurve::initFitData(XYFitCurve::FitData& fitData) {
	nsl_fit_model_category modelCategory = fitData.modelCategory;
	unsigned int modelType = fitData.modelType;
	QString& model = fitData.model;
	QStringList& paramNames = fitData.paramNames;
	QStringList& paramNamesUtf8 = fitData.paramNamesUtf8;
	int degree = fitData.degree;
	QVector<double>& paramStartValues = fitData.paramStartValues;
	QVector<double>& paramLowerLimits = fitData.paramLowerLimits;
	QVector<double>& paramUpperLimits = fitData.paramUpperLimits;
	QVector<bool>& paramFixed = fitData.paramFixed;

	DEBUG("XYFitCurve::initFitData() for model category = " << modelCategory << ", model type = " << modelType << ", degree = " << degree);

	if (modelCategory != nsl_fit_model_custom)
		paramNames.clear();
	paramNamesUtf8.clear();

	// 10 indices used in multi degree models
	QStringList indices = {QString::fromUtf8("\u2081"), QString::fromUtf8("\u2082"), QString::fromUtf8("\u2083"),
		QString::fromUtf8("\u2084"), QString::fromUtf8("\u2085"), QString::fromUtf8("\u2086"), QString::fromUtf8("\u2087"),
		QString::fromUtf8("\u2088"), QString::fromUtf8("\u2089"), QString::fromUtf8("\u2081") + QString::fromUtf8("\u2080")};

	switch (modelCategory) {
	case nsl_fit_model_basic:
		model = nsl_fit_model_basic_equation[fitData.modelType];
		switch (modelType) {
		case nsl_fit_model_polynomial:
			paramNames << "c0" << "c1";
			paramNamesUtf8 << QString::fromUtf8("c\u2080") << QString::fromUtf8("c\u2081");
			if (degree == 2) {
				model += " + c2*x^2";
				paramNames << "c2";
				paramNamesUtf8 << QString::fromUtf8("c\u2082");
			} else if (degree > 2) {
				for (int i = 2; i <= degree; ++i) {
					QString numStr = QString::number(i);
					model += "+c" + numStr + "*x^" + numStr;
					paramNames << "c" + numStr;
					paramNamesUtf8 << "c" + indices[i-1];
				}
			}
			break;
		case nsl_fit_model_power:
			if (degree == 1) {
				paramNames << "a" << "b";
			} else {
				paramNames << "a" << "b" << "c";
				model = "a + b*x^c";
			}
			break;
		case nsl_fit_model_exponential:
			if (degree == 1) {
				paramNames << "a" << "b";
			} else {
				for (int i = 1; i <= degree; i++) {
					QString numStr = QString::number(i);
					if (i == 1)
						model = "a1*exp(b1*x)";
					else
						model += " + a" + numStr + "*exp(b" + numStr + "*x)";
					paramNames << "a" + numStr << "b" + numStr;
					paramNamesUtf8 << "a" + indices[i-1] << "b" + indices[i-1];
				}
			}
			break;
		case nsl_fit_model_inverse_exponential:
			degree = 1;
			paramNames << "a" << "b" << "c";
			break;
		case nsl_fit_model_fourier:
			paramNames << "w" << "a0" << "a1" << "b1";
			paramNamesUtf8 << QString::fromUtf8("\u03c9") << QString::fromUtf8("a\u2080")
				<< QString::fromUtf8("a\u2081") << QString::fromUtf8("b\u2081");
			if (degree > 1) {
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					model += "+ (a" + numStr + "*cos(" + numStr + "*w*x) + b" + numStr + "*sin(" + numStr + "*w*x))";
					paramNames << "a" + numStr << "b" + numStr;
					paramNamesUtf8 << "a" + indices[i-1] << "b" + indices[i-1];
				}
			}
			break;
		}
		break;
	case nsl_fit_model_peak:
		model = nsl_fit_model_peak_equation[fitData.modelType];
		switch ((nsl_fit_model_type_peak)modelType) {
		case nsl_fit_model_gaussian:
			switch (degree) {
			case 1:
				paramNames << "s" << "mu" << "a";
				paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				model = "1./sqrt(2*pi) * (a1/s1 * exp(-((x-mu1)/s1)^2/2) + a2/s2 * exp(-((x-mu2)/s2)^2/2))";
				paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				model = "1./sqrt(2*pi) * (a1/s1 * exp(-((x-mu1)/s1)^2/2) + a2/s2 * exp(-((x-mu2)/s2)^2/2) + a3/s3 * exp(-((x-mu3)/s3)^2/2))";
				paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				model = "1./sqrt(2*pi) * (";
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += "a" + numStr + "/s" + numStr + "* exp(-((x-mu" + numStr + ")/s" + numStr + ")^2/2)";
					paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				model += ")";
			}
			break;
		case nsl_fit_model_lorentz:
			switch (degree) {
			case 1:
				paramNames << "g" << "mu" << "a";
				paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				model = "1./pi * (a1 * g1/(g1^2+(x-mu1)^2) + a2 * g2/(g2^2+(x-mu2)^2))";
				paramNames << "g1" << "mu1" << "a1" << "g2" << "mu2" << "a2";
				paramNamesUtf8 << QString::fromUtf8("\u03b3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03b3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				model = "1./pi * (a1 * g1/(g1^2+(x-mu1)^2) + a2 * g2/(g2^2+(x-mu2)^2) + a3 * g3/(g3^2+(x-mu3)^2))";
				paramNames << "g1" << "mu1" << "a1" << "g2" << "mu2" << "a2" << "g3" << "mu3" << "a3";
				paramNamesUtf8 << QString::fromUtf8("\u03b3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03b3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03b3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				model = "1./pi * (";
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += "a" + numStr + " * g" + numStr + "/(g" + numStr + "^2+(x-mu" + numStr + ")^2)";
					paramNames << "g" + numStr << "mu" + numStr << "a" + numStr;
					paramNamesUtf8 << QString::fromUtf8("\u03b3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				model += ")";
			}
			break;
		case nsl_fit_model_sech:
			switch (degree) {
			case 1:
				paramNames << "s" << "mu" << "a";
				paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				model = "1/pi * (a1/s1 * sech((x-mu1)/s1) + a2/s2 * sech((x-mu2)/s2))";
				paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				model = "1/pi * (a1/s1 * sech((x-mu1)/s1) + a2/s2 * sech((x-mu2)/s2) + a3/s3 * sech((x-mu3)/s3))";
				paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				model = "1/pi * (";
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += "a" + numStr + "/s" + numStr + "* sech((x-mu" + numStr + ")/s" + numStr + ")";
					paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				model += ")";
			}
			break;
		case nsl_fit_model_logistic:
			switch (degree) {
			case 1:
				paramNames << "s" << "mu" << "a";
				paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				model = "1/4 * (a1/s1 * sech((x-mu1)/2/s1)**2 + a2/s2 * sech((x-mu2)/2/s2)**2)";
				paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				model = "1/4 * (a1/s1 * sech((x-mu1)/2/s1)**2 + a2/s2 * sech((x-mu2)/2/s2)**2 + a3/s3 * sech((x-mu3)/2/s3)**2)";
				paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				model = "1/4 * (";
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += "a" + numStr + "/s" + numStr + "* sech((x-mu" + numStr + ")/2/s" + numStr + ")**2";
					paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				model += ")";
			}
			break;
		}
		break;
	case nsl_fit_model_growth:
		model = nsl_fit_model_growth_equation[fitData.modelType];
		switch ((nsl_fit_model_type_growth)modelType) {
		case nsl_fit_model_atan:
		case nsl_fit_model_tanh:
		case nsl_fit_model_algebraic_sigmoid:
		case nsl_fit_model_erf:
		case nsl_fit_model_gudermann:
			paramNames << "s" << "mu" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_fit_model_sigmoid:
			paramNames << "k" << "mu" << "a";
			paramNamesUtf8 << "k" << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_fit_model_hill:
			paramNames << "s" << "n" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03c3") << "n" << "A";
			break;
		case nsl_fit_model_gompertz:
			paramNames << "a" << "b" << "c";
			break;
		}
		break;
	case nsl_fit_model_distribution:
		model = nsl_sf_stats_distribution_equation[fitData.modelType];
		switch ((nsl_sf_stats_distribution)modelType) {
		case nsl_sf_stats_gaussian:
		case nsl_sf_stats_laplace:
		case nsl_sf_stats_rayleigh_tail:
		case nsl_sf_stats_lognormal:
		case nsl_sf_stats_logistic:
		case nsl_sf_stats_sech:
			paramNames << "s" << "mu" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gaussian_tail:
			paramNames << "s" << "mu" << "A" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A" << "a";
			break;
		case nsl_sf_stats_exponential:
			paramNames << "l" << "mu" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03bb") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_exponential_power:
			paramNames << "s" << "mu" << "b" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "b" << "A";
			break;
		case nsl_sf_stats_cauchy_lorentz:
		case nsl_sf_stats_levy:
			paramNames << "g" << "mu" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_rayleigh:
			paramNames << "s" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03c3") << "A";
			break;
		case nsl_sf_stats_landau:
			paramNames << "a";
			paramNamesUtf8 << "A";
			break;
		case nsl_sf_stats_levy_alpha_stable:	// unused distributions
		case nsl_sf_stats_levy_skew_alpha_stable:
		case nsl_sf_stats_bernoulli:
			break;
		case nsl_sf_stats_gamma:
			paramNames << "t" << "k" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03b8") << "k" << "A";
			break;
		case nsl_sf_stats_flat:
			paramNames << "a" << "b" << "A";
			break;
		case nsl_sf_stats_chi_squared:
			paramNames << "n" << "a";
			paramNamesUtf8 << "n" << "A";
			break;
		case nsl_sf_stats_fdist:
			paramNames << "n1" << "n2" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03bd") + QString::fromUtf8("\u2081")
				<< QString::fromUtf8("\u03bd") + QString::fromUtf8("\u2082") << "A";
			break;
		case nsl_sf_stats_tdist:
			paramNames << "n" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03bd") << "A";
			break;
		case nsl_sf_stats_beta:
		case nsl_sf_stats_pareto:
			paramNames << "a" << "b" << "A";
			break;
		case nsl_sf_stats_weibull:
			paramNames << "k" << "l" << "mu" << "a";
			paramNamesUtf8 << "k" << QString::fromUtf8("\u03bb") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gumbel1:
			paramNames << "s" << "b" << "mu" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03b2") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gumbel2:
			paramNames << "a" << "b" << "mu" << "A";
			paramNamesUtf8 << "a" << "b" << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_poisson:
			paramNames << "l" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03bb") << "A";
			break;
		case nsl_sf_stats_binomial:
		case nsl_sf_stats_negative_binomial:
		case nsl_sf_stats_pascal:
			paramNames << "p" << "n" << "a";
			paramNamesUtf8 << "p" << "n" << "A";
			break;
		case nsl_sf_stats_geometric:
		case nsl_sf_stats_logarithmic:
			paramNames << "p" << "a";
			paramNamesUtf8 << "p" << "A";
			break;
		case nsl_sf_stats_hypergeometric:
			paramNames << "n1" << "n2" << "t" << "a";
			paramNamesUtf8 << "n" + QString::fromUtf8("\u2081") << "n" + QString::fromUtf8("\u2082") << "t" << "A";
			break;
		case nsl_sf_stats_maxwell_boltzmann:
			paramNames << "s" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03c3") << "A";
			break;
		case nsl_sf_stats_frechet:
			paramNames << "g" << "mu" << "s" << "a";
			paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << QString::fromUtf8("\u03c3") << "A";
			break;
		}
		break;
	case nsl_fit_model_custom:
		break;
	}

	if (paramNamesUtf8.isEmpty())
		paramNamesUtf8 << paramNames;

	//resize the vector for the start values and set the elements to 1.0
	//in case a custom model is used, do nothing, we take over the previous values
	if (modelCategory != nsl_fit_model_custom) {
		const int np = paramNames.size();
		paramStartValues.resize(np);
		paramFixed.resize(np);
		paramLowerLimits.resize(np);
		paramUpperLimits.resize(np);

		for (int i = 0; i < np; ++i) {
			paramStartValues[i] = 1.0;
			paramFixed[i] = false;
			paramLowerLimits[i] = -DBL_MAX;
			paramUpperLimits[i] = DBL_MAX;
		}

		// set some model-dependent start values
		if (modelCategory == nsl_fit_model_distribution) {
			nsl_sf_stats_distribution type = (nsl_sf_stats_distribution)modelType;
			if (type == nsl_sf_stats_flat)
				paramStartValues[0] = -1.0;
			else if (type == nsl_sf_stats_frechet || type == nsl_sf_stats_levy || type == nsl_sf_stats_exponential_power)
				paramStartValues[1] = 0.0;
			else if (type == nsl_sf_stats_weibull || type == nsl_sf_stats_gumbel2)
				paramStartValues[2] = 0.0;
			else if (type == nsl_sf_stats_binomial || type == nsl_sf_stats_negative_binomial || type == nsl_sf_stats_pascal
				|| type == nsl_sf_stats_geometric || type == nsl_sf_stats_logarithmic)
				paramStartValues[0] = 0.5;
		}
	}
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFitCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-fit-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, yDataColumn, yDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, xErrorColumn, xErrorColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, yErrorColumn, yErrorColumn)
const QString& XYFitCurve::xDataColumnPath() const { Q_D(const XYFitCurve); return d->xDataColumnPath; }
const QString& XYFitCurve::yDataColumnPath() const { Q_D(const XYFitCurve); return d->yDataColumnPath; }
const QString& XYFitCurve::xErrorColumnPath() const { Q_D(const XYFitCurve);return d->xErrorColumnPath; }
const QString& XYFitCurve::yErrorColumnPath() const { Q_D(const XYFitCurve);return d->yErrorColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYFitCurve, XYFitCurve::FitData, fitData, fitData)

const XYFitCurve::FitResult& XYFitCurve::fitResult() const {
	Q_D(const XYFitCurve);
	return d->fitResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYFitCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYFitCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->xDataColumn) {
		exec(new XYFitCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		handleSourceDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYFitCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->yDataColumn) {
		exec(new XYFitCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		handleSourceDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetXErrorColumn, const AbstractColumn*, xErrorColumn)
void XYFitCurve::setXErrorColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->xErrorColumn) {
		exec(new XYFitCurveSetXErrorColumnCmd(d, column, i18n("%1: assign x-error")));
		handleSourceDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetYErrorColumn, const AbstractColumn*, yErrorColumn)
void XYFitCurve::setYErrorColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->yErrorColumn) {
		exec(new XYFitCurveSetYErrorColumnCmd(d, column, i18n("%1: assign y-error")));
		handleSourceDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData, recalculate);
void XYFitCurve::setFitData(const XYFitCurve::FitData& fitData) {
	Q_D(XYFitCurve);
	exec(new XYFitCurveSetFitDataCmd(d, fitData, i18n("%1: set fit options and perform the fit")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFitCurvePrivate::XYFitCurvePrivate(XYFitCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), xErrorColumn(0), yErrorColumn(0),
	xColumn(0), yColumn(0), residualsColumn(0),
	xVector(0), yVector(0), residualsVector(0),
	q(owner)  {

}

XYFitCurvePrivate::~XYFitCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

// data structure to pass parameter to fit functions
struct data {
	size_t n;	//number of data points
	double* x;	//pointer to the vector with x-data values
	double* y;	//pointer to the vector with y-data values
	double* weight;	//pointer to the vector with weight values
	nsl_fit_model_category modelCategory;
	unsigned int modelType;
	int degree;
	QString* func;	// string containing the definition of the model/function
	QStringList* paramNames;
	double* paramMin;	// lower parameter limits
	double* paramMax;	// upper parameter limits
	bool* paramFixed;	// parameter fixed?
};

/*!
 * \param paramValues vector containing current values of the fit parameters
 * \param params
 * \param f vector with the weighted residuals weight[i]*(Yi - y[i])
 */
int func_f(const gsl_vector* paramValues, void* params, gsl_vector* f) {
	size_t n = ((struct data*)params)->n;
	double* x = ((struct data*)params)->x;
	double* y = ((struct data*)params)->y;
	double* weight = ((struct data*)params)->weight;
	nsl_fit_model_category modelCategory = ((struct data*)params)->modelCategory;
	unsigned int modelType = ((struct data*)params)->modelType;
	QByteArray funcba = ((struct data*)params)->func->toLatin1();	// a local byte array is needed!
	const char *func = funcba.constData();	// function to evaluate
	QStringList* paramNames = ((struct data*)params)->paramNames;
	double *min = ((struct data*)params)->paramMin;
	double *max = ((struct data*)params)->paramMax;

	// set current values of the parameters
	for (int i = 0; i < paramNames->size(); i++) {
		double x = gsl_vector_get(paramValues, i);
		// bound values if limits are set
		QByteArray paramnameba = paramNames->at(i).toLatin1();
		assign_variable(paramnameba.constData(), nsl_fit_map_bound(x, min[i], max[i]));
		QDEBUG("Parameter"<<i<<" (\" "<<paramnameba.constData()<<"\")"<<'['<<min[i]<<','<<max[i]
			<<"] free/bound:"<<QString::number(x, 'g', 15)<<' '<<QString::number(nsl_fit_map_bound(x, min[i], max[i]), 'g', 15));
	}

	for (size_t i = 0; i < n; i++) {
		if (std::isnan(x[i]) || std::isnan(y[i]))
			continue;

		// checks for allowed values of x for different models
		// TODO: more to check
		if (modelCategory == nsl_fit_model_distribution && modelType == nsl_sf_stats_lognormal) {
			if (x[i] < 0)
				x[i] = 0;
		}

		assign_variable("x", x[i]);
		double Yi = parse(func);

//		DEBUG("evaluate function \"" << func << "\": f(x["<< i <<"]) = " << Yi);

		if (parse_errors() > 0)
			return GSL_EINVAL;

		gsl_vector_set(f, i, weight[i] * (Yi - y[i]));
	}

	return GSL_SUCCESS;
}

/*!
 * calculates the matrix elements of Jacobian matrix
 * \param paramValues current parameter values
 * \param params
 * \param J Jacobian matrix
 * */
int func_df(const gsl_vector* paramValues, void* params, gsl_matrix* J) {
	const size_t n = ((struct data*)params)->n;
	double* xVector = ((struct data*)params)->x;
	double* weight = ((struct data*)params)->weight;
	nsl_fit_model_category modelCategory = ((struct data*)params)->modelCategory;
	unsigned int modelType = ((struct data*)params)->modelType;
	int degree = ((struct data*)params)->degree;
	QStringList* paramNames = ((struct data*)params)->paramNames;
	double *min = ((struct data*)params)->paramMin;
	double *max = ((struct data*)params)->paramMax;
	bool *fixed = ((struct data*)params)->paramFixed;

	// calculate the Jacobian matrix:
	// Jacobian matrix J(i,j) = df_i / dx_j
	// where f_i = w_i*(Y_i - y_i),
	// Y_i = model and the x_j are the parameters
	double x;

	switch (modelCategory) {
	case nsl_fit_model_basic:
		switch (modelType) {
		case nsl_fit_model_polynomial:	// Y(x) = c0 + c1*x + ... + cn*x^n
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				for (int j = 0; j < paramNames->size(); ++j) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_polynomial_param_deriv(x, j, weight[i]));
				}
			}
			break;
		case nsl_fit_model_power:	// Y(x) = a*x^b or Y(x) = a + b*x^c.
			if (degree == 1) {
				const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
				const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
				for (size_t i = 0; i < n; i++) {
					x = xVector[i];

					for (int j = 0; j < 2; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_power1_param_deriv(j, x, a, b, weight[i]));
					}
				}
			} else if (degree == 2) {
				const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
				const double c = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
				for (size_t i = 0; i < n; i++) {
					x = xVector[i];

					for (int j = 0; j < 3; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_power2_param_deriv(j, x, b, c, weight[i]));
					}
				}
			}
			break;
		case nsl_fit_model_exponential:	{ // Y(x) = a*exp(b*x) + c*exp(d*x) + ...
			double *p = new double[2*degree];
			for (int i = 0; i < 2*degree; i++)
				p[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, i), min[i], max[i]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 2*degree; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_exponentialn_param_deriv(j, x, p, weight[i]));
				}
			}
			delete[] p;

			break;
		}
		case nsl_fit_model_inverse_exponential: {	// Y(x) = a*(1-exp(b*x))+c
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_inverse_exponential_param_deriv(j, x, a, b, weight[i]));
				}
			}
			break;
		}
		case nsl_fit_model_fourier: {	// Y(x) = a0 + (a1*cos(w*x) + b1*sin(w*x)) + ... + (an*cos(n*w*x) + bn*sin(n*w*x)
			//parameters: w, a0, a1, b1, ... an, bn
			double* a = new double[degree];
			double* b = new double[degree];
			double w = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			a[0] = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			b[0] = 0;
			for (int i = 1; i < degree; ++i) {
				a[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, 2*i), min[2*i], max[2*i]);
				b[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, 2*i+1), min[2*i+1], max[2*i+1]);
			}
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				double wd = 0; //first derivative with respect to the w parameter
				for (int j = 1; j < degree; ++j) {
					wd += -a[j]*j*x*sin(j*w*x) + b[j]*j*x*cos(j*w*x);
				}

				gsl_matrix_set(J, i, 0, weight[i]*wd);
				gsl_matrix_set(J, i, 1, weight[i]);
				for (int j = 1; j <= degree; ++j) {
					gsl_matrix_set(J, i, 2*j, nsl_fit_model_fourier_param_deriv(0, j, x, w, weight[i]));
					gsl_matrix_set(J, i, 2*j+1, nsl_fit_model_fourier_param_deriv(1, j, x, w, weight[i]));
				}

				for (int j = 0; j <= 2*degree+1; j++)
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
			}

			delete[] a;
			delete[] b;

			break;
        	}
		}
		break;
	case nsl_fit_model_peak:
		switch (modelType) {
		case nsl_fit_model_gaussian:
		case nsl_fit_model_lorentz:
		case nsl_fit_model_sech:
		case nsl_fit_model_logistic: {
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < degree; ++j) {
					const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j), min[3*j], max[3*j]);
					const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j+1), min[3*j+1], max[3*j+1]);
					const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j+2), min[3*j+2], max[3*j+2]);

					switch (modelType) {
					case nsl_fit_model_gaussian:
						gsl_matrix_set(J, i, 3*j, nsl_fit_model_gaussian_param_deriv(0, x, s, mu, a, weight[i]));
						gsl_matrix_set(J, i, 3*j+1, nsl_fit_model_gaussian_param_deriv(1, x, s, mu, a, weight[i]));
						gsl_matrix_set(J, i, 3*j+2, nsl_fit_model_gaussian_param_deriv(2, x, s, mu, a, weight[i]));
						break;
					case nsl_fit_model_lorentz:	// s,t,a
						gsl_matrix_set(J, i, 3*j, nsl_fit_model_lorentz_param_deriv(0, x, s, mu, a, weight[i]));
						gsl_matrix_set(J, i, 3*j+1, nsl_fit_model_lorentz_param_deriv(1, x, s, mu, a, weight[i]));
						gsl_matrix_set(J, i, 3*j+2, nsl_fit_model_lorentz_param_deriv(2, x, s, mu, a, weight[i]));
						break;
					case nsl_fit_model_sech:
						gsl_matrix_set(J, i, 3*j, nsl_fit_model_sech_param_deriv(0, x, s, mu, a, weight[i]));
						gsl_matrix_set(J, i, 3*j+1, nsl_fit_model_sech_param_deriv(1, x, s, mu, a, weight[i]));
						gsl_matrix_set(J, i, 3*j+2, nsl_fit_model_sech_param_deriv(2, x, s, mu, a, weight[i]));
						break;
					case nsl_fit_model_logistic:
						gsl_matrix_set(J, i, 3*j, nsl_fit_model_logistic_param_deriv(0, x, s, mu, a, weight[i]));
						gsl_matrix_set(J, i, 3*j+1, nsl_fit_model_logistic_param_deriv(1, x, s, mu, a, weight[i]));
						gsl_matrix_set(J, i, 3*j+2, nsl_fit_model_logistic_param_deriv(2, x, s, mu, a, weight[i]));
						break;
					}
				}

				for (int j = 0; j < 3*degree; j++)
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
			}
			break;
		}
		}
		break;
	case nsl_fit_model_growth:
		switch ((nsl_fit_model_type_growth)modelType) {
		case nsl_fit_model_atan:
		case nsl_fit_model_tanh:
		case nsl_fit_model_algebraic_sigmoid:
		case nsl_fit_model_sigmoid: 		// Y(x) = a/(1+exp(-k*(x-mu)))
		case nsl_fit_model_erf:
		case nsl_fit_model_hill:
		case nsl_fit_model_gompertz:		// Y(x) = a*exp(-b*exp(-c*x));
		case nsl_fit_model_gudermann: {
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else {
						if ((nsl_fit_model_type_growth)modelType == nsl_fit_model_atan)
							gsl_matrix_set(J, i, j, nsl_fit_model_atan_param_deriv(j, x, s, mu, a, weight[i]));
						else if ((nsl_fit_model_type_growth)modelType == nsl_fit_model_tanh)
							gsl_matrix_set(J, i, j, nsl_fit_model_tanh_param_deriv(j, x, s, mu, a, weight[i]));
						else if ((nsl_fit_model_type_growth)modelType == nsl_fit_model_algebraic_sigmoid)
							gsl_matrix_set(J, i, j, nsl_fit_model_algebraic_sigmoid_param_deriv(j, x, s, mu, a, weight[i]));
						else if ((nsl_fit_model_type_growth)modelType == nsl_fit_model_sigmoid)
							gsl_matrix_set(J, i, j, nsl_fit_model_sigmoid_param_deriv(j, x, s, mu, a, weight[i]));
						else if ((nsl_fit_model_type_growth)modelType == nsl_fit_model_erf)
							gsl_matrix_set(J, i, j, nsl_fit_model_erf_param_deriv(j, x, s, mu, a, weight[i]));
						else if ((nsl_fit_model_type_growth)modelType == nsl_fit_model_hill)
							gsl_matrix_set(J, i, j, nsl_fit_model_hill_param_deriv(j, x, s, mu, a, weight[i]));
						else if ((nsl_fit_model_type_growth)modelType == nsl_fit_model_gompertz)
							gsl_matrix_set(J, i, j, nsl_fit_model_gompertz_param_deriv(j, x, s, mu, a, weight[i]));
						else if ((nsl_fit_model_type_growth)modelType == nsl_fit_model_gudermann)
							gsl_matrix_set(J, i, j, nsl_fit_model_gudermann_param_deriv(j, x, s, mu, a, weight[i]));
					}
				}
			}
			break;
		}
		}
		break;
	case nsl_fit_model_distribution:
		switch ((nsl_sf_stats_distribution)modelType) {
		case nsl_sf_stats_gaussian:
		case nsl_sf_stats_exponential:
		case nsl_sf_stats_laplace:
		case nsl_sf_stats_cauchy_lorentz:
		case nsl_sf_stats_rayleigh_tail:
		case nsl_sf_stats_lognormal:
		case nsl_sf_stats_logistic:
		case nsl_sf_stats_sech:
		case nsl_sf_stats_levy: {
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else {
						switch (modelType) {
						case nsl_sf_stats_gaussian:
							gsl_matrix_set(J, i, j, nsl_fit_model_gaussian_param_deriv(j, x, s, mu, a, weight[i]));
							break;
						case nsl_sf_stats_exponential:
							gsl_matrix_set(J, i, j, nsl_fit_model_exponential_param_deriv(j, x, s, mu, a, weight[i]));
							break;
						case nsl_sf_stats_laplace:
							gsl_matrix_set(J, i, j, nsl_fit_model_laplace_param_deriv(j, x, s, mu, a, weight[i]));
							break;
						case nsl_sf_stats_cauchy_lorentz:
							gsl_matrix_set(J, i, j, nsl_fit_model_lorentz_param_deriv(j, x, s, mu, a, weight[i]));
							break;
						case nsl_sf_stats_rayleigh_tail:
							gsl_matrix_set(J, i, j, nsl_fit_model_rayleigh_tail_param_deriv(j, x, s, mu, a, weight[i]));
							break;
						case nsl_sf_stats_lognormal:
							if (x > 0)
								gsl_matrix_set(J, i, j, nsl_fit_model_lognormal_param_deriv(j, x, s, mu, a, weight[i]));
							else
								gsl_matrix_set(J, i, j, 0.);
							break;
						case nsl_sf_stats_logistic:
							gsl_matrix_set(J, i, j, nsl_fit_model_logistic_param_deriv(j, x, s, mu, a, weight[i]));
							break;
						case nsl_sf_stats_sech:
							gsl_matrix_set(J, i, j, nsl_fit_model_sech_dist_param_deriv(j, x, s, mu, a, weight[i]));
							break;
						case nsl_sf_stats_levy:
							gsl_matrix_set(J, i, j, nsl_fit_model_levy_param_deriv(j, x, s, mu, a, weight[i]));
							break;
						}
					}
				}
			}
			break;
		}
		case nsl_sf_stats_gaussian_tail: {
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double A = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_gaussian_tail_param_deriv(j, x, s, mu, A, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_exponential_power: {
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_exp_pow_param_deriv(j, x, s, mu, b, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_rayleigh: {
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_rayleigh_param_deriv(j, x, s, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_gamma: {
			const double t = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double k = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_gamma_param_deriv(j, x, t, k, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_flat: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double A = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_flat_param_deriv(j, x, a, b, A, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_chi_squared: {
			const double n = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_chi_square_param_deriv(j, x, n, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_tdist: {
			const double n = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_students_t_param_deriv(j, x, n, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_fdist: {
			const double n1 = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double n2 = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_fdist_param_deriv(j, x, n1, n2, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_beta:
		case nsl_sf_stats_pareto: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double A = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else {
						switch (modelType) {
						case nsl_sf_stats_beta:
							gsl_matrix_set(J, i, j, nsl_fit_model_beta_param_deriv(j, x, a, b, A, weight[i]));
							break;
						case nsl_sf_stats_pareto:
							gsl_matrix_set(J, i, j, nsl_fit_model_pareto_param_deriv(j, x, a, b, A, weight[i]));
							break;
						}
					}
				}
			}
			break;
		}
		case nsl_sf_stats_weibull: {
			const double k = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double l = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else {
						if (x > 0)
							gsl_matrix_set(J, i, j, nsl_fit_model_weibull_param_deriv(j, x, k, l, mu, a, weight[i]));
						else
							gsl_matrix_set(J, i, j, 0.);
					}
				}
			}
			break;
		}
		case nsl_sf_stats_gumbel1: {
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_gumbel1_param_deriv(j, x, s, b, mu, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_gumbel2: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double A = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_gumbel2_param_deriv(j, x, a, b, mu, A, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_poisson: {
			const double l = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_poisson_param_deriv(j, x, l, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_maxwell_boltzmann: {	// Y(x) = a*sqrt(2/pi) * x^2/s^3 * exp(-(x/s)^2/2)
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_maxwell_param_deriv(j, x, s, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_frechet: {
			const double g = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_frechet_param_deriv(j, x, g, mu, s, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_landau: {
			// const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (fixed[0])
					gsl_matrix_set(J, i, 0, 0.);
				else
					gsl_matrix_set(J, i, 0, nsl_fit_model_landau_param_deriv(0, x, weight[i]));
			}
			break;
		}
		case nsl_sf_stats_binomial:
		case nsl_sf_stats_negative_binomial:
		case nsl_sf_stats_pascal: {
			const double p = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double N = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else {
						switch (modelType) {
						case nsl_sf_stats_binomial:
							gsl_matrix_set(J, i, j, nsl_fit_model_binomial_param_deriv(j, x, p, N, a, weight[i]));
							break;
						case nsl_sf_stats_negative_binomial:
							gsl_matrix_set(J, i, j, nsl_fit_model_negative_binomial_param_deriv(j, x, p, N, a, weight[i]));
							break;
						case nsl_sf_stats_pascal:
							gsl_matrix_set(J, i, j, nsl_fit_model_pascal_param_deriv(j, x, p, N, a, weight[i]));
							break;
						}
					}
				}
			}
			break;
		}
		case nsl_sf_stats_geometric:
		case nsl_sf_stats_logarithmic: {
			const double p = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else {
						switch (modelType) {
						case nsl_sf_stats_geometric:
							gsl_matrix_set(J, i, j, nsl_fit_model_geometric_param_deriv(j, x, p, a, weight[i]));
							break;
						case nsl_sf_stats_logarithmic:
							gsl_matrix_set(J, i, j, nsl_fit_model_logarithmic_param_deriv(j, x, p, a, weight[i]));
							break;
						}
					}
				}
			}
			break;
		}
		case nsl_sf_stats_hypergeometric: {
			const double n1 = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double n2 = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double t = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_hypergeometric_param_deriv(j, x, n1, n2, t, a, weight[i]));
				}
			}
			break;
		}
		// unused distributions
		case nsl_sf_stats_levy_alpha_stable:
		case nsl_sf_stats_levy_skew_alpha_stable:
		case nsl_sf_stats_bernoulli:
			break;
		}
		break;
	case nsl_fit_model_custom:
		QByteArray funcba = ((struct data*)params)->func->toLatin1();
		const char* func = funcba.data();
		QByteArray nameba;
		double value;
		const unsigned int np = paramNames->size();
		for (size_t i = 0; i < n; i++) {
			x = xVector[i];
			assign_variable("x", x);

			for (unsigned int j = 0; j < np; j++) {
				for (unsigned int k = 0; k < np; k++) {
					if (k != j) {
						nameba = paramNames->at(k).toLatin1();
						value = nsl_fit_map_bound(gsl_vector_get(paramValues, k), min[k], max[k]);
						assign_variable(nameba.data(), value);
					}
				}

				nameba = paramNames->at(j).toLatin1();
				const char *name = nameba.data();
				value = nsl_fit_map_bound(gsl_vector_get(paramValues, j), min[j], max[j]);
				assign_variable(name, value);
				const double f_p = parse(func);

				const double eps = 1.e-9 * fabs(f_p);	// adapt step size to value
				value += eps;
				assign_variable(name, value);
				const double f_pdp = parse(func);

//		qDebug()<<"evaluate deriv"<<QString(func)<<": f(x["<<i<<"]) ="<<QString::number(f_p, 'g', 15);
//		qDebug()<<"evaluate deriv"<<QString(func)<<": f(x["<<i<<"]+dx) ="<<QString::number(f_pdp, 'g', 15);
//		qDebug()<<"	deriv = "<<QString::number((f_pdp-f_p)/eps/sigma, 'g', 15);

				if (fixed[j])
					gsl_matrix_set(J, i, j, 0.);
				else	// calculate finite difference
					gsl_matrix_set(J, i, j, weight[i]*(f_pdp - f_p)/eps);
			}
		}
	}

	return GSL_SUCCESS;
}

int func_fdf(const gsl_vector* x, void* params, gsl_vector* f, gsl_matrix* J) {
	func_f(x, params, f);
	func_df(x, params, J);

	return GSL_SUCCESS;
}

void XYFitCurvePrivate::recalculate() {
	DEBUG("XYFitCurvePrivate::recalculate()");
	QElapsedTimer timer;
	timer.start();

	//create fit result columns if not available yet, clear them otherwise
	if (!xColumn) {
		xColumn = new Column("x", AbstractColumn::Numeric);
		yColumn = new Column("y", AbstractColumn::Numeric);
		residualsColumn = new Column("residuals", AbstractColumn::Numeric);
		xVector = static_cast<QVector<double>* >(xColumn->data());
		yVector = static_cast<QVector<double>* >(yColumn->data());
		residualsVector = static_cast<QVector<double>* >(residualsColumn->data());

		xColumn->setHidden(true);
		q->addChild(xColumn);

		yColumn->setHidden(true);
		q->addChild(yColumn);

		q->addChild(residualsColumn);

		q->setUndoAware(false);
		q->setXColumn(xColumn);
		q->setYColumn(yColumn);
		q->setUndoAware(true);
	} else {
		xVector->clear();
		yVector->clear();
		residualsVector->clear();
	}

	// clear the previous result
	fitResult = XYFitCurve::FitResult();

	//fit settings
	const unsigned int maxIters = fitData.maxIterations;	//maximal number of iterations
	const double delta = fitData.eps;		//fit tolerance
	const unsigned int np = fitData.paramNames.size(); //number of fit parameters
	if (np == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Model has no parameters.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//determine the data source columns
	const AbstractColumn* tmpXDataColumn = 0;
	const AbstractColumn* tmpYDataColumn = 0;
	if (dataSourceType == XYCurve::DataSourceSpreadsheet) {
		//spreadsheet columns as data source
		tmpXDataColumn = xDataColumn;
		tmpYDataColumn = yDataColumn;
	} else {
		//curve columns as data source
		tmpXDataColumn = dataSourceCurve->xColumn();
		tmpYDataColumn = dataSourceCurve->yColumn();
	}

	if (!tmpXDataColumn || !tmpYDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//check column sizes
	if (tmpXDataColumn->rowCount() != tmpYDataColumn->rowCount()) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	if (yErrorColumn) {
		if (yErrorColumn->rowCount() < xDataColumn->rowCount()) {
			fitResult.available = true;
			fitResult.valid = false;
			fitResult.status = i18n("Not sufficient weight data points provided.");
			emit (q->dataChanged());
			sourceDataChangedSinceLastRecalc = false;
			return;
		}
	}

	//copy all valid data point for the fit to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	QVector<double> xerrorVector;
	QVector<double> yerrorVector;
	double xmin, xmax;
	if (fitData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = fitData.xRange.first();
		xmax = fitData.xRange.last();
	}

	for (int row = 0; row < tmpXDataColumn->rowCount(); ++row) {
		//only copy those data where _all_ values (for x and y and errors, if given) are valid
		if (!std::isnan(tmpXDataColumn->valueAt(row)) && !std::isnan(tmpYDataColumn->valueAt(row))
			&& !tmpXDataColumn->isMasked(row) && !tmpYDataColumn->isMasked(row)) {

			// only when inside given range
			if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
				if (dataSourceType == XYCurve::DataSourceCurve || (!xErrorColumn && !yErrorColumn) || !fitData.useDataErrors) {	// x-y
					xdataVector.append(tmpXDataColumn->valueAt(row));
					ydataVector.append(tmpYDataColumn->valueAt(row));
				} else if (!xErrorColumn) {		// x-y-dy
					if (!std::isnan(yErrorColumn->valueAt(row))) {
						xdataVector.append(tmpXDataColumn->valueAt(row));
						ydataVector.append(tmpYDataColumn->valueAt(row));
						yerrorVector.append(yErrorColumn->valueAt(row));
					}
				} else {				// x-y-dx-dy
					if (!std::isnan(xErrorColumn->valueAt(row)) && !std::isnan(yErrorColumn->valueAt(row))) {
						xdataVector.append(tmpXDataColumn->valueAt(row));
						ydataVector.append(tmpYDataColumn->valueAt(row));
						xerrorVector.append(xErrorColumn->valueAt(row));
						yerrorVector.append(yErrorColumn->valueAt(row));
					}
				}
			}
		}
	}
	//number of data points to fit
	const size_t n = xdataVector.size();
	DEBUG("number of data points: " << n);
	if (n == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("No data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	if (n < np) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("The number of data points (%1) must be greater than or equal to the number of parameters (%2).", n, np);
		emit (q->dataChanged());
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* xerror = xerrorVector.data();	// size may be 0
	double* yerror = yerrorVector.data();	// size may be 0
	DEBUG("x errors: " << xerrorVector.size());
	DEBUG("y errors: " << yerrorVector.size());
	double* weight = new double[n];

	for (size_t i = 0; i < n; i++)
		weight[i] = 1.;

	switch (fitData.weightsType) {
	case nsl_fit_weight_no:
		break;
	case nsl_fit_weight_instrumental:
		if (yerrorVector.size() > 0)
			for(size_t i = 0; i < n; i++)
				weight[i] = 1./gsl_pow_2(yerror[i]);
		break;
	case nsl_fit_weight_direct:
		if (yerrorVector.size() > 0)
			for(size_t i = 0; i < n; i++)
				weight[i] = yerror[i];
		break;
	case nsl_fit_weight_inverse:
		if (yerrorVector.size() > 0)
			for(size_t i = 0; i < n; i++)
				weight[i] = 1./yerror[i];
		break;
	case nsl_fit_weight_statistical:
		for (size_t i = 0; i < n; i++)
			weight[i] = 1./ydata[i];
		break;
	case nsl_fit_weight_relative:
		for (size_t i = 0; i < n; i++)
			weight[i] = 1./gsl_pow_2(ydata[i]);
		break;
	case nsl_fit_weight_statistical_fit:
	case nsl_fit_weight_relative_fit:
		break;
	}

	/////////////////////// GSL >= 2 has a complete new interface! But the old one is still supported. ///////////////////////////
	// GSL >= 2 : "the 'fdf' field of gsl_multifit_function_fdf is now deprecated and does not need to be specified for nonlinear least squares problems"
	for (unsigned int i = 0; i < np; i++)
		DEBUG("parameter " << i << " fixed: " << fitData.paramFixed.data()[i]);

	//function to fit
	gsl_multifit_function_fdf f;
	DEBUG("model = " << fitData.model.toStdString());
	struct data params = {n, xdata, ydata, weight, fitData.modelCategory, fitData.modelType, fitData.degree, &fitData.model, &fitData.paramNames, fitData.paramLowerLimits.data(), fitData.paramUpperLimits.data(), fitData.paramFixed.data()};
	f.f = &func_f;
	f.df = &func_df;
	f.fdf = &func_fdf;
	f.n = n;
	f.p = np;
	f.params = &params;

	// initialize the derivative solver (using Levenberg-Marquardt robust solver)
	const gsl_multifit_fdfsolver_type* T = gsl_multifit_fdfsolver_lmsder;
	gsl_multifit_fdfsolver* s = gsl_multifit_fdfsolver_alloc(T, n, np);

	// set start values
	double* x_init = fitData.paramStartValues.data();
	double* x_min = fitData.paramLowerLimits.data();
	double* x_max = fitData.paramUpperLimits.data();
	// scale start values if limits are set
	for (unsigned int i = 0; i < np; i++)
		x_init[i] = nsl_fit_map_unbound(x_init[i], x_min[i], x_max[i]);
	gsl_vector_view x = gsl_vector_view_array(x_init, np);
	// initialize solver with function f and initial guess x
	gsl_multifit_fdfsolver_set(s, &f, &x.vector);

	// iterate
	int status;
	unsigned int iter = 0;
	fitResult.solverOutput.clear();
	writeSolverState(s);
	do {
		iter++;

		// update weights for Y-depending weights
		if (fitData.weightsType == nsl_fit_weight_statistical_fit) {
			for (size_t i = 0; i < n; i++)
				weight[i] = 1./(gsl_vector_get(s->f, i) + ydata[i]);	// 1/Y_i
		} else if (fitData.weightsType == nsl_fit_weight_relative_fit) {
			for (size_t i = 0; i < n; i++)
				weight[i] = 1./gsl_pow_2(gsl_vector_get(s->f, i) + ydata[i]);	// 1/Y_i^2
		}

		status = gsl_multifit_fdfsolver_iterate(s);
		writeSolverState(s);
		if (status) {
			DEBUG("iter " << iter << ", status = " << gsl_strerror(status));
			break;
		}
		status = gsl_multifit_test_delta(s->dx, s->x, delta, delta);
	} while (status == GSL_CONTINUE && iter < maxIters);

	// second run for x-error fitting
	if (xerrorVector.size() > 0) {
		DEBUG("Rerun fit with x errors");

		// y'(x)
		double *yd = new double[n];
		for (size_t i = 0; i < n; i++) {
			size_t index = i;
			if (index == n-1)
				index = n-2;
			yd[i] = gsl_vector_get(s->f, index+1) + ydata[index+1] - gsl_vector_get(s->f, index) - ydata[index];
			yd[i] /= (xdata[index+1] - xdata[index]);
		}

		switch (fitData.weightsType) {
		case nsl_fit_weight_no:
			break;
		case nsl_fit_weight_instrumental:
			for (size_t i = 0; i < n; i++) {
				double sigma;
				if (yerrorVector.size() > 0)	// x- and y-error
					// sigma = sqrt(sigma_y^2 + (y'(x)*sigma_x)^2)
					sigma = sqrt(gsl_pow_2(yerror[i]) + gsl_pow_2(yd[i] * xerror[i]));
				else	// only x-error
					sigma = yd[i] * xerror[i];
				weight[i] = 1./gsl_pow_2(sigma);
			}
			break;
		// other weight types: y'(x) considered correctly?
		case nsl_fit_weight_direct:
			for (size_t i = 0; i < n; i++) {
				weight[i] = xerror[i]/yd[i];
				if (yerrorVector.size() > 0)
					weight[i] += yerror[i];
			}
			break;
		case nsl_fit_weight_inverse:
			for (size_t i = 0; i < n; i++) {
				weight[i] = yd[i]/xerror[i];
				if (yerrorVector.size() > 0)
					weight[i] += 1./yerror[i];
			}
			break;
		case nsl_fit_weight_statistical:
		case nsl_fit_weight_relative:
			break;
		case nsl_fit_weight_statistical_fit:
			for (size_t i = 0; i < n; i++)
				weight[i] = 1./(gsl_vector_get(s->f, i) + ydata[i]);	// 1/Y_i
			break;
		case nsl_fit_weight_relative_fit:
			for (size_t i = 0; i < n; i++)
				weight[i] = 1./gsl_pow_2(gsl_vector_get(s->f, i) + ydata[i]);	// 1/Y_i^2
			break;
		}
		delete[] yd;

		do {
			iter++;
			status = gsl_multifit_fdfsolver_iterate(s);
			writeSolverState(s);
			if (status) break;
			status = gsl_multifit_test_delta(s->dx, s->x, delta, delta);
		} while (status == GSL_CONTINUE && iter < maxIters);
	}

	delete[] weight;

	// unscale start values
	for (unsigned int i = 0; i < np; i++)
		x_init[i] = nsl_fit_map_bound(x_init[i], x_min[i], x_max[i]);

	//get the covariance matrix
	//TODO: scale the Jacobian when limits are used before constructing the covar matrix?
	gsl_matrix* covar = gsl_matrix_alloc(np, np);
#if GSL_MAJOR_VERSION >= 2
	// the Jacobian is not part of the solver anymore
	gsl_matrix *J = gsl_matrix_alloc(s->fdf->n, s->fdf->p);
	gsl_multifit_fdfsolver_jac(s, J);
	gsl_multifit_covar(J, 0.0, covar);
#else
	gsl_multifit_covar(s->J, 0.0, covar);
#endif

	//write the result
	fitResult.available = true;
	fitResult.valid = true;
	fitResult.status = QString(gsl_strerror(status)); // i18n? GSL does not support translations
	fitResult.iterations = iter;
	fitResult.dof = n - np;

	//gsl_blas_dnrm2() - computes the Euclidian norm (||r||_2 = \sqrt {\sum r_i^2}) of the vector with the elements weight[i]*(Yi - y[i])
	//gsl_blas_dasum() - computes the absolute sum \sum |r_i| of the elements of the vector with the elements weight[i]*(Yi - y[i])
	fitResult.sse = gsl_pow_2(gsl_blas_dnrm2(s->f));
	if (fitResult.dof != 0) {
		fitResult.rms = fitResult.sse/fitResult.dof;
		fitResult.rsd = sqrt(fitResult.rms);
	}
	fitResult.mse = fitResult.sse/n;
	fitResult.rmse = sqrt(fitResult.mse);
	fitResult.mae = gsl_blas_dasum(s->f)/n;
	//needed for coefficient of determination, R-squared
	fitResult.sst = gsl_stats_tss(ydata, 1, n);

	fitResult.rsquare = nsl_stats_rsquare(fitResult.sse, fitResult.sst);
	fitResult.rsquareAdj = nsl_stats_rsquareAdj(fitResult.rsquare, np, fitResult.dof);
	fitResult.chisq_p = nsl_stats_chisq_p(fitResult.sse, fitResult.dof);
	fitResult.fdist_F = nsl_stats_fdist_F(fitResult.sst, fitResult.rms);
	fitResult.fdist_p = nsl_stats_fdist_p(fitResult.fdist_F, np, fitResult.dof);
	fitResult.aic = nsl_stats_aic(fitResult.sse, n, np);
	fitResult.bic = nsl_stats_bic(fitResult.sse, n, np);

	//parameter values
	const double c = GSL_MIN_DBL(1., sqrt(fitResult.rms)); //limit error for poor fit
	fitResult.paramValues.resize(np);
	fitResult.errorValues.resize(np);
	fitResult.tdist_tValues.resize(np);
	fitResult.tdist_pValues.resize(np);
	fitResult.tdist_marginValues.resize(np);
	for (unsigned int i = 0; i < np; i++) {
		// scale resulting values if they are bounded
		fitResult.paramValues[i] = nsl_fit_map_bound(gsl_vector_get(s->x, i), x_min[i], x_max[i]);
		// use results as start values if desired
		if (fitData.useResults) {
			fitData.paramStartValues.data()[i] = fitResult.paramValues[i];
			DEBUG("saving parameter " << i << ": " << fitResult.paramValues[i] << ' ' << fitData.paramStartValues.data()[i]);
		}
		fitResult.errorValues[i] = c*sqrt(gsl_matrix_get(covar, i, i));
		fitResult.tdist_tValues[i] = nsl_stats_tdist_t(fitResult.paramValues.at(i), fitResult.errorValues.at(i));
		fitResult.tdist_pValues[i] = nsl_stats_tdist_p(fitResult.tdist_tValues.at(i), fitResult.dof);
		fitResult.tdist_marginValues[i] = nsl_stats_tdist_margin(0.05, fitResult.dof, fitResult.errorValues.at(i));
	}

	// fill residuals vector. To get residuals on the correct x values, fill the rest with zeros.
	residualsVector->resize(tmpXDataColumn->rowCount());
	if (fitData.evaluateFullRange) {	// evaluate full range of residuals
		xVector->resize(tmpXDataColumn->rowCount());
		for (int i = 0; i < tmpXDataColumn->rowCount(); i++)
			(*xVector)[i] = tmpXDataColumn->valueAt(i);
		ExpressionParser* parser = ExpressionParser::getInstance();
		bool rc = parser->evaluateCartesian(fitData.model, xVector, residualsVector,
							fitData.paramNames, fitResult.paramValues);
		for (int i = 0; i < tmpXDataColumn->rowCount(); i++)
			(*residualsVector)[i] = tmpYDataColumn->valueAt(i) - (*residualsVector)[i];
		if (!rc)
			residualsVector->clear();
	} else {	// only selected range
		size_t j = 0;
		for (int i = 0; i < tmpXDataColumn->rowCount(); i++) {
			if (tmpXDataColumn->valueAt(i) >= xmin && tmpXDataColumn->valueAt(i) <= xmax)
				residualsVector->data()[i] = - gsl_vector_get(s->f, j++);
			else	// outside range
				residualsVector->data()[i] = 0;
		}
	}
	residualsColumn->setChanged();

	//free resources
	gsl_multifit_fdfsolver_free(s);
	gsl_matrix_free(covar);

	//calculate the fit function (vectors)
	ExpressionParser* parser = ExpressionParser::getInstance();
	if (fitData.evaluateFullRange) { // evaluate fit on full data range if selected
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	}
	xVector->resize(fitData.evaluatedPoints);
	yVector->resize(fitData.evaluatedPoints);
	bool rc = parser->evaluateCartesian(fitData.model, QString::number(xmin), QString::number(xmax), fitData.evaluatedPoints, xVector, yVector,
						fitData.paramNames, fitResult.paramValues);
	if (!rc) {
		xVector->clear();
		yVector->clear();
	}

	fitResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastRecalc = false;
}

/*!
 * writes out the current state of the solver \c s
 */
void XYFitCurvePrivate::writeSolverState(gsl_multifit_fdfsolver* s) {
	QString state;

	//current parameter values, semicolon separated
	double* min = fitData.paramLowerLimits.data();
	double* max = fitData.paramUpperLimits.data();
	for (int i = 0; i < fitData.paramNames.size(); ++i) {
		const double x = gsl_vector_get(s->x, i);
		// map parameter if bounded
		state += QString::number(nsl_fit_map_bound(x, min[i], max[i])) + '\t';
	}

	//current value of the chi2-function
	state += QString::number(gsl_pow_2(gsl_blas_dnrm2(s->f)));
	state += ';';

	fitResult.solverOutput += state;
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFitCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFitCurve);

	writer->writeStartElement("xyFitCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-fit-curve specific information

	//fit data - only save model expression and parameter names for custom model, otherwise they are set in XYFitCurve::initFitData()
	writer->writeStartElement("fitData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	WRITE_COLUMN(d->xErrorColumn, xErrorColumn);
	WRITE_COLUMN(d->yErrorColumn, yErrorColumn);
	writer->writeAttribute("autoRange", QString::number(d->fitData.autoRange));
	writer->writeAttribute("xRangeMin", QString::number(d->fitData.xRange.first(), 'g', 15));
	writer->writeAttribute("xRangeMax", QString::number(d->fitData.xRange.last(), 'g', 15));
	writer->writeAttribute("modelCategory", QString::number(d->fitData.modelCategory));
	writer->writeAttribute("modelType", QString::number(d->fitData.modelType));
	writer->writeAttribute("weightsType", QString::number(d->fitData.weightsType));
	writer->writeAttribute("degree", QString::number(d->fitData.degree));
	if (d->fitData.modelCategory == nsl_fit_model_custom)
		writer->writeAttribute("model", d->fitData.model);
	writer->writeAttribute("maxIterations", QString::number(d->fitData.maxIterations));
	writer->writeAttribute("eps", QString::number(d->fitData.eps, 'g', 15));
	writer->writeAttribute("evaluatedPoints", QString::number(d->fitData.evaluatedPoints));
	writer->writeAttribute("evaluateFullRange", QString::number(d->fitData.evaluateFullRange));
	writer->writeAttribute("useDataErrors", QString::number(d->fitData.useDataErrors));
	writer->writeAttribute("useResults", QString::number(d->fitData.useResults));

	if (d->fitData.modelCategory == nsl_fit_model_custom) {
		writer->writeStartElement("paramNames");
		foreach (const QString &name, d->fitData.paramNames)
			writer->writeTextElement("name", name);
		writer->writeEndElement();
	}

	writer->writeStartElement("paramStartValues");
	foreach (const double &value, d->fitData.paramStartValues)
		writer->writeTextElement("startValue", QString::number(value, 'g', 15));
	writer->writeEndElement();

	// use 16 digits to handle -DBL_MAX
	writer->writeStartElement("paramLowerLimits");
	foreach (const double &limit, d->fitData.paramLowerLimits)
		writer->writeTextElement("lowerLimit", QString::number(limit, 'g', 16));
	writer->writeEndElement();

	// use 16 digits to handle DBL_MAX
	writer->writeStartElement("paramUpperLimits");
	foreach (const double &limit, d->fitData.paramUpperLimits)
		writer->writeTextElement("upperLimit", QString::number(limit, 'g', 16));
	writer->writeEndElement();

	writer->writeStartElement("paramFixed");
	foreach (const double &fixed, d->fitData.paramFixed)
		writer->writeTextElement("fixed", QString::number(fixed));
	writer->writeEndElement();

	writer->writeEndElement();

	//fit results (generated columns and goodness of the fit)
	writer->writeStartElement("fitResult");
	writer->writeAttribute("available", QString::number(d->fitResult.available));
	writer->writeAttribute("valid", QString::number(d->fitResult.valid));
	writer->writeAttribute("status", d->fitResult.status);
	writer->writeAttribute("iterations", QString::number(d->fitResult.iterations));
	writer->writeAttribute("time", QString::number(d->fitResult.elapsedTime));
	writer->writeAttribute("dof", QString::number(d->fitResult.dof));
	writer->writeAttribute("sse", QString::number(d->fitResult.sse, 'g', 15));
	writer->writeAttribute("sst", QString::number(d->fitResult.sst, 'g', 15));
	writer->writeAttribute("rms", QString::number(d->fitResult.rms, 'g', 15));
	writer->writeAttribute("rsd", QString::number(d->fitResult.rsd, 'g', 15));
	writer->writeAttribute("mse", QString::number(d->fitResult.mse, 'g', 15));
	writer->writeAttribute("rmse", QString::number(d->fitResult.rmse, 'g', 15));
	writer->writeAttribute("mae", QString::number(d->fitResult.mae, 'g', 15));
	writer->writeAttribute("rsquare", QString::number(d->fitResult.rsquare, 'g', 15));
	writer->writeAttribute("rsquareAdj", QString::number(d->fitResult.rsquareAdj, 'g', 15));
	writer->writeAttribute("chisq_p", QString::number(d->fitResult.chisq_p, 'g', 15));
	writer->writeAttribute("fdist_F", QString::number(d->fitResult.fdist_F, 'g', 15));
	writer->writeAttribute("fdist_p", QString::number(d->fitResult.fdist_p, 'g', 15));
	writer->writeAttribute("aic", QString::number(d->fitResult.aic, 'g', 15));
	writer->writeAttribute("bic", QString::number(d->fitResult.bic, 'g', 15));
	writer->writeAttribute("solverOutput", d->fitResult.solverOutput);

	writer->writeStartElement("paramValues");
	foreach (const double &value, d->fitResult.paramValues)
		writer->writeTextElement("value", QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeStartElement("errorValues");
	foreach (const double &value, d->fitResult.errorValues)
		writer->writeTextElement("error", QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeStartElement("tdist_tValues");
	foreach (const double &value, d->fitResult.tdist_tValues)
		writer->writeTextElement("tdist_t", QString::number(value, 'g', 15));

	writer->writeEndElement();
	writer->writeStartElement("tdist_pValues");
	foreach (const double &value, d->fitResult.tdist_pValues)
		writer->writeTextElement("tdist_p", QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeEndElement();
	writer->writeStartElement("tdist_marginValues");
	foreach (const double &value, d->fitResult.tdist_marginValues)
		writer->writeTextElement("tdist_margin", QString::number(value, 'g', 15));
	writer->writeEndElement();

	//save calculated columns if available
	if (d->xColumn && d->yColumn && d->residualsColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
		d->residualsColumn->save(writer);
	}

	writer->writeEndElement(); //"fitResult"
	writer->writeEndElement(); //"xyFitCurve"
}

//! Load from XML
bool XYFitCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYFitCurve);

	if (!reader->isStartElement() || reader->name() != "xyFitCurve") {
		reader->raiseError(i18n("no xy fit curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyFitCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader, preview) )
				return false;
			if (preview)
				return true;
		} else if (reader->name() == "fitData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			READ_COLUMN(xErrorColumn);
			READ_COLUMN(yErrorColumn);

			READ_INT_VALUE("autoRange", fitData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", fitData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", fitData.xRange.last());
			READ_INT_VALUE("modelCategory", fitData.modelCategory, nsl_fit_model_category);
			READ_INT_VALUE("modelType", fitData.modelType, unsigned int);
			READ_INT_VALUE("weightsType", fitData.weightsType, nsl_fit_weight_type);
			READ_INT_VALUE("degree", fitData.degree, int);
			if (d->fitData.modelCategory == nsl_fit_model_custom) {
				READ_STRING_VALUE("model", fitData.model);
				DEBUG("read model = " << d->fitData.model.toStdString());
			}
			READ_INT_VALUE("maxIterations", fitData.maxIterations, int);
			READ_DOUBLE_VALUE("eps", fitData.eps);
			READ_INT_VALUE("fittedPoints", fitData.evaluatedPoints, size_t);	// old name
			READ_INT_VALUE("evaluatedPoints", fitData.evaluatedPoints, size_t);
			READ_INT_VALUE("evaluateFullRange", fitData.evaluateFullRange, bool);
			READ_INT_VALUE("useDataErrors", fitData.useDataErrors, bool);
			READ_INT_VALUE("useResults", fitData.useResults, bool);

			//set the model expression and the parameter names (can be derived from the saved values for category, type and degree)
			XYFitCurve::initFitData(d->fitData);
		} else if (reader->name() == "name") {	// needed for custom model
			d->fitData.paramNames << reader->readElementText();
		} else if (reader->name() == "startValue") {
			d->fitData.paramStartValues << reader->readElementText().toDouble();
		} else if (reader->name() == "fixed") {
			d->fitData.paramFixed << (bool)reader->readElementText().toInt();
		} else if (reader->name() == "lowerLimit") {
			bool ok;
			double x = reader->readElementText().toDouble(&ok);
			if (ok)	// -DBL_MAX results in conversion error
				d->fitData.paramLowerLimits << x;
			else
				d->fitData.paramLowerLimits << -DBL_MAX;
		} else if (reader->name() == "upperLimit") {
			bool ok;
			double x = reader->readElementText().toDouble(&ok);
			if (ok)	// DBL_MAX results in conversion error
				d->fitData.paramUpperLimits << x;
			else
				d->fitData.paramUpperLimits << DBL_MAX;
		} else if (reader->name() == "value") {
			d->fitResult.paramValues << reader->readElementText().toDouble();
		} else if (reader->name() == "error") {
			d->fitResult.errorValues << reader->readElementText().toDouble();
		} else if (reader->name() == "tdist_t") {
			d->fitResult.tdist_tValues << reader->readElementText().toDouble();
		} else if (reader->name() == "tdist_p") {
			d->fitResult.tdist_tValues << reader->readElementText().toDouble();
		} else if (reader->name() == "tdist_margin") {
			d->fitResult.tdist_marginValues << reader->readElementText().toDouble();
		} else if (reader->name() == "fitResult") {
			attribs = reader->attributes();

			READ_INT_VALUE("available", fitResult.available, int);
			READ_INT_VALUE("valid", fitResult.valid, int);
			READ_STRING_VALUE("status", fitResult.status);
			READ_INT_VALUE("iterations", fitResult.iterations, int);
			READ_INT_VALUE("time", fitResult.elapsedTime, int);
			READ_DOUBLE_VALUE("dof", fitResult.dof);
			READ_DOUBLE_VALUE("sse", fitResult.sse);
			READ_DOUBLE_VALUE("sst", fitResult.sst);
			READ_DOUBLE_VALUE("rms", fitResult.rms);
			READ_DOUBLE_VALUE("rsd", fitResult.rsd);
			READ_DOUBLE_VALUE("mse", fitResult.mse);
			READ_DOUBLE_VALUE("rmse", fitResult.rmse);
			READ_DOUBLE_VALUE("mae", fitResult.mae);
			READ_DOUBLE_VALUE("rsquare", fitResult.rsquare);
			READ_DOUBLE_VALUE("rsquareAdj", fitResult.rsquareAdj);
			READ_DOUBLE_VALUE("chisq_p", fitResult.chisq_p);
			READ_DOUBLE_VALUE("fdist_F", fitResult.fdist_F);
			READ_DOUBLE_VALUE("fdist_p", fitResult.fdist_p);
			READ_DOUBLE_VALUE("aic", fitResult.aic);
			READ_DOUBLE_VALUE("bic", fitResult.bic);
			READ_STRING_VALUE("solverOutput", fitResult.solverOutput);
		} else if (reader->name() == "column") {
			Column* column = new Column("", AbstractColumn::Numeric);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			if (column->name() == "x")
				d->xColumn = column;
			else if (column->name() == "y")
				d->yColumn = column;
			else if (column->name() == "residuals")
				d->residualsColumn = column;
		}
	}

	// new fit model style (reset model type of old projects)
	if (d->fitData.modelCategory == nsl_fit_model_basic && d->fitData.modelType >= NSL_FIT_MODEL_BASIC_COUNT) {
		d->fitData.modelType = 0;
		d->fitData.degree = 1;
		// reset size of fields not touched by initFitData()
		d->fitData.paramStartValues.resize(2);
		d->fitData.paramFixed.resize(2);
		d->fitResult.paramValues.resize(2);
		d->fitResult.errorValues.resize(2);
		d->fitResult.tdist_tValues.resize(2);
		d->fitResult.tdist_pValues.resize(2);
		d->fitResult.tdist_marginValues.resize(2);
	}

	// wait for data to be read before using the pointers
	QThreadPool::globalInstance()->waitForDone();

	if (d->xColumn && d->yColumn && d->residualsColumn) {
		d->xColumn->setHidden(true);
		addChild(d->xColumn);

		d->yColumn->setHidden(true);
		addChild(d->yColumn);

		addChild(d->residualsColumn);

		d->xVector = static_cast<QVector<double>* >(d->xColumn->data());
		d->yVector = static_cast<QVector<double>* >(d->yColumn->data());
		d->residualsVector = static_cast<QVector<double>* >(d->residualsColumn->data());

		XYCurve::d_ptr->xColumn = d->xColumn;
		XYCurve::d_ptr->yColumn = d->yColumn;
	}

	return true;
}
