
/***************************************************************************
    File                 : XYFitCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a fit model
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016-2020 Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "backend/gsl/errors.h"
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

#include <QDateTime>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

XYFitCurve::XYFitCurve(const QString& name)
	: XYAnalysisCurve(name, new XYFitCurvePrivate(this), AspectType::XYFitCurve) {
}

XYFitCurve::XYFitCurve(const QString& name, XYFitCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYFitCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYFitCurve::~XYFitCurve() = default;

void XYFitCurve::recalculate() {
	Q_D(XYFitCurve);
	d->recalculate();
}

void XYFitCurve::evaluate(bool preview) {
	Q_D(XYFitCurve);
	d->evaluate(preview);
}

void XYFitCurve::initStartValues(const XYCurve* curve) {
	Q_D(XYFitCurve);
	XYFitCurve::FitData& fitData = d->fitData;
	initStartValues(fitData, curve);
}

void XYFitCurve::initStartValues(XYFitCurve::FitData& fitData, const XYCurve* curve) {
	DEBUG("XYFitCurve::initStartValues()");
	if (!curve) {
		DEBUG("	no curve given");
		return;
	}

	const Column* tmpXDataColumn = dynamic_cast<const Column*>(curve->xColumn());
	const Column* tmpYDataColumn = dynamic_cast<const Column*>(curve->yColumn());

	if (!tmpXDataColumn || !tmpYDataColumn) {
		DEBUG("	data columns not available");
		return;
	}

	DEBUG("	x data rows = " << tmpXDataColumn->rowCount());

	nsl_fit_model_category modelCategory = fitData.modelCategory;
	int modelType = fitData.modelType;
	int degree = fitData.degree;
	DEBUG("	fit model type = " << modelType << ", degree = " << degree);

	QVector<double>& paramStartValues = fitData.paramStartValues;
	//QVector<double>* xVector = static_cast<QVector<double>* >(tmpXDataColumn->data());

	//double xmean = gsl_stats_mean(xVector->constData(), 1, tmpXDataColumn->rowCount());
	double xmin = tmpXDataColumn->minimum();
	double xmax = tmpXDataColumn->maximum();
	//double ymin = tmpYDataColumn->minimum();
	double ymax = tmpYDataColumn->maximum();
	double xrange = xmax - xmin;
	//double yrange = ymax-ymin;
	DEBUG("	x min/max = " << xmin << ' ' << xmax);
	//DEBUG("	y min/max = " << ymin << ' ' << ymax);

	// guess start values for parameter
	switch (modelCategory) {
	case nsl_fit_model_basic:
		switch (modelType) {
		case nsl_fit_model_polynomial:
			// not needed (works anyway)
			break;
		//TODO: handle basic models
		case nsl_fit_model_power:
		case nsl_fit_model_exponential:
		case nsl_fit_model_inverse_exponential:
		case nsl_fit_model_fourier:
			break;
		}
		break;
	case nsl_fit_model_peak:
		// use equidistant mu's and (xmax-xmin)/(10*degree) as sigma(, gamma)
		switch (modelType) {
		case nsl_fit_model_gaussian:
		case nsl_fit_model_lorentz:
		case nsl_fit_model_sech:
		case nsl_fit_model_logistic:
			for (int d = 0; d < degree; d++) {
				paramStartValues[3*d+2] = xmin + (d+1.)*xrange/(degree+1.);	// mu
				paramStartValues[3*d+1] = xrange/(10.*degree);	// sigma
				paramStartValues[3*d] = paramStartValues[3*d+1] * ymax;		// A = sigma * ymax
			}
			break;
		case nsl_fit_model_voigt:
			for (int d = 0; d < degree; d++) {
				paramStartValues[4*d+1] = xmin + (d+1.)*xrange/(degree+1.);	// mu
				paramStartValues[4*d+2] = xrange/(10.*degree);	// sigma
				paramStartValues[4*d+3] = xrange/(10.*degree);	// gamma
			}
			break;
		case nsl_fit_model_pseudovoigt1:
			for (int d = 0; d < degree; d++) {
				paramStartValues[4*d+1] = 0.5;	// eta
				paramStartValues[4*d+2] = xrange/(10.*degree);	// sigma
				paramStartValues[4*d+3] = xmin + (d+1.)*xrange/(degree+1.);	// mu
			}
			break;
		}
		break;
	case nsl_fit_model_growth:
		switch (modelType) {
		case nsl_fit_model_atan:
		case nsl_fit_model_tanh:
		case nsl_fit_model_algebraic_sigmoid:
		case nsl_fit_model_erf:
		case nsl_fit_model_gudermann:
		case nsl_fit_model_sigmoid:
			// use (xmax+xmin)/2 as mu and (xmax-xmin)/10 as sigma
			paramStartValues[1] = (xmax+xmin)/2.;
			paramStartValues[2] = xrange/10.;
			break;
		case nsl_fit_model_hill:
			paramStartValues[2] = xrange/10.;
			break;
		case nsl_fit_model_gompertz:
			//TODO
			break;
		}
		break;
	case nsl_fit_model_distribution:
		switch (modelType) {
		case nsl_sf_stats_gaussian:
		case nsl_sf_stats_laplace:
		case nsl_sf_stats_rayleigh_tail:
		case nsl_sf_stats_lognormal:
		case nsl_sf_stats_logistic:
		case nsl_sf_stats_sech:
		case nsl_sf_stats_cauchy_lorentz:
		case nsl_sf_stats_levy:
			// mu
			paramStartValues[2] = (xmin+xmax)/2.;
			// sigma
			paramStartValues[1] = xrange/10.;
			// A = sigma * y_max
			paramStartValues[0] = paramStartValues[1] * ymax;
			break;
		//TODO: other types
		default:
			break;
		}
		break;
	case nsl_fit_model_custom:
		// not possible
		break;
	}
}

/*!
 * sets the parameter names for given model category, model type and degree in \c fitData for given action
 */
void XYFitCurve::initFitData(PlotDataDialog::AnalysisAction action) {
	//TODO: exclude others too?
	if (action == PlotDataDialog::AnalysisAction::DataReduction)
		return;

	Q_D(XYFitCurve);
	XYFitCurve::FitData& fitData = d->fitData;
	if (action == PlotDataDialog::AnalysisAction::FitLinear) {
		//Linear
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_polynomial;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::AnalysisAction::FitPower) {
		//Power
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_power;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::AnalysisAction::FitExp1) {
		//Exponential (degree 1)
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_exponential;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::AnalysisAction::FitExp2) {
		//Exponential (degree 2)
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_exponential;
		fitData.degree = 2;
	} else if (action == PlotDataDialog::AnalysisAction::FitInvExp) {
		//Inverse exponential
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_inverse_exponential;
	} else if (action == PlotDataDialog::AnalysisAction::FitGauss) {
		//Gauss
		fitData.modelCategory = nsl_fit_model_peak;
		fitData.modelType = (int)nsl_fit_model_gaussian;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::AnalysisAction::FitCauchyLorentz) {
		//Cauchy-Lorentz
		fitData.modelCategory = nsl_fit_model_peak;
		fitData.modelType = (int)nsl_fit_model_lorentz;
		fitData.degree = 1;
	} else if (action == PlotDataDialog::AnalysisAction::FitTan) {
		//Arc tangent
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = (int)nsl_fit_model_atan;
	} else if (action == PlotDataDialog::AnalysisAction::FitTanh) {
		//Hyperbolic tangent
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = (int)nsl_fit_model_tanh;
	} else if (action == PlotDataDialog::AnalysisAction::FitErrFunc) {
		//Error function
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = (int)nsl_fit_model_erf;
	} else {
		//Custom
		fitData.modelCategory = nsl_fit_model_custom;
		fitData.modelType = 0;
	}

	XYFitCurve::initFitData(fitData);
}

/*!
 * sets the model expression and the parameter names for given model category, model type and degree in \c fitData
 */
void XYFitCurve::initFitData(XYFitCurve::FitData& fitData) {
	DEBUG("XYFitCurve::initFitData()")
	nsl_fit_model_category modelCategory = fitData.modelCategory;
	int modelType = fitData.modelType;
	QString& model = fitData.model;
	QStringList& paramNames = fitData.paramNames;
	QStringList& paramNamesUtf8 = fitData.paramNamesUtf8;
	int degree = fitData.degree;
	QVector<double>& paramStartValues = fitData.paramStartValues;
	QVector<double>& paramLowerLimits = fitData.paramLowerLimits;
	QVector<double>& paramUpperLimits = fitData.paramUpperLimits;
	QVector<bool>& paramFixed = fitData.paramFixed;

	if (modelCategory != nsl_fit_model_custom) {
		DEBUG("XYFitCurve::initFitData() for model category = " << nsl_fit_model_category_name[modelCategory] << ", model type = " << modelType
			<< ", degree = " << degree);
		paramNames.clear();
	} else {
		DEBUG("XYFitCurve::initFitData() for model category = nsl_fit_model_custom, model type = " << modelType << ", degree = " << degree);
	}
	paramNamesUtf8.clear();

	// 10 indices used in multi degree models
	QStringList indices = { UTF8_QSTRING("₁"), UTF8_QSTRING("₂"), UTF8_QSTRING("₃"),
		UTF8_QSTRING("₄"), UTF8_QSTRING("₅"), UTF8_QSTRING("₆"), UTF8_QSTRING("₇"),
		UTF8_QSTRING("₈"), UTF8_QSTRING("₉"), UTF8_QSTRING("₁₀")};

	switch (modelCategory) {
	case nsl_fit_model_basic:
		model = nsl_fit_model_basic_equation[fitData.modelType];
		switch (modelType) {
		case nsl_fit_model_polynomial:
			paramNames << "c0" << "c1";
			paramNamesUtf8 << UTF8_QSTRING("c₀") << UTF8_QSTRING("c₁");
			if (degree == 2) {
				model += " + c2*x^2";
				paramNames << "c2";
				paramNamesUtf8 << UTF8_QSTRING("c₂");
			} else if (degree > 2) {
				for (int i = 2; i <= degree; ++i) {
					QString numStr = QString::number(i);
					model += "+c" + numStr + "*x^" + numStr;
					paramNames << 'c' + numStr;
					paramNamesUtf8 << 'c' + indices[i-1];
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
					paramNames << 'a' + numStr << 'b' + numStr;
					paramNamesUtf8 << 'a' + indices[i-1] << 'b' + indices[i-1];
				}
			}
			break;
		case nsl_fit_model_inverse_exponential:
			paramNames << "a" << "b" << "c";
			break;
		case nsl_fit_model_fourier:
			paramNames << "w" << "a0" << "a1" << "b1";
			paramNamesUtf8 << UTF8_QSTRING("ω") << UTF8_QSTRING("a₀")
				<< UTF8_QSTRING("a₁") << UTF8_QSTRING("b₁");
			if (degree > 1) {
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					model += "+ (a" + numStr + "*cos(" + numStr + "*w*x) + b" + numStr + "*sin(" + numStr + "*w*x))";
					paramNames << 'a' + numStr << 'b' + numStr;
					paramNamesUtf8 << 'a' + indices[i-1] << 'b' + indices[i-1];
				}
			}
			break;
		}
		break;
	case nsl_fit_model_peak:
		model = nsl_fit_model_peak_equation[fitData.modelType];
		switch (modelType) {
		case nsl_fit_model_gaussian:
			switch (degree) {
			case 1:
				paramNames << "a" << "s" << "mu";
				paramNamesUtf8 << "A" << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
				break;
			default:
				model = "1./sqrt(2*pi) * (";
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += 'a' + numStr + "/s" + numStr + "* exp(-((x-mu" + numStr + ")/s" + numStr + ")^2/2)";
					paramNames << 'a' + numStr << 's' + numStr << "mu" + numStr;
					paramNamesUtf8 << 'A' + indices[i-1] << UTF8_QSTRING("σ") + indices[i-1] << UTF8_QSTRING("μ") + indices[i-1];
				}
				model += ')';
			}
			break;
		case nsl_fit_model_lorentz:
			switch (degree) {
			case 1:
				paramNames << "a" << "g" << "mu";
				paramNamesUtf8 << "A" << UTF8_QSTRING("γ") << UTF8_QSTRING("μ");
				break;
			default:
				model = "1./pi * (";
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += 'a' + numStr + " * g" + numStr + "/(g" + numStr + "^2+(x-mu" + numStr + ")^2)";
					paramNames << 'a' + numStr << 'g' + numStr << "mu" + numStr;
					paramNamesUtf8 << 'A' + indices[i-1] << UTF8_QSTRING("γ") + indices[i-1] << UTF8_QSTRING("μ") + indices[i-1];
				}
				model += ')';
			}
			break;
		case nsl_fit_model_sech:
			switch (degree) {
			case 1:
				paramNames << "a" << "s" << "mu";
				paramNamesUtf8 << "A" << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
				break;
			default:
				model = "1/pi * (";
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += 'a' + numStr + "/s" + numStr + "* sech((x-mu" + numStr + ")/s" + numStr + ')';
					paramNames << 'a' + numStr << 's' + numStr << "mu" + numStr;
					paramNamesUtf8 << 'A' + indices[i-1] << UTF8_QSTRING("σ") + indices[i-1] << UTF8_QSTRING("μ") + indices[i-1];
				}
				model += ')';
			}
			break;
		case nsl_fit_model_logistic:
			switch (degree) {
			case 1:
				paramNames << "a" << "s" << "mu";
				paramNamesUtf8 << "A" << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
				break;
			default:
				model = "1/4 * (";
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += 'a' + numStr + "/s" + numStr + "* sech((x-mu" + numStr + ")/2/s" + numStr + ")**2";
					paramNames << 'a' + numStr << 's' + numStr << "mu" + numStr;
					paramNamesUtf8 << 'A' + indices[i-1] << UTF8_QSTRING("σ") + indices[i-1] << UTF8_QSTRING("μ") + indices[i-1];
				}
				model += ')';
			}
			break;
		case nsl_fit_model_voigt:
			switch (degree) {
			case 1:
				paramNames << "a" << "mu" << "s" << "g";
				paramNamesUtf8 << "A" << UTF8_QSTRING("μ") << UTF8_QSTRING("σ") << UTF8_QSTRING("γ");
				break;
			default:
				model.clear();
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += 'a' + numStr + "*voigt(x-mu" + numStr + ",s" + numStr + ",g" + numStr + ')';
					paramNames << 'a' + numStr << "mu" + numStr << 's' + numStr << 'g' + numStr;
					paramNamesUtf8 << 'A' + indices[i-1] << UTF8_QSTRING("μ") + indices[i-1] << UTF8_QSTRING("σ") + indices[i-1] << UTF8_QSTRING("γ") + indices[i-1];
				}
			}
			break;
		case nsl_fit_model_pseudovoigt1:
			switch (degree) {
			case 1:
				paramNames << "a" << "et" << "w" << "mu";	// eta function exists!
				paramNamesUtf8 << "A" << UTF8_QSTRING("η") << "w" << UTF8_QSTRING("μ");
				break;
			default:
				model.clear();
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += " + ";
					model += 'a' + numStr + "*pseudovoigt1(x-mu" + numStr + ",eta" + numStr + ",w" + numStr + ')';
					paramNames << 'a' + numStr << "eta" + numStr << 'w' + numStr << "mu" + numStr;
					paramNamesUtf8 << 'A' + indices[i-1] << UTF8_QSTRING("η") + indices[i-1] << 'w' + indices[i-1] << UTF8_QSTRING("μ") + indices[i-1];
				}
			}
			break;
		}
		break;
	case nsl_fit_model_growth:
		model = nsl_fit_model_growth_equation[fitData.modelType];
		switch (modelType) {
		case nsl_fit_model_atan:
		case nsl_fit_model_tanh:
		case nsl_fit_model_algebraic_sigmoid:
		case nsl_fit_model_erf:
		case nsl_fit_model_gudermann:
			paramNames << "a" << "mu" << "s";
			paramNamesUtf8 << "A" << UTF8_QSTRING("μ") << UTF8_QSTRING("σ");
			break;
		case nsl_fit_model_sigmoid:
			paramNames << "a" << "mu" << "k";
			paramNamesUtf8 << "A" << UTF8_QSTRING("μ") << "k";
			break;
		case nsl_fit_model_hill:
			paramNames << "a" << "n" << "a";
			paramNamesUtf8 << "A" << "n" << UTF8_QSTRING("σ");
			break;
		case nsl_fit_model_gompertz:
			paramNames << "a" << "b" << "c";
			break;
		}
		break;
	case nsl_fit_model_distribution:
		model = nsl_sf_stats_distribution_equation[fitData.modelType];
		switch (modelType) {
		case nsl_sf_stats_gaussian:
		case nsl_sf_stats_laplace:
		case nsl_sf_stats_rayleigh_tail:
		case nsl_sf_stats_lognormal:
		case nsl_sf_stats_logistic:
		case nsl_sf_stats_sech:
			paramNames << "a" << "s" << "mu";
			paramNamesUtf8 << "A" << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_gaussian_tail:
			paramNames << "A" << "s" << "a" << "mu";
			paramNamesUtf8 << "A" << UTF8_QSTRING("σ") << "a" << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_exponential:
			paramNames << "a" << "l" << "mu";
			paramNamesUtf8 << "A" << UTF8_QSTRING("λ") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_exponential_power:
			paramNames << "a" << "s" << "b" << "mu";
			paramNamesUtf8 << "A" << UTF8_QSTRING("σ") << "b" << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_cauchy_lorentz:
		case nsl_sf_stats_levy:
			paramNames << "a" << "g" << "mu";
			paramNamesUtf8 << "A" << UTF8_QSTRING("γ") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_rayleigh:
			paramNames << "a" << "s";
			paramNamesUtf8 << "A" << UTF8_QSTRING("σ");
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
			paramNames << "a" << "k" << "t";
			paramNamesUtf8 << "A"<< "k" << UTF8_QSTRING("θ");
			break;
		case nsl_sf_stats_flat:
			paramNames << "A" << "b" << "a";
			break;
		case nsl_sf_stats_chi_squared:
			paramNames << "a" << "n";
			paramNamesUtf8 << "A" << "n";
			break;
		case nsl_sf_stats_fdist:
			paramNames << "a" << "n1" << "n2";
			paramNamesUtf8 << "A" << UTF8_QSTRING("ν₁") << UTF8_QSTRING("ν₂");
			break;
		case nsl_sf_stats_tdist:
			paramNames << "a" << "n";
			paramNamesUtf8 << "A" << UTF8_QSTRING("ν");
			break;
		case nsl_sf_stats_beta:
		case nsl_sf_stats_pareto:
			paramNames << "A" << "a" << "b";
			break;
		case nsl_sf_stats_weibull:
			paramNames << "a" << "k" << "l" << "mu";
			paramNamesUtf8 << "A" << "k" << UTF8_QSTRING("λ") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_gumbel1:
			paramNames << "a" << "s" << "mu" << "b";
			paramNamesUtf8 << "A" << UTF8_QSTRING("σ") << UTF8_QSTRING("μ") << UTF8_QSTRING("β");
			break;
		case nsl_sf_stats_gumbel2:
			paramNames << "A" << "a" << "b" << "mu";
			paramNamesUtf8 << "A" << "a" << "b" << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_poisson:
			paramNames << "a" << "l";
			paramNamesUtf8 << "A" << UTF8_QSTRING("λ");
			break;
		case nsl_sf_stats_binomial:
		case nsl_sf_stats_negative_binomial:
		case nsl_sf_stats_pascal:
			paramNames << "a" << "p" << "n";
			paramNamesUtf8 << "A" << "p" << "n";
			break;
		case nsl_sf_stats_geometric:
		case nsl_sf_stats_logarithmic:
			paramNames << "a" << "p";
			paramNamesUtf8 << "A" << "p";
			break;
		case nsl_sf_stats_hypergeometric:
			paramNames << "a" << "n1" << "n2" << "t";
			paramNamesUtf8 << "A" << UTF8_QSTRING("n₁") << UTF8_QSTRING("n₂") << "t";
			break;
		case nsl_sf_stats_maxwell_boltzmann:
			paramNames << "a" << "s";
			paramNamesUtf8 << "A" << UTF8_QSTRING("σ");
			break;
		case nsl_sf_stats_frechet:
			paramNames << "a" << "g" << "s" << "mu";
			paramNamesUtf8 << "A" << UTF8_QSTRING("γ") << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
			break;
		}
		break;
	case nsl_fit_model_custom:
		break;
	}
	DEBUG("XYFitCurve::initFitData()	model: " << STDSTRING(model));
	DEBUG("XYFitCurve::initFitData()	# params: " << paramNames.size());

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
			paramLowerLimits[i] = -std::numeric_limits<double>::max();
			paramUpperLimits[i] = std::numeric_limits<double>::max();
		}

		// set some model-dependent start values
		// TODO: see initStartValues()
		if (modelCategory == nsl_fit_model_distribution) {
			if (modelType == (int)nsl_sf_stats_flat)
				paramStartValues[2] = -1.0;
			else if (modelType == (int)nsl_sf_stats_levy)
				paramStartValues[2] = 0.0;
			else if (modelType == (int)nsl_sf_stats_exponential_power || modelType == (int)nsl_sf_stats_weibull || modelType == (int)nsl_sf_stats_gumbel2
				|| modelType == (int)nsl_sf_stats_frechet)
				paramStartValues[3] = 0.0;
			else if (modelType == (int)nsl_sf_stats_binomial || modelType == (int)nsl_sf_stats_negative_binomial || modelType == (int)nsl_sf_stats_pascal
				|| modelType == (int)nsl_sf_stats_geometric || modelType == (int)nsl_sf_stats_logarithmic)
				paramStartValues[1] = 0.5;
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
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, xErrorColumn, xErrorColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, yErrorColumn, yErrorColumn)
const QString& XYFitCurve::xErrorColumnPath() const { Q_D(const XYFitCurve); return d->xErrorColumnPath; }
const QString& XYFitCurve::yErrorColumnPath() const { Q_D(const XYFitCurve); return d->yErrorColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYFitCurve, XYFitCurve::FitData, fitData, fitData)

const XYFitCurve::FitResult& XYFitCurve::fitResult() const {
	Q_D(const XYFitCurve);
	return d->fitResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYFitCurve, SetXErrorColumn, const AbstractColumn*, xErrorColumn)
void XYFitCurve::setXErrorColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->xErrorColumn) {
		exec(new XYFitCurveSetXErrorColumnCmd(d, column, ki18n("%1: assign x-error")));
		handleSourceDataChanged();
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, [=](){ handleSourceDataChanged(); });
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetYErrorColumn, const AbstractColumn*, yErrorColumn)
void XYFitCurve::setYErrorColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->yErrorColumn) {
		exec(new XYFitCurveSetYErrorColumnCmd(d, column, ki18n("%1: assign y-error")));
		handleSourceDataChanged();
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, [=](){ handleSourceDataChanged(); });
			//TODO disconnect on undo
		}
	}
}

// do not recalculate (allow preview)
//STD_SETTER_CMD_IMPL_F_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData, recalculate)
STD_SETTER_CMD_IMPL_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData)
void XYFitCurve::setFitData(const XYFitCurve::FitData& fitData) {
	Q_D(XYFitCurve);
	exec(new XYFitCurveSetFitDataCmd(d, fitData, ki18n("%1: set fit options and perform the fit")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFitCurvePrivate::XYFitCurvePrivate(XYFitCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner) {}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYFitCurvePrivate::~XYFitCurvePrivate() = default;

// data structure to pass parameter to fit functions
struct data {
	size_t n;	//number of data points
	double* x;	//pointer to the vector with x-data values
	double* y;	//pointer to the vector with y-data values
	double* weight;	//pointer to the vector with weight values
	nsl_fit_model_category modelCategory;
	int modelType;
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
	//DEBUG("func_f");
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
		double v = gsl_vector_get(paramValues, (size_t)i);
		// bound values if limits are set
		QByteArray paramnameba = paramNames->at(i).toLatin1();
		assign_variable(paramnameba.constData(), nsl_fit_map_bound(v, min[i], max[i]));
		QDEBUG("Parameter"<<i<<" (\" "<<paramnameba.constData()<<"\")"<<'['<<min[i]<<','<<max[i]
			<<"] free/bound:"<<QString::number(v, 'g', 15)<<' '<<QString::number(nsl_fit_map_bound(v, min[i], max[i]), 'g', 15));
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
		//DEBUG("evaluate function \"" << func << "\" @ x = " << x[i] << ":");
		double Yi = parse(func);
		//DEBUG("	f(x["<< i <<"]) = " << Yi);

		if (parse_errors() > 0)
			return GSL_EINVAL;

		gsl_vector_set(f, i, sqrt(weight[i]) * (Yi - y[i]));
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
	//DEBUG("func_df");
	const size_t n = ((struct data*)params)->n;
	double* xVector = ((struct data*)params)->x;
	double* weight = ((struct data*)params)->weight;
	nsl_fit_model_category modelCategory = ((struct data*)params)->modelCategory;
	unsigned int modelType = ((struct data*)params)->modelType;
	unsigned int degree = ((struct data*)params)->degree;
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
				for (unsigned int j = 0; j < (unsigned int)paramNames->size(); ++j) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_polynomial_param_deriv(x, j, weight[i]));
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
							gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
						else
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_power1_param_deriv(j, x, a, b, weight[i]));
					}
				}
			} else if (degree == 2) {
				const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
				const double c = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
				for (size_t i = 0; i < n; i++) {
					x = xVector[i];

					for (int j = 0; j < 3; j++) {
						if (fixed[j])
							gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
						else
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_power2_param_deriv(j, x, b, c, weight[i]));
					}
				}
			}
			break;
		case nsl_fit_model_exponential:	{ // Y(x) = a*exp(b*x) + c*exp(d*x) + ...
			double *p = new double[2*degree];
			for (unsigned int i = 0; i < 2*degree; i++)
				p[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, i), min[i], max[i]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 2*degree; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_exponentialn_param_deriv(j, x, p, weight[i]));
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

				for (unsigned int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_inverse_exponential_param_deriv(j, x, a, b, weight[i]));
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
			for (unsigned int i = 1; i < degree; ++i) {
				a[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, 2*i), min[2*i], max[2*i]);
				b[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, 2*i+1), min[2*i+1], max[2*i+1]);
			}
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				double wd = 0; //first derivative with respect to the w parameter
				for (unsigned int j = 1; j < degree; ++j) {
					wd += -a[j]*j*x*sin(j*w*x) + b[j]*j*x*cos(j*w*x);
				}

				gsl_matrix_set(J, i, 0, weight[i]*wd);
				gsl_matrix_set(J, i, 1, weight[i]);
				for (unsigned int j = 1; j <= degree; ++j) {
					gsl_matrix_set(J, (size_t)i, (size_t)(2*j), nsl_fit_model_fourier_param_deriv(0, j, x, w, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(2*j+1), nsl_fit_model_fourier_param_deriv(1, j, x, w, weight[i]));
				}

				for (unsigned int j = 0; j <= 2*degree+1; j++)
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
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
		case nsl_fit_model_logistic:
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < degree; ++j) {
					const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j), min[3*j], max[3*j]);
					const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j+1), min[3*j+1], max[3*j+1]);
					const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j+2), min[3*j+2], max[3*j+2]);

					switch (modelType) {
					case nsl_fit_model_gaussian:
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j), nsl_fit_model_gaussian_param_deriv(0, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j+1), nsl_fit_model_gaussian_param_deriv(1, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j+2), nsl_fit_model_gaussian_param_deriv(2, x, a, s, mu, weight[i]));
						break;
					case nsl_fit_model_lorentz:	// a,s,t
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j), nsl_fit_model_lorentz_param_deriv(0, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j+1), nsl_fit_model_lorentz_param_deriv(1, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j+2), nsl_fit_model_lorentz_param_deriv(2, x, a, s, mu, weight[i]));
						break;
					case nsl_fit_model_sech:
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j), nsl_fit_model_sech_param_deriv(0, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j+1), nsl_fit_model_sech_param_deriv(1, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j+2), nsl_fit_model_sech_param_deriv(2, x, a, s, mu, weight[i]));
						break;
					case nsl_fit_model_logistic:
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j), nsl_fit_model_logistic_param_deriv(0, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j+1), nsl_fit_model_logistic_param_deriv(1, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3*j+2), nsl_fit_model_logistic_param_deriv(2, x, a, s, mu, weight[i]));
						break;
					}
				}

				for (unsigned int j = 0; j < 3*degree; j++)
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
			}
			break;
		case nsl_fit_model_voigt:
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < degree; ++j) {
					const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 4*j), min[4*j], max[4*j]);
					const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 4*j+1), min[4*j+1], max[4*j+1]);
					const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 4*j+2), min[4*j+2], max[4*j+2]);
					const double g = nsl_fit_map_bound(gsl_vector_get(paramValues, 4*j+3), min[4*j+3], max[4*j+3]);

					gsl_matrix_set(J, (size_t)i, (size_t)(4*j), nsl_fit_model_voigt_param_deriv(0, x, a, mu, s, g, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4*j+1), nsl_fit_model_voigt_param_deriv(1, x, a, mu, s, g, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4*j+2), nsl_fit_model_voigt_param_deriv(2, x, a, mu, s, g, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4*j+3), nsl_fit_model_voigt_param_deriv(3, x, a, mu, s, g, weight[i]));
				}
				for (unsigned int j = 0; j < 4*degree; j++)
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
			}
			break;
		case nsl_fit_model_pseudovoigt1:
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < degree; ++j) {
					const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 4*j), min[4*j], max[4*j]);
					const double eta = nsl_fit_map_bound(gsl_vector_get(paramValues, 4*j+1), min[4*j+1], max[4*j+1]);
					const double w = nsl_fit_map_bound(gsl_vector_get(paramValues, 4*j+2), min[4*j+2], max[4*j+2]);
					const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 4*j+3), min[4*j+3], max[4*j+3]);

					gsl_matrix_set(J, (size_t)i, (size_t)(4*j), nsl_fit_model_voigt_param_deriv(0, x, a, eta, w, mu, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4*j+1), nsl_fit_model_voigt_param_deriv(1, x, a, eta, w, mu, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4*j+2), nsl_fit_model_voigt_param_deriv(2, x, a, eta, w, mu, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4*j+3), nsl_fit_model_voigt_param_deriv(3, x, a, eta, w, mu, weight[i]));
				}
				for (unsigned int j = 0; j < 4*degree; j++)
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
			}
			break;
		}
		break;
	case nsl_fit_model_growth: {
		const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
		const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
		const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);

		for (size_t i = 0; i < n; i++) {
			x = xVector[i];

			for (unsigned int j = 0; j < 3; j++) {
				if (fixed[j]) {
					gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
				} else {
					switch (modelType) {
					case nsl_fit_model_atan:
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_atan_param_deriv(j, x, a, mu, s, weight[i]));
						break;
					case nsl_fit_model_tanh:
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_tanh_param_deriv(j, x, a, mu, s, weight[i]));
						break;
					case nsl_fit_model_algebraic_sigmoid:
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_algebraic_sigmoid_param_deriv(j, x, a, mu, s, weight[i]));
						break;
					case nsl_fit_model_sigmoid:
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_sigmoid_param_deriv(j, x, a, mu, s, weight[i]));
						break;
					case nsl_fit_model_erf:
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_erf_param_deriv(j, x, a, mu, s, weight[i]));
						break;
					case nsl_fit_model_hill:
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_hill_param_deriv(j, x, a, mu, s, weight[i]));
						break;
					case nsl_fit_model_gompertz:
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_gompertz_param_deriv(j, x, a, mu, s, weight[i]));
						break;
					case nsl_fit_model_gudermann:
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_gudermann_param_deriv(j, x, a, mu, s, weight[i]));
						break;
					}
				}
			}
		}
		}
		break;
	case nsl_fit_model_distribution:
		switch (modelType) {
		case nsl_sf_stats_gaussian:
		case nsl_sf_stats_exponential:
		case nsl_sf_stats_laplace:
		case nsl_sf_stats_cauchy_lorentz:
		case nsl_sf_stats_rayleigh_tail:
		case nsl_sf_stats_lognormal:
		case nsl_sf_stats_logistic:
		case nsl_sf_stats_sech:
		case nsl_sf_stats_levy: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else {
						switch (modelType) {
						case nsl_sf_stats_gaussian:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_gaussian_param_deriv(j, x, a, s, mu, weight[i]));
							break;
						case nsl_sf_stats_exponential:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_exponential_param_deriv(j, x, a, s, mu, weight[i]));
							break;
						case nsl_sf_stats_laplace:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_laplace_param_deriv(j, x, a, s, mu, weight[i]));
							break;
						case nsl_sf_stats_cauchy_lorentz:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_lorentz_param_deriv(j, x, a, s, mu, weight[i]));
							break;
						case nsl_sf_stats_rayleigh_tail:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_rayleigh_tail_param_deriv(j, x, a, s, mu, weight[i]));
							break;
						case nsl_sf_stats_lognormal:
							if (x > 0)
								gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_lognormal_param_deriv(j, x, a, s, mu, weight[i]));
							else
								gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
							break;
						case nsl_sf_stats_logistic:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_logistic_param_deriv(j, x, a, s, mu, weight[i]));
							break;
						case nsl_sf_stats_sech:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_sech_dist_param_deriv(j, x, a, s, mu, weight[i]));
							break;
						case nsl_sf_stats_levy:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_levy_param_deriv(j, x, a, s, mu, weight[i]));
							break;
						}
					}
				}
			}
			break;
		}
		case nsl_sf_stats_gaussian_tail: {
			const double A = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_gaussian_tail_param_deriv(j, x, A, s, a, mu, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_exponential_power: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_exp_pow_param_deriv(j, x, a, s, b, mu, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_rayleigh: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_rayleigh_param_deriv(j, x, a, s, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_gamma: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double k = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double t = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_gamma_param_deriv(j, x, a, k, t, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_flat: {
			const double A = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_flat_param_deriv(j, x, A, b, a, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_chi_squared: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double nu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_chi_square_param_deriv(j, x, a, nu, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_tdist: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double nu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_students_t_param_deriv(j, x, a, nu, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_fdist: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double n1 = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double n2 = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_fdist_param_deriv(j, x, a, n1, n2, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_beta:
		case nsl_sf_stats_pareto: {
			const double A = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else {
						switch (modelType) {
						case nsl_sf_stats_beta:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_beta_param_deriv(j, x, A, a, b, weight[i]));
							break;
						case nsl_sf_stats_pareto:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_pareto_param_deriv(j, x, A, a, b, weight[i]));
							break;
						}
					}
				}
			}
			break;
		}
		case nsl_sf_stats_weibull: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double k = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double l = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else {
						if (x > 0)
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_weibull_param_deriv(j, x, a, k, l, mu, weight[i]));
						else
							gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					}
				}
			}
			break;
		}
		case nsl_sf_stats_gumbel1: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_gumbel1_param_deriv(j, x, a, s, mu, b, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_gumbel2: {
			const double A = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_gumbel2_param_deriv(j, x, A, a, b, mu, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_poisson: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double l = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_poisson_param_deriv(j, x, a, l, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_maxwell_boltzmann: {	// Y(x) = a*sqrt(2/pi) * x^2/s^3 * exp(-(x/s)^2/2)
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_maxwell_param_deriv(j, x, a, s, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_frechet: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double g = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_frechet_param_deriv(j, x, a, g, s, mu, weight[i]));
				}
			}
			break;
		}
		case nsl_sf_stats_landau: {
			// const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (fixed[0])
					gsl_matrix_set(J, (size_t)i, 0, 0.);
				else
					gsl_matrix_set(J, (size_t)i, 0, nsl_fit_model_landau_param_deriv(0, x, weight[i]));
			}
			break;
		}
		case nsl_sf_stats_binomial:
		case nsl_sf_stats_negative_binomial:
		case nsl_sf_stats_pascal: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double p = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double N = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else {
						switch (modelType) {
						case nsl_sf_stats_binomial:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_binomial_param_deriv(j, x, a, p, N, weight[i]));
							break;
						case nsl_sf_stats_negative_binomial:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_negative_binomial_param_deriv(j, x, a, p, N, weight[i]));
							break;
						case nsl_sf_stats_pascal:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_pascal_param_deriv(j, x, a, p, N, weight[i]));
							break;
						}
					}
				}
			}
			break;
		}
		case nsl_sf_stats_geometric:
		case nsl_sf_stats_logarithmic: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double p = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else {
						switch (modelType) {
						case nsl_sf_stats_geometric:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_geometric_param_deriv(j, x, a, p, weight[i]));
							break;
						case nsl_sf_stats_logarithmic:
							gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_logarithmic_param_deriv(j, x, a, p, weight[i]));
							break;
						}
					}
				}
			}
			break;
		}
		case nsl_sf_stats_hypergeometric: {
			const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			const double n1 = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			const double n2 = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			const double t = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_hypergeometric_param_deriv(j, x, a, n1, n2, t, weight[i]));
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

				double eps = 1.e-9;
				if (std::abs(f_p) > 0)
					eps *= std::abs(f_p);	// scale step size with function value
				value += eps;
				assign_variable(name, value);
				const double f_pdp = parse(func);

//				DEBUG("evaluate deriv"<<QString(func)<<": f(x["<<i<<"]) ="<<QString::number(f_p, 'g', 15));
//				DEBUG("evaluate deriv"<<QString(func)<<": f(x["<<i<<"]+dx) ="<<QString::number(f_pdp, 'g', 15));
//				DEBUG("	deriv = " << STDSTRING(QString::number(sqrt(weight[i])*(f_pdp-f_p)/eps, 'g', 15));

				if (fixed[j])
					gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
				else	// calculate finite difference
					gsl_matrix_set(J, (size_t)i, (size_t)j, sqrt(weight[i])*(f_pdp - f_p)/eps);
			}
		}
	}

	return GSL_SUCCESS;
}

int func_fdf(const gsl_vector* x, void* params, gsl_vector* f, gsl_matrix* J) {
	//DEBUG("func_fdf");
	func_f(x, params, f);
	func_df(x, params, J);

	return GSL_SUCCESS;
}

/* prepare the fit result columns */
void XYFitCurvePrivate::prepareResultColumns() {
	DEBUG("XYFitCurvePrivate::prepareResultColumns()")
	//create fit result columns if not available yet, clear them otherwise
	if (!xColumn) {	// all columns are treated together
		DEBUG("	Creating columns")
		xColumn = new Column("x", AbstractColumn::ColumnMode::Numeric);
		yColumn = new Column("y", AbstractColumn::ColumnMode::Numeric);
		residualsColumn = new Column("residuals", AbstractColumn::ColumnMode::Numeric);
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
		DEBUG("	Clear columns")
		xVector->clear();
		yVector->clear();
		//TODO: residualsVector->clear();
	}
	DEBUG("XYFitCurvePrivate::prepareResultColumns() DONE")
}

void XYFitCurvePrivate::recalculate() {
	DEBUG("XYFitCurvePrivate::recalculate()");

	QElapsedTimer timer;
	timer.start();

	// prepare source data columns
	const AbstractColumn* tmpXDataColumn = nullptr;
	const AbstractColumn* tmpYDataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		DEBUG("	spreadsheet columns as data source");
		tmpXDataColumn = xDataColumn;
		tmpYDataColumn = yDataColumn;
	} else {
		DEBUG("	curve columns as data source");
		tmpXDataColumn = dataSourceCurve->xColumn();
		tmpYDataColumn = dataSourceCurve->yColumn();
	}

	// clear the previous result
	fitResult = XYFitCurve::FitResult();

	if (!tmpXDataColumn || !tmpYDataColumn) {
		DEBUG("	ERROR: Preparing source data columns failed!");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	prepareResultColumns();

	//fit settings
	const unsigned int maxIters = fitData.maxIterations;	//maximal number of iterations
	const double delta = fitData.eps;		//fit tolerance
	const unsigned int np = fitData.paramNames.size(); //number of fit parameters
	if (np == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Model has no parameters.");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	if (yErrorColumn) {
		if (yErrorColumn->rowCount() < tmpXDataColumn->rowCount()) {
			fitResult.available = true;
			fitResult.valid = false;
			fitResult.status = i18n("Not sufficient weight data points provided.");
			emit q->dataChanged();
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
		xmin = fitData.fitRange.left();
		xmax = fitData.fitRange.right();
	}
	DEBUG("	fit range = " << xmin << " .. " << xmax);

	//logic from XYAnalysisCurve::copyData(), extended by the handling of error columns.
	//TODO: decide how to deal with non-numerical error columns
	int rowCount = qMin(tmpXDataColumn->rowCount(), tmpYDataColumn->rowCount());
	for (int row = 0; row < rowCount; ++row) {
		// omit invalid data
		if (!tmpXDataColumn->isValid(row) || tmpXDataColumn->isMasked(row) ||
				!tmpYDataColumn->isValid(row) || tmpYDataColumn->isMasked(row))
			continue;

		double x = NAN;
		switch (tmpXDataColumn->columnMode()) {
		case AbstractColumn::ColumnMode::Numeric:
			x = tmpXDataColumn->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::Integer:
			x = tmpXDataColumn->integerAt(row);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			x = tmpXDataColumn->bigIntAt(row);
			break;
		case AbstractColumn::ColumnMode::Text:	// not valid
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			x = tmpXDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
		}

		double y = NAN;
		switch (tmpYDataColumn->columnMode()) {
		case AbstractColumn::ColumnMode::Numeric:
			y = tmpYDataColumn->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::Integer:
			y = tmpYDataColumn->integerAt(row);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			y = tmpYDataColumn->bigIntAt(row);
			break;
		case AbstractColumn::ColumnMode::Text:	// not valid
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			y = tmpYDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
		}

		// only when inside given range
		if (x >= xmin && x <= xmax) {
			if ((!xErrorColumn && !yErrorColumn) || !fitData.useDataErrors) {	// x-y
				xdataVector.append(x);
				ydataVector.append(y);
			} else if (!xErrorColumn && yErrorColumn) {	// x-y-dy
				if (!std::isnan(yErrorColumn->valueAt(row))) {
					xdataVector.append(x);
					ydataVector.append(y);
					yerrorVector.append(yErrorColumn->valueAt(row));
				}
			} else if (xErrorColumn && yErrorColumn) {	// x-y-dx-dy
				if (!std::isnan(xErrorColumn->valueAt(row)) && !std::isnan(yErrorColumn->valueAt(row))) {
					xdataVector.append(x);
					ydataVector.append(y);
					xerrorVector.append(xErrorColumn->valueAt(row));
					yerrorVector.append(yErrorColumn->valueAt(row));
				}
			}
		}
	}

	//number of data points to fit
	const size_t n = xdataVector.size();
	DEBUG("	number of data points: " << n);
	if (n == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("No data points available.");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	if (n < np) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("The number of data points (%1) must be greater than or equal to the number of parameters (%2).", n, np);
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	if (fitData.model.simplified().isEmpty()) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Fit model not specified.");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* xerror = xerrorVector.data();	// size may be 0
	double* yerror = yerrorVector.data();	// size may be 0
	DEBUG("	x error vector size: " << xerrorVector.size());
	DEBUG("	y error vector size: " << yerrorVector.size());
	double* weight = new double[n];

	for (size_t i = 0; i < n; i++)
		weight[i] = 1.;

	const double minError = 1.e-199;	// minimum error for weighting

	switch (fitData.yWeightsType) {
	case nsl_fit_weight_no:
	case nsl_fit_weight_statistical_fit:
	case nsl_fit_weight_relative_fit:
		break;
	case nsl_fit_weight_instrumental:	// yerror are sigmas
		for (int i = 0; i < (int)n; i++)
			if (i < yerrorVector.size())
				weight[i] = 1./gsl_pow_2(qMax(yerror[i], qMax(sqrt(minError), fabs(ydata[i]) * 1.e-15)));
		break;
	case nsl_fit_weight_direct:		// yerror are weights
		for (int i = 0; i < (int)n; i++)
			if (i < yerrorVector.size())
				weight[i] = yerror[i];
		break;
	case nsl_fit_weight_inverse:		// yerror are inverse weights
		for (int i = 0; i < (int)n; i++)
			if (i < yerrorVector.size())
				weight[i] = 1./qMax(yerror[i], qMax(minError, fabs(ydata[i]) * 1.e-15));
		break;
	case nsl_fit_weight_statistical:
		for (int i = 0; i < (int)n; i++)
			weight[i] = 1./qMax(ydata[i], minError);
		break;
	case nsl_fit_weight_relative:
		for (int i = 0; i < (int)n; i++)
			weight[i] = 1./qMax(gsl_pow_2(ydata[i]), minError);
		break;
	}

	/////////////////////// GSL >= 2 has a complete new interface! But the old one is still supported. ///////////////////////////
	// GSL >= 2 : "the 'fdf' field of gsl_multifit_function_fdf is now deprecated and does not need to be specified for nonlinear least squares problems"
	unsigned int nf = 0;	// number of fixed parameter
	for (unsigned int i = 0; i < np; i++) {
		const bool fixed = fitData.paramFixed.data()[i];
		if (fixed)
			nf++;
		DEBUG("	parameter " << i << " fixed: " << fixed);
	}

	//function to fit
	gsl_multifit_function_fdf f;
	DEBUG("	model = " << STDSTRING(fitData.model));
	struct data params = {n, xdata, ydata, weight, fitData.modelCategory, fitData.modelType, fitData.degree, &fitData.model, &fitData.paramNames, fitData.paramLowerLimits.data(), fitData.paramUpperLimits.data(), fitData.paramFixed.data()};
	f.f = &func_f;
	f.df = &func_df;
	f.fdf = &func_fdf;
	f.n = n;
	f.p = np;
	f.params = &params;

	DEBUG("	initialize the derivative solver (using Levenberg-Marquardt robust solver)");
	const gsl_multifit_fdfsolver_type* T = gsl_multifit_fdfsolver_lmsder;
	gsl_multifit_fdfsolver* s = gsl_multifit_fdfsolver_alloc(T, n, np);

	DEBUG("	set start values");
	double* x_init = fitData.paramStartValues.data();
	double* x_min = fitData.paramLowerLimits.data();
	double* x_max = fitData.paramUpperLimits.data();
	DEBUG("	scale start values if limits are set");
	for (unsigned int i = 0; i < np; i++)
		x_init[i] = nsl_fit_map_unbound(x_init[i], x_min[i], x_max[i]);
	DEBUG(" 	DONE");
	gsl_vector_view x = gsl_vector_view_array(x_init, np);
	DEBUG("	Turning off GSL error handler to avoid overflow/underflow");
	gsl_set_error_handler_off();
	DEBUG("	Initialize solver with function f and initial guess x");
	gsl_multifit_fdfsolver_set(s, &f, &x.vector);

	DEBUG("	Iterate ...");
	int status = GSL_SUCCESS;
	unsigned int iter = 0;
	fitResult.solverOutput.clear();
	writeSolverState(s);
	do {
		iter++;
		DEBUG("		iter " << iter);

		// update weights for Y-depending weights (using function values from residuals)
		if (fitData.yWeightsType == nsl_fit_weight_statistical_fit) {
			for (size_t i = 0; i < n; i++)
				weight[i] = 1./(gsl_vector_get(s->f, i)/sqrt(weight[i]) + ydata[i]);	// 1/Y_i
		} else if (fitData.yWeightsType == nsl_fit_weight_relative_fit) {
			for (size_t i = 0; i < n; i++)
				weight[i] = 1./gsl_pow_2(gsl_vector_get(s->f, i)/sqrt(weight[i]) + ydata[i]);	// 1/Y_i^2
		}

		if (nf == np) {	// all fixed parameter
			DEBUG("	all parameter fixed. Stop iteration.")
			break;
		}
		DEBUG("		run fdfsolver_iterate");
		status = gsl_multifit_fdfsolver_iterate(s);
		DEBUG("		fdfsolver_iterate DONE");
		writeSolverState(s);
		if (status) {
			DEBUG("		iter " << iter << ", status = " << gsl_strerror(status));
			if (status == GSL_ETOLX) 	// change in the position vector falls below machine precision: no progress
				status = GSL_SUCCESS;
			break;
		}
		status = gsl_multifit_test_delta(s->dx, s->x, delta, delta);
		DEBUG("		iter " << iter << ", test status = " << gsl_strerror(status));
	} while (status == GSL_CONTINUE && iter < maxIters);

	// second run for x-error fitting
	if (xerrorVector.size() > 0) {
		DEBUG("	Rerun fit with x errors");

		unsigned int iter2 = 0;
		double chisq = 0, chisqOld = 0;
		double *fun = new double[n];
		do {
			iter2++;
			chisqOld = chisq;
			//printf("iter2 = %d\n", iter2);

			// calculate function from residuals
			for (size_t i = 0; i < n; i++)
				fun[i] = gsl_vector_get(s->f, i) * 1./sqrt(weight[i]) + ydata[i];

			// calculate weight[i]
			for (size_t i = 0; i < n; i++) {
				// calculate df[i]
				size_t index = i-1;
				if (i == 0)
					index = i;
				if (i == n-1)
					index = i-2;
				double df = (fun[index+1] - fun[index])/(xdata[index+1] - xdata[index]);
				//printf("df = %g\n", df);

				double sigmasq = 1.;
				switch (fitData.xWeightsType) {	// x-error type: f'(x)^2*s_x^2 = f'(x)/w_x
				case nsl_fit_weight_no:
					break;
				case nsl_fit_weight_direct:	// xerror = w_x
					sigmasq = df*df/qMax(xerror[i], minError);
					break;
				case nsl_fit_weight_instrumental:	// xerror = s_x
					sigmasq = df*df*xerror[i]*xerror[i];
					break;
				case nsl_fit_weight_inverse:	// xerror = 1/w_x = s_x^2
					sigmasq = df*df*xerror[i];
					break;
				case nsl_fit_weight_statistical:	// s_x^2 = 1/w_x = x
					sigmasq = xdata[i];
					break;
				case nsl_fit_weight_relative:		// s_x^2 = 1/w_x = x^2
					sigmasq = xdata[i]*xdata[i];
					break;
				case nsl_fit_weight_statistical_fit:	// unused
				case nsl_fit_weight_relative_fit:
					break;
				}

				if (yerrorVector.size() > 0) {
					switch (fitData.yWeightsType) {	// y-error types: s_y^2 = 1/w_y
					case nsl_fit_weight_no:
						break;
					case nsl_fit_weight_direct:	// yerror = w_y
						sigmasq += 1./qMax(yerror[i], minError);
						break;
					case nsl_fit_weight_instrumental:	// yerror = s_y
						sigmasq += yerror[i]*yerror[i];
						break;
					case nsl_fit_weight_inverse:	// yerror = 1/w_y
						sigmasq += yerror[i];
						break;
					case nsl_fit_weight_statistical:	// unused
					case nsl_fit_weight_relative:
						break;
					case nsl_fit_weight_statistical_fit:	// s_y^2 = 1/w_y = Y_i
						sigmasq += fun[i];
						break;
					case nsl_fit_weight_relative_fit:	// s_y^2 = 1/w_y = Y_i^2
						sigmasq += fun[i]*fun[i];
						break;
					}
				}

				//printf ("sigma[%d] = %g\n", i, sqrt(sigmasq));
				weight[i] = 1./qMax(sigmasq, minError);
			}

			// update weights
			gsl_multifit_fdfsolver_set(s, &f, &x.vector);

			do {	// fit
				iter++;
				writeSolverState(s);
				status = gsl_multifit_fdfsolver_iterate(s);
				//printf ("status = %s\n", gsl_strerror (status));
				if (nf == np) 	// stop if all parameters fix
					break;

				if (status) {
					DEBUG("		iter " << iter << ", status = " << gsl_strerror(status));
					if (status == GSL_ETOLX) 	// change in the position vector falls below machine precision: no progress
						status = GSL_SUCCESS;
					break;
				}
				status = gsl_multifit_test_delta(s->dx, s->x, delta, delta);
			} while (status == GSL_CONTINUE && iter < maxIters);

			chisq = gsl_blas_dnrm2(s->f);
		} while (iter2 < maxIters && fabs(chisq-chisqOld) > fitData.eps);

		delete[] fun;
	}

	delete[] weight;

	// unscale start parameter
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
	fitResult.status = gslErrorToString(status);
	fitResult.iterations = iter;
	fitResult.dof = n - (np - nf);	// samples - (parameter - fixed parameter)

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
	// SST needed for coefficient of determination, R-squared and F test
	fitResult.sst = gsl_stats_tss(ydata, 1, n);
	// for a linear model without intercept R-squared is calculated differently
	// see https://cran.r-project.org/doc/FAQ/R-FAQ.html#Why-does-summary_0028_0029-report-strange-results-for-the-R_005e2-estimate-when-I-fit-a-linear-model-with-no-intercept_003f
	if (fitData.modelCategory == nsl_fit_model_basic && fitData.modelType == nsl_fit_model_polynomial && fitData.degree == 1 && x_init[0] == 0) {
		DEBUG("	Using alternative R^2 for linear model without intercept");
		fitResult.sst = gsl_stats_tss_m(ydata, 1, n, 0);
	}
	if (fitResult.sst < fitResult.sse) {
		DEBUG("	Using alternative R^2 since R^2 would be negative (probably custom model without intercept)");
		fitResult.sst = gsl_stats_tss_m(ydata, 1, n, 0);
	}

	fitResult.rsquare = nsl_stats_rsquare(fitResult.sse, fitResult.sst);
	fitResult.rsquareAdj = nsl_stats_rsquareAdj(fitResult.rsquare, np, fitResult.dof, 1);
	fitResult.chisq_p = nsl_stats_chisq_p(fitResult.sse, fitResult.dof);
	fitResult.fdist_F = nsl_stats_fdist_F(fitResult.rsquare, np, fitResult.dof);
	fitResult.fdist_p = nsl_stats_fdist_p(fitResult.fdist_F, np, fitResult.dof);
	fitResult.logLik = nsl_stats_logLik(fitResult.sse, n);
	fitResult.aic = nsl_stats_aic(fitResult.sse, n, np, 1);
	fitResult.bic = nsl_stats_bic(fitResult.sse, n, np, 1);

	//parameter values
	// GSL: const double c = GSL_MAX_DBL(1., sqrt(fitResult.rms)); // increase error for poor fit
	// NIST: const double c = sqrt(fitResult.rms); // increase error for poor fit, decrease for good fit
	const double c = sqrt(fitResult.rms);
	fitResult.paramValues.resize(np);
	fitResult.errorValues.resize(np);
	fitResult.tdist_tValues.resize(np);
	fitResult.tdist_pValues.resize(np);
	fitResult.tdist_marginValues.resize(np);
	// CI = 100* (1 - alpha)
	const double alpha = 1.0 - fitData.confidenceInterval/100.;
	for (unsigned int i = 0; i < np; i++) {
		// scale resulting values if they are bounded
		fitResult.paramValues[i] = nsl_fit_map_bound(gsl_vector_get(s->x, i), x_min[i], x_max[i]);
		// use results as start values if desired
		if (fitData.useResults) {
			fitData.paramStartValues.data()[i] = fitResult.paramValues[i];
			DEBUG("	saving parameter " << i << ": " << fitResult.paramValues[i] << ' ' << fitData.paramStartValues.data()[i]);
		}
		fitResult.errorValues[i] = c*sqrt(gsl_matrix_get(covar, i, i));
		fitResult.tdist_tValues[i] = nsl_stats_tdist_t(fitResult.paramValues.at(i), fitResult.errorValues.at(i));
		fitResult.tdist_pValues[i] = nsl_stats_tdist_p(fitResult.tdist_tValues.at(i), fitResult.dof);
		fitResult.tdist_marginValues[i] = nsl_stats_tdist_margin(alpha, fitResult.dof, fitResult.errorValues.at(i));
	}

	// fill residuals vector. To get residuals on the correct x values, fill the rest with zeros.
	residualsVector->resize(tmpXDataColumn->rowCount());
	DEBUG("	Residual vector size: " << residualsVector->size())
	if (fitData.autoRange) {	// evaluate full range of residuals
		xVector->resize(tmpXDataColumn->rowCount());
		auto mode = tmpXDataColumn->columnMode();
		for (int i = 0; i < tmpXDataColumn->rowCount(); i++)
			if (mode == AbstractColumn::ColumnMode::Numeric)
				(*xVector)[i] = tmpXDataColumn->valueAt(i);
			else if (mode == AbstractColumn::ColumnMode::Integer)
				(*xVector)[i] = tmpXDataColumn->integerAt(i);
			else if (mode == AbstractColumn::ColumnMode::BigInt)
				(*xVector)[i] = tmpXDataColumn->bigIntAt(i);
			else if (mode == AbstractColumn::ColumnMode::DateTime)
				(*xVector)[i] = tmpXDataColumn->dateTimeAt(i).toMSecsSinceEpoch();

		ExpressionParser* parser = ExpressionParser::getInstance();
		bool rc = parser->evaluateCartesian(fitData.model, xVector, residualsVector,
							fitData.paramNames, fitResult.paramValues);
		if (rc) {
			for (int i = 0; i < tmpXDataColumn->rowCount(); i++)
				(*residualsVector)[i] = tmpYDataColumn->valueAt(i) - (*residualsVector)[i];
		} else {
			DEBUG("	ERROR: Failed parsing residuals")
			residualsVector->clear();
		}
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
	evaluate();
	fitResult.elapsedTime = timer.elapsed();

	sourceDataChangedSinceLastRecalc = false;
	DEBUG("XYFitCurvePrivate::recalculate() DONE");
}

/* evaluate fit function (preview == true: use start values, default: false) */
void XYFitCurvePrivate::evaluate(bool preview) {
	DEBUG("XYFitCurvePrivate::evaluate() preview = " << preview);

	// prepare source data columns
	const AbstractColumn* tmpXDataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		DEBUG("	spreadsheet columns as data source");
		tmpXDataColumn = xDataColumn;
	} else {
		DEBUG("	curve columns as data source");
		if (dataSourceCurve)
			tmpXDataColumn = dataSourceCurve->xColumn();
	}

	if (!tmpXDataColumn) {
		DEBUG("	ERROR: Preparing source data column failed!");
		recalcLogicalPoints();
		emit q->dataChanged();
		return;
	}

	//only needed for preview (else we have all columns)
	// should not harm even if not in preview now that residuals are not cleared
	if (preview)
		prepareResultColumns();

	if (!xVector || !yVector) {
		DEBUG(" xVector or yVector not defined!");
		recalcLogicalPoints();
		emit q->dataChanged();
		return;
	}

	if (fitData.model.simplified().isEmpty()) {
		DEBUG(" no fit-model specified.");
		recalcLogicalPoints();
		emit q->dataChanged();
		return;
	}

	ExpressionParser* parser = ExpressionParser::getInstance();
	Range<double> xRange{tmpXDataColumn->minimum(), tmpXDataColumn->maximum()};	// full data range
	if (!fitData.autoEvalRange) { 	// use given range for evaluation
		if (!fitData.evalRange.isZero()) 	// avoid zero range
			xRange = fitData.evalRange;
	}
	DEBUG("	eval range = " << STDSTRING(xRange.toString()));
	xVector->resize((int)fitData.evaluatedPoints);
	yVector->resize((int)fitData.evaluatedPoints);
	DEBUG("	vector size = " << xVector->size());

	QVector<double> paramValues = fitResult.paramValues;
	if (preview)	// results not available yet
		paramValues = fitData.paramStartValues;

	bool rc = parser->evaluateCartesian(fitData.model, QString::number(xRange.left()), QString::number(xRange.right()), (int)fitData.evaluatedPoints,
						xVector, yVector, fitData.paramNames, paramValues);
	if (!rc) {
		DEBUG("	ERROR: Parsing fit function failed")
		xVector->clear();
		yVector->clear();
		residualsVector->clear();
	}

	recalcLogicalPoints();
	emit q->dataChanged();
	DEBUG("XYFitCurvePrivate::evaluate() DONE");
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
	DEBUG("	chi = " << gsl_pow_2(gsl_blas_dnrm2(s->f)));

	fitResult.solverOutput += state;
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFitCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFitCurve);

	writer->writeStartElement("xyFitCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-fit-curve specific information

	//fit data - only save model expression and parameter names for custom model, otherwise they are set in XYFitCurve::initFitData()
	writer->writeStartElement("fitData");
	WRITE_COLUMN(d->xErrorColumn, xErrorColumn);
	WRITE_COLUMN(d->yErrorColumn, yErrorColumn);
	writer->writeAttribute("autoRange", QString::number(d->fitData.autoRange));
	writer->writeAttribute("fitRangeMin", QString::number(d->fitData.fitRange.left(), 'g', 15));
	writer->writeAttribute("fitRangeMax", QString::number(d->fitData.fitRange.right(), 'g', 15));
	writer->writeAttribute("modelCategory", QString::number(d->fitData.modelCategory));
	writer->writeAttribute("modelType", QString::number(d->fitData.modelType));
	writer->writeAttribute("xWeightsType", QString::number(d->fitData.xWeightsType));
	writer->writeAttribute("weightsType", QString::number(d->fitData.yWeightsType));
	writer->writeAttribute("degree", QString::number(d->fitData.degree));
	if (d->fitData.modelCategory == nsl_fit_model_custom)
		writer->writeAttribute("model", d->fitData.model);
	writer->writeAttribute("maxIterations", QString::number(d->fitData.maxIterations));
	writer->writeAttribute("eps", QString::number(d->fitData.eps, 'g', 15));
	writer->writeAttribute("evaluatedPoints", QString::number(d->fitData.evaluatedPoints));
	writer->writeAttribute("autoEvalRange", QString::number(d->fitData.autoEvalRange));
	writer->writeAttribute("useDataErrors", QString::number(d->fitData.useDataErrors));
	writer->writeAttribute("useResults", QString::number(d->fitData.useResults));
	writer->writeAttribute("previewEnabled", QString::number(d->fitData.previewEnabled));
	writer->writeAttribute("confidenceInterval", QString::number(d->fitData.confidenceInterval));

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

	writer->writeEndElement(); //"fitData"

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
	DEBUG("XYFitCurve::load()");
	Q_D(XYFitCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyFitCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "fitData") {
			attribs = reader->attributes();

			READ_COLUMN(xErrorColumn);
			READ_COLUMN(yErrorColumn);

			READ_INT_VALUE("autoRange", fitData.autoRange, bool);
			double left{0}, right{0};
			READ_DOUBLE_VALUE_LOCAL("xRangeMin", left);	// old name
			READ_DOUBLE_VALUE_LOCAL("xRangeMax", right);	// old name
			READ_DOUBLE_VALUE_LOCAL("fitRangeMin", left);
			READ_DOUBLE_VALUE_LOCAL("fitRangeMax", right);
			d->fitData.fitRange.setRange(left, right);
			READ_INT_VALUE("modelCategory", fitData.modelCategory, nsl_fit_model_category);
			READ_INT_VALUE("modelType", fitData.modelType, int);
			READ_INT_VALUE("xWeightsType", fitData.xWeightsType, nsl_fit_weight_type);
			READ_INT_VALUE("weightsType", fitData.yWeightsType, nsl_fit_weight_type);
			READ_INT_VALUE("degree", fitData.degree, int);
			if (d->fitData.modelCategory == nsl_fit_model_custom) {
				READ_STRING_VALUE("model", fitData.model);
				DEBUG("read model = " << STDSTRING(d->fitData.model));
			}
			READ_INT_VALUE("maxIterations", fitData.maxIterations, int);
			READ_DOUBLE_VALUE("eps", fitData.eps);
			READ_INT_VALUE("fittedPoints", fitData.evaluatedPoints, size_t);	// old name
			READ_INT_VALUE("evaluatedPoints", fitData.evaluatedPoints, size_t);
			READ_INT_VALUE("evaluateFullRange", fitData.autoEvalRange, bool);	// old name
			READ_INT_VALUE("autoEvalRange", fitData.autoEvalRange, bool);
			READ_INT_VALUE("useDataErrors", fitData.useDataErrors, bool);
			READ_INT_VALUE("useResults", fitData.useResults, bool);
			READ_INT_VALUE("previewEnabled", fitData.previewEnabled, bool);
			READ_DOUBLE_VALUE("confidenceInterval", fitData.confidenceInterval);

			//set the model expression and the parameter names (can be derived from the saved values for category, type and degree)
			XYFitCurve::initFitData(d->fitData);
			// remove default names and start values (will be read from project later)
			d->fitData.paramStartValues.clear();

		} else if (!preview && reader->name() == "name") {	// needed for custom model
			d->fitData.paramNames << reader->readElementText();
		} else if (!preview && reader->name() == "startValue") {
			d->fitData.paramStartValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == "fixed") {
			d->fitData.paramFixed << (bool)reader->readElementText().toInt();
		} else if (!preview && reader->name() == "lowerLimit") {
			bool ok;
			double x = reader->readElementText().toDouble(&ok);
			if (ok)	// -DBL_MAX results in conversion error
				d->fitData.paramLowerLimits << x;
			else
				d->fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		} else if (!preview && reader->name() == "upperLimit") {
			bool ok;
			double x = reader->readElementText().toDouble(&ok);
			if (ok)	// DBL_MAX results in conversion error
				d->fitData.paramUpperLimits << x;
			else
				d->fitData.paramUpperLimits << std::numeric_limits<double>::max();
		} else if (!preview && reader->name() == "value") {
			d->fitResult.paramValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == "error") {
			d->fitResult.errorValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == "tdist_t") {
			d->fitResult.tdist_tValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == "tdist_p") {
			d->fitResult.tdist_pValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == "tdist_margin") {
			d->fitResult.tdist_marginValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == "fitResult") {
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
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Numeric);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			DEBUG("############################   reading column " << STDSTRING(column->name()))
			if (column->name() == "x")
				d->xColumn = column;
			else if (column->name() == "y")
				d->yColumn = column;
			else if (column->name() == "residuals")
				d->residualsColumn = column;
		}
	}

	////////////////////////////// fix old projects /////////////////////////

	// reset model type of old projects due to new model style
	if (d->fitData.modelCategory == nsl_fit_model_basic && d->fitData.modelType >= NSL_FIT_MODEL_BASIC_COUNT) {
		DEBUG("XYFitCurve::load() RESET old fit model");
		d->fitData.modelType = 0;
		d->fitData.degree = 1;
		d->fitData.paramNames.clear();
		d->fitData.paramNamesUtf8.clear();
		// reset size of fields not touched by initFitData()
		d->fitData.paramStartValues.resize(2);
		d->fitData.paramFixed.resize(2);
		d->fitResult.paramValues.resize(2);
		d->fitResult.errorValues.resize(2);
		d->fitResult.tdist_tValues.resize(2);
		d->fitResult.tdist_pValues.resize(2);
		d->fitResult.tdist_marginValues.resize(2);
	}

	// older projects also save the param names for non-custom models: remove them
	while (d->fitData.paramNames.size() > d->fitData.paramStartValues.size())
		d->fitData.paramNames.removeLast();

	// not present in old projects
	if (d->fitData.paramNamesUtf8.isEmpty())
		d->fitData.paramNamesUtf8 << d->fitData.paramNames;

	// not present in old projects
	int np = d->fitResult.paramValues.size();
	if (d->fitResult.tdist_tValues.size() == 0)
		d->fitResult.tdist_tValues.resize(np);
	if (d->fitResult.tdist_pValues.size() == 0)
		d->fitResult.tdist_pValues.resize(np);
	if (d->fitResult.tdist_marginValues.size() == 0)
		d->fitResult.tdist_marginValues.resize(np);

	// Loading done. Check some parameter
	DEBUG("XYFitCurve::load() model type = " << d->fitData.modelType);
	DEBUG("XYFitCurve::load() # params = " << d->fitData.paramNames.size());
	DEBUG("XYFitCurve::load() # start values = " << d->fitData.paramStartValues.size());
	//for (const auto& value : d->fitData.paramStartValues)
	//	DEBUG("XYFitCurve::load() # start value = " << value);

	if (preview)
		return true;

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

		recalcLogicalPoints();
	}

	DEBUG("XYFitCurve::load() DONE");
	return true;
}
