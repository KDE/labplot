/*
	File                 : XYFitCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a fit model
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class XYFitCurve
  \brief A xy-curve defined by a fit model

  \ingroup worksheet
*/

#include "XYFitCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/errors.h"
#include "backend/gsl/Parser.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/nsl/nsl_stats.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_version.h>

XYFitCurve::XYFitCurve(const QString& name)
	: XYAnalysisCurve(name, new XYFitCurvePrivate(this), AspectType::XYFitCurve) {
}

XYFitCurve::XYFitCurve(const QString& name, XYFitCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYFitCurve) {
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYFitCurve::~XYFitCurve() = default;

void XYFitCurve::recalculate() {
	Q_D(XYFitCurve);
	d->recalculate();
}

void XYFitCurve::evaluate(bool preview) {
	Q_D(XYFitCurve);
	if (d->evaluate(preview)) {
		// redraw the curve
		recalc();
		Q_EMIT dataChanged();
	}
}

const XYAnalysisCurve::Result& XYFitCurve::result() const {
	Q_D(const XYFitCurve);
	return d->fitResult;
}
void XYFitCurve::initStartValues(const XYCurve* curve) {
	Q_D(XYFitCurve);
	XYFitCurve::FitData& fitData = d->fitData;
	initStartValues(fitData, curve);
}

void XYFitCurve::initStartValues(XYFitCurve::FitData& fitData, const XYCurve* curve) {
	DEBUG(Q_FUNC_INFO);
	// TODO: curve used for anything?
	if (!curve) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no curve given");
		return;
	}

	Q_D(XYFitCurve);
	const Column* xColumn = dynamic_cast<const Column*>(d->xDataColumn);
	const Column* yColumn = dynamic_cast<const Column*>(d->yDataColumn);
	const Column* yErrorColumn = dynamic_cast<const Column*>(d->yErrorColumn);

	if (!xColumn || !yColumn) {
		DEBUG(Q_FUNC_INFO << ", data columns not available");
		return;
	}

	DEBUG(Q_FUNC_INFO << ", x data rows = " << xColumn->rowCount());
	DEBUG(Q_FUNC_INFO << ", y data rows = " << yColumn->rowCount());

	nsl_fit_model_category modelCategory = fitData.modelCategory;
	int modelType = fitData.modelType;
	const int degree = fitData.degree;
	DEBUG(Q_FUNC_INFO << ", fit model type = " << modelType << ", degree = " << degree);

	QVector<double>& paramStartValues = fitData.paramStartValues;
	// QVector<double>* xVector = static_cast<QVector<double>* >(tmpXDataColumn->data());

	// double xmean = gsl_stats_mean(xVector->constData(), 1, tmpXDataColumn->rowCount());
	double xmin = xColumn->minimum();
	double xmax = xColumn->maximum();
	// double ymin = tmpYDataColumn->minimum();
	double ymax = yColumn->maximum();
	double xrange = xmax - xmin;
	// double yrange = ymax-ymin;
	DEBUG(Q_FUNC_INFO << ", x min/max = " << xmin << ' ' << xmax);
	// DEBUG(Q_FUNC_INFO <<", y min/max = " << ymin << ' ' << ymax);

	// guess start values of parameter
	switch (modelCategory) {
	case nsl_fit_model_basic:
		switch (modelType) {
		case nsl_fit_model_polynomial: { // do a multiparameter linear regression
			// copy all valid data point for the fit to temporary vectors
			QVector<double> xdataVector;
			QVector<double> ydataVector;
			QVector<double> yerrorVector;

			Range<double> xRange{xmin, xmax};
			if (fitData.autoRange) { // auto x range of data to fit
				fitData.fitRange = xRange;
			} else { // custom x range of data to fit
				if (!fitData.fitRange.isZero()) // avoid problems with user specified zero range
					xRange.setRange(fitData.fitRange.start(), fitData.fitRange.end());
			}

			const int rowCount = std::min(xColumn->rowCount(), yColumn->rowCount());
			for (int row = 0; row < rowCount; ++row) {
				// omit invalid data
				if (!xColumn->isValid(row) || xColumn->isMasked(row) || !yColumn->isValid(row) || yColumn->isMasked(row))
					continue;

				double x = NAN;
				switch (xColumn->columnMode()) {
				case AbstractColumn::ColumnMode::Double:
				case AbstractColumn::ColumnMode::Integer:
				case AbstractColumn::ColumnMode::BigInt:
					x = xColumn->valueAt(row);
					break;
				case AbstractColumn::ColumnMode::Text: // not valid
					break;
				case AbstractColumn::ColumnMode::DateTime:
				case AbstractColumn::ColumnMode::Day:
				case AbstractColumn::ColumnMode::Month:
					x = xColumn->dateTimeAt(row).toMSecsSinceEpoch();
				}

				double y = NAN;
				switch (yColumn->columnMode()) {
				case AbstractColumn::ColumnMode::Double:
				case AbstractColumn::ColumnMode::Integer:
				case AbstractColumn::ColumnMode::BigInt:
					y = yColumn->valueAt(row);
					break;
				case AbstractColumn::ColumnMode::Text: // not valid
					break;
				case AbstractColumn::ColumnMode::DateTime:
				case AbstractColumn::ColumnMode::Day:
				case AbstractColumn::ColumnMode::Month:
					y = yColumn->dateTimeAt(row).toMSecsSinceEpoch();
				}

				if (x >= xRange.start() && x <= xRange.end()) { // only when inside given range
					if (!yErrorColumn || !fitData.useDataErrors) { // x-y
						xdataVector.append(x);
						ydataVector.append(y);
					} else if (yErrorColumn) { // x-y-dy
						if (!std::isnan(yErrorColumn->valueAt(row))) {
							xdataVector.append(x);
							ydataVector.append(y);
							yerrorVector.append(yErrorColumn->valueAt(row));
						}
					}
				}
			}

			const int n = xdataVector.size();
			double* xdata = xdataVector.data();
			double* ydata = ydataVector.data();
			double* yerror = yerrorVector.data(); // size may be 0

			const double np = degree + 1;
			gsl_matrix* X = gsl_matrix_alloc(n, np); // X matrix
			gsl_vector* y = gsl_vector_alloc(n); // y values
			gsl_vector* w = gsl_vector_alloc(n); // weights

			gsl_vector* c = gsl_vector_alloc(np); // best fit parameter
			gsl_matrix* cov = gsl_matrix_alloc(np, np);

			const double minError = 1.e-199; // minimum error for weighting
			for (int i = 0; i < n; i++) {
				double xi = xdata[i];
				double yi = ydata[i];

				for (int j = 0; j < np; j++)
					gsl_matrix_set(X, i, j, gsl_pow_int(xi, j));
				gsl_vector_set(y, i, yi);

				if (yErrorColumn && i < yerrorVector.size()) {
					switch (fitData.yWeightsType) {
					case nsl_fit_weight_no:
					case nsl_fit_weight_statistical_fit:
					case nsl_fit_weight_relative_fit:
						gsl_vector_set(w, i, 1.);
						break;
					case nsl_fit_weight_instrumental: // yerror are sigmas
						gsl_vector_set(w, i, 1. / gsl_pow_2(std::max(yerror[i], std::max(sqrt(minError), std::abs(yi) * 1.e-15))));
						break;
					case nsl_fit_weight_direct: // yerror are weights
						gsl_vector_set(w, i, yerror[i]);
						break;
					case nsl_fit_weight_inverse: // yerror are inverse weights
						gsl_vector_set(w, i, 1. / std::max(yerror[i], std::max(minError, std::abs(yi) * 1.e-15)));
						break;
					case nsl_fit_weight_statistical:
						gsl_vector_set(w, i, 1. / std::max(yi, minError));
						break;
					case nsl_fit_weight_relative:
						gsl_vector_set(w, i, 1. / std::max(gsl_pow_2(yi), minError));
						break;
					}
				} else
					gsl_vector_set(w, i, 1.);
			}

			auto* work = gsl_multifit_linear_alloc(n, np);
			double chisq;
			int status = gsl_multifit_wlinear(X, w, y, c, cov, &chisq, work);
			gsl_multifit_linear_free(work);
			gsl_vector_free(w);

			if (paramStartValues.size() < np) {
				DEBUG(Q_FUNC_INFO << ", WARNING: start value vector is smaller than np! (" << paramStartValues.size() << " < " << np << ")")
				paramStartValues.resize(np);
			}
			for (int i = 0; i < np; i++) {
				const auto value = gsl_vector_get(c, i);
				if (!std::isnan(value))
					paramStartValues[i] = value;
			}

			// results
			d->fitResult = XYFitCurve::FitResult(); // clear result

			d->fitResult.available = true;
			d->fitResult.valid = true;
			d->fitResult.status = gslErrorToString(status);

			d->fitResult.sse = chisq;
			d->fitResult.dof = n - np;
			// SST needed for coefficient of determination, R-squared and F test
			d->fitResult.sst = gsl_stats_tss(y->data, 1, n);
			// for a linear model without intercept R-squared is calculated differently
			// also using alternative R^2 when R^2 would be negative
			if (degree == 1 || d->fitResult.sst < d->fitResult.sse)
				d->fitResult.sst = gsl_stats_tss_m(y->data, 1, n, 0);

			d->fitResult.calculateResult(n, np);

			d->fitResult.paramValues.resize(np);
			d->fitResult.errorValues.resize(np);
			d->fitResult.tdist_tValues.resize(np);
			d->fitResult.tdist_pValues.resize(np);
			d->fitResult.marginValues.resize(np);

			const double cerr = sqrt(d->fitResult.rms);
			// CI = 100 * (1 - alpha)
			const double alpha = 1.0 - fitData.confidenceInterval / 100.;
			for (unsigned int i = 0; i < np; i++) {
				for (unsigned int j = 0; j <= i; j++)
					d->fitResult.correlationMatrix << gsl_matrix_get(cov, i, j) / sqrt(gsl_matrix_get(cov, i, i)) / sqrt(gsl_matrix_get(cov, j, j));
				d->fitResult.paramValues[i] = gsl_vector_get(c, i);
				d->fitResult.errorValues[i] = cerr * sqrt(gsl_matrix_get(cov, i, i));
				d->fitResult.tdist_tValues[i] = nsl_stats_tdist_t(d->fitResult.paramValues.at(i), d->fitResult.errorValues.at(i));
				d->fitResult.tdist_pValues[i] = nsl_stats_tdist_p(d->fitResult.tdist_tValues.at(i), d->fitResult.dof);
				d->fitResult.marginValues[i] = nsl_stats_tdist_margin(alpha, d->fitResult.dof, d->fitResult.errorValues.at(i));
			}

			// residuals
			gsl_vector* r = gsl_vector_alloc(n);
			status = gsl_multifit_linear_residuals(X, y, c, r);
			if (!status)
				d->fitResult.mae = gsl_blas_dasum(r) / n;
			// TODO: show residuals?

			gsl_matrix_free(X);
			gsl_vector_free(y);
			gsl_vector_free(c);
			gsl_matrix_free(cov);
			break;
		}
		// TODO: use regression for all basic models?
		case nsl_fit_model_power: // a x^b, a + b x^c
		case nsl_fit_model_exponential: // a e^(bx), a1 e^(b1 x) + a2 e^(b2 x), ...
		case nsl_fit_model_inverse_exponential: // a (1-e^(bx)) + c
		case nsl_fit_model_fourier: // a0 + a1*sin(x) + b1 * cos(x) + ...
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
				paramStartValues[3 * d + 2] = xmin + (d + 1.) * xrange / (degree + 1.); // mu
				paramStartValues[3 * d + 1] = xrange / (10. * degree); // sigma
				paramStartValues[3 * d] = paramStartValues[3 * d + 1] * ymax; // A = sigma * ymax
			}
			break;
		case nsl_fit_model_voigt:
			for (int d = 0; d < degree; d++) {
				paramStartValues[4 * d + 1] = xmin + (d + 1.) * xrange / (degree + 1.); // mu
				paramStartValues[4 * d + 2] = xrange / (10. * degree); // sigma
				paramStartValues[4 * d + 3] = xrange / (10. * degree); // gamma
			}
			break;
		case nsl_fit_model_pseudovoigt1:
			for (int d = 0; d < degree; d++) {
				paramStartValues[4 * d + 1] = 0.5; // eta
				paramStartValues[4 * d + 2] = xrange / (10. * degree); // sigma
				paramStartValues[4 * d + 3] = xmin + (d + 1.) * xrange / (degree + 1.); // mu
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
			paramStartValues[1] = (xmax + xmin) / 2.;
			paramStartValues[2] = xrange / 10.;
			break;
		case nsl_fit_model_hill:
			paramStartValues[2] = xrange / 10.;
			break;
		case nsl_fit_model_gompertz:
			// TODO
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
			paramStartValues[2] = (xmin + xmax) / 2.;
			// sigma
			paramStartValues[1] = xrange / 10.;
			// A = sigma * y_max
			paramStartValues[0] = paramStartValues[1] * ymax;
			break;
		// TODO: other types
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
void XYFitCurve::initFitData(XYAnalysisCurve::AnalysisAction action) {
	// TODO: exclude others too?
	if (action == XYAnalysisCurve::AnalysisAction::DataReduction)
		return;

	Q_D(XYFitCurve);
	XYFitCurve::FitData& fitData = d->fitData;
	if (action == XYAnalysisCurve::AnalysisAction::FitLinear) {
		// Linear
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_polynomial;
		fitData.degree = 1;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitPower) {
		// Power
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_power;
		fitData.degree = 1;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitExp1) {
		// Exponential (degree 1)
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_exponential;
		fitData.degree = 1;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitExp2) {
		// Exponential (degree 2)
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_exponential;
		fitData.degree = 2;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitInvExp) {
		// Inverse exponential
		fitData.modelCategory = nsl_fit_model_basic;
		fitData.modelType = (int)nsl_fit_model_inverse_exponential;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitGauss) {
		// Gauss
		fitData.modelCategory = nsl_fit_model_peak;
		fitData.modelType = (int)nsl_fit_model_gaussian;
		fitData.degree = 1;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitCauchyLorentz) {
		// Cauchy-Lorentz
		fitData.modelCategory = nsl_fit_model_peak;
		fitData.modelType = (int)nsl_fit_model_lorentz;
		fitData.degree = 1;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitTan) {
		// Arc tangent
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = (int)nsl_fit_model_atan;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitTanh) {
		// Hyperbolic tangent
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = (int)nsl_fit_model_tanh;
	} else if (action == XYAnalysisCurve::AnalysisAction::FitErrFunc) {
		// Error function
		fitData.modelCategory = nsl_fit_model_growth;
		fitData.modelType = (int)nsl_fit_model_erf;
	} else {
		// Custom
		fitData.modelCategory = nsl_fit_model_custom;
		fitData.modelType = 0;
	}

	XYFitCurve::initFitData(fitData);
}

/*!
 * sets the model expression and the parameter names for given model category, model type and degree in \c fitData
 */
void XYFitCurve::initFitData(XYFitCurve::FitData& fitData) {
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
		DEBUG(Q_FUNC_INFO << ", for model category = " << nsl_fit_model_category_name[modelCategory] << ", model type = " << modelType
						  << ", degree = " << degree);
		paramNames.clear();
	} else {
		DEBUG(Q_FUNC_INFO << ", for model category = nsl_fit_model_custom, model type = " << modelType << ", degree = " << degree);
	}
	paramNamesUtf8.clear();

	// 10 indices used in multi degree models
	QStringList indices = {UTF8_QSTRING("₁"),
						   UTF8_QSTRING("₂"),
						   UTF8_QSTRING("₃"),
						   UTF8_QSTRING("₄"),
						   UTF8_QSTRING("₅"),
						   UTF8_QSTRING("₆"),
						   UTF8_QSTRING("₇"),
						   UTF8_QSTRING("₈"),
						   UTF8_QSTRING("₉"),
						   UTF8_QSTRING("₁₀")};

	switch (modelCategory) {
	case nsl_fit_model_basic:
		model = QLatin1String(nsl_fit_model_basic_equation[fitData.modelType]);
		switch (modelType) {
		case nsl_fit_model_polynomial:
			paramNames << QStringLiteral("c0") << QStringLiteral("c1");
			paramNamesUtf8 << UTF8_QSTRING("c₀") << UTF8_QSTRING("c₁");
			if (degree == 2) {
				model += QStringLiteral(" + c2*x^2");
				paramNames << QStringLiteral("c2");
				paramNamesUtf8 << UTF8_QSTRING("c₂");
			} else if (degree > 2) {
				for (int i = 2; i <= degree; ++i) {
					QString numStr = QString::number(i);
					model += QStringLiteral("+c") + numStr + QStringLiteral("*x^") + numStr;
					paramNames << QStringLiteral("c") + numStr;
					paramNamesUtf8 << QStringLiteral("c") + indices[i - 1];
				}
			}
			break;
		case nsl_fit_model_power:
			if (degree == 1) {
				paramNames << QStringLiteral("a") << QStringLiteral("b");
			} else {
				paramNames << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c");
				model = QStringLiteral("a + b*x^c");
			}
			break;
		case nsl_fit_model_exponential:
			if (degree == 1) {
				paramNames << QStringLiteral("a") << QStringLiteral("b");
			} else {
				for (int i = 1; i <= degree; i++) {
					QString numStr = QString::number(i);
					if (i == 1)
						model = QStringLiteral("a1*exp(b1*x)");
					else
						model += QStringLiteral(" + a") + numStr + QStringLiteral("*exp(b") + numStr + QStringLiteral("*x)");
					paramNames << QStringLiteral("a") + numStr << QStringLiteral("b") + numStr;
					paramNamesUtf8 << QStringLiteral("a") + indices[i - 1] << QStringLiteral("b") + indices[i - 1];
				}
			}
			break;
		case nsl_fit_model_inverse_exponential:
			paramNames << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c");
			break;
		case nsl_fit_model_fourier:
			paramNames << QStringLiteral("w") << QStringLiteral("a0") << QStringLiteral("a1") << QStringLiteral("b1");
			paramNamesUtf8 << UTF8_QSTRING("ω") << UTF8_QSTRING("a₀") << UTF8_QSTRING("a₁") << UTF8_QSTRING("b₁");
			if (degree > 1) {
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					model += QStringLiteral("+ (a") + numStr + QStringLiteral("*cos(") + numStr + QStringLiteral("*w*x) + b") + numStr + QStringLiteral("*sin(")
						+ numStr + QStringLiteral("*w*x))");
					paramNames << QStringLiteral("a") + numStr << QStringLiteral("b") + numStr;
					paramNamesUtf8 << QStringLiteral("a") + indices[i - 1] << QStringLiteral("b") + indices[i - 1];
				}
			}
			break;
		}
		break;
	case nsl_fit_model_peak:
		model = QLatin1String(nsl_fit_model_peak_equation[fitData.modelType]);
		switch (modelType) {
		case nsl_fit_model_gaussian:
			switch (degree) {
			case 1:
				paramNames << QStringLiteral("a") << QStringLiteral("s") << QStringLiteral("mu");
				paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
				break;
			default:
				model = QStringLiteral("1/sqrt(2*pi) * (");
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += QStringLiteral(" + ");
					model += QStringLiteral("a") + numStr + QStringLiteral("/s") + numStr + QStringLiteral("* exp(-((x-mu") + numStr + QStringLiteral(")/s")
						+ numStr + QStringLiteral(")^2/2)");
					paramNames << QStringLiteral("a") + numStr << QStringLiteral("s") + numStr << QStringLiteral("mu") + numStr;
					paramNamesUtf8 << QStringLiteral("A") + indices[i - 1] << UTF8_QSTRING("σ") + indices[i - 1] << UTF8_QSTRING("μ") + indices[i - 1];
				}
				model += QStringLiteral(")");
			}
			break;
		case nsl_fit_model_lorentz:
			switch (degree) {
			case 1:
				paramNames << QStringLiteral("a") << QStringLiteral("g") << QStringLiteral("mu");
				paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("γ") << UTF8_QSTRING("μ");
				break;
			default:
				model = QStringLiteral("1/pi * (");
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += QStringLiteral(" + ");
					model += QStringLiteral("a") + numStr + QStringLiteral(" * g") + numStr + QStringLiteral("/(g") + numStr + QStringLiteral("^2+(x-mu")
						+ numStr + QStringLiteral(")^2)");
					paramNames << QStringLiteral("a") + numStr << QStringLiteral("g") + numStr << QStringLiteral("mu") + numStr;
					paramNamesUtf8 << QStringLiteral("A") + indices[i - 1] << UTF8_QSTRING("γ") + indices[i - 1] << UTF8_QSTRING("μ") + indices[i - 1];
				}
				model += QStringLiteral(")");
			}
			break;
		case nsl_fit_model_sech:
			switch (degree) {
			case 1:
				paramNames << QStringLiteral("a") << QStringLiteral("s") << QStringLiteral("mu");
				paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
				break;
			default:
				model = QStringLiteral("1/pi * (");
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += QStringLiteral(" + ");
					model += QStringLiteral("a") + numStr + QStringLiteral("/s") + numStr + QStringLiteral("* sech((x-mu") + numStr + QStringLiteral(")/s")
						+ numStr + QStringLiteral(")");
					paramNames << QStringLiteral("a") + numStr << QStringLiteral("s") + numStr << QStringLiteral("mu") + numStr;
					paramNamesUtf8 << QStringLiteral("A") + indices[i - 1] << UTF8_QSTRING("σ") + indices[i - 1] << UTF8_QSTRING("μ") + indices[i - 1];
				}
				model += QStringLiteral(")");
			}
			break;
		case nsl_fit_model_logistic:
			switch (degree) {
			case 1:
				paramNames << QStringLiteral("a") << QStringLiteral("s") << QStringLiteral("mu");
				paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
				break;
			default:
				model = QStringLiteral("1/4 * (");
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += QStringLiteral(" + ");
					model += QStringLiteral("a") + numStr + QStringLiteral("/s") + numStr + QStringLiteral("* sech((x-mu") + numStr + QStringLiteral(")/2/s")
						+ numStr + QStringLiteral(")**2");
					paramNames << QStringLiteral("a") + numStr << QStringLiteral("s") + numStr << QStringLiteral("mu") + numStr;
					paramNamesUtf8 << QStringLiteral("A") + indices[i - 1] << UTF8_QSTRING("σ") + indices[i - 1] << UTF8_QSTRING("μ") + indices[i - 1];
				}
				model += QStringLiteral(")");
			}
			break;
		case nsl_fit_model_voigt:
			switch (degree) {
			case 1:
				paramNames << QStringLiteral("a") << QStringLiteral("mu") << QStringLiteral("s") << QStringLiteral("g");
				paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("μ") << UTF8_QSTRING("σ") << UTF8_QSTRING("γ");
				break;
			default:
				model.clear();
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += QStringLiteral(" + ");
					model += QStringLiteral("a") + numStr + QStringLiteral("*voigt(x-mu") + numStr + QStringLiteral(",s") + numStr + QStringLiteral(",g")
						+ numStr + QStringLiteral(")");
					paramNames << QStringLiteral("a") + numStr << QStringLiteral("mu") + numStr << QStringLiteral("s") + numStr << QStringLiteral("g") + numStr;
					paramNamesUtf8 << QStringLiteral("A") + indices[i - 1] << UTF8_QSTRING("μ") + indices[i - 1] << UTF8_QSTRING("σ") + indices[i - 1]
								   << UTF8_QSTRING("γ") + indices[i - 1];
				}
			}
			break;
		case nsl_fit_model_pseudovoigt1:
			switch (degree) {
			case 1:
				paramNames << QStringLiteral("a") << QStringLiteral("et") << QStringLiteral("w") << QStringLiteral("mu"); // eta already exists as function!
				paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("η") << QStringLiteral("w") << UTF8_QSTRING("μ");
				break;
			default:
				model.clear();
				for (int i = 1; i <= degree; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						model += QStringLiteral(" + ");
					model += QStringLiteral("a") + numStr + QStringLiteral("*pseudovoigt1(x-mu") + numStr + QStringLiteral(",eta") + numStr
						+ QStringLiteral(",w") + numStr + QStringLiteral(")");
					paramNames << QStringLiteral("a") + numStr << QStringLiteral("eta") + numStr << QStringLiteral("w") + numStr
							   << QStringLiteral("mu") + numStr;
					paramNamesUtf8 << QStringLiteral("A") + indices[i - 1] << UTF8_QSTRING("η") + indices[i - 1] << QStringLiteral("w") + indices[i - 1]
								   << UTF8_QSTRING("μ") + indices[i - 1];
				}
			}
			break;
		}
		break;
	case nsl_fit_model_growth:
		model = QLatin1String(nsl_fit_model_growth_equation[fitData.modelType]);
		switch (modelType) {
		case nsl_fit_model_atan:
		case nsl_fit_model_tanh:
		case nsl_fit_model_algebraic_sigmoid:
		case nsl_fit_model_erf:
		case nsl_fit_model_gudermann:
			paramNames << QStringLiteral("a") << QStringLiteral("mu") << QStringLiteral("s");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("μ") << UTF8_QSTRING("σ");
			break;
		case nsl_fit_model_sigmoid:
			paramNames << QStringLiteral("a") << QStringLiteral("mu") << QStringLiteral("k");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("μ") << QStringLiteral("k");
			break;
		case nsl_fit_model_hill:
			paramNames << QStringLiteral("a") << QStringLiteral("n") << QStringLiteral("a");
			paramNamesUtf8 << QStringLiteral("A") << QStringLiteral("n") << UTF8_QSTRING("σ");
			break;
		case nsl_fit_model_gompertz:
			paramNames << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c");
			break;
		}
		break;
	case nsl_fit_model_distribution:
		model = QLatin1String(nsl_sf_stats_distribution_equation[fitData.modelType]);
		switch (modelType) {
		case nsl_sf_stats_gaussian:
		case nsl_sf_stats_laplace:
		case nsl_sf_stats_rayleigh_tail:
		case nsl_sf_stats_lognormal:
		case nsl_sf_stats_logistic:
		case nsl_sf_stats_sech:
			paramNames << QStringLiteral("a") << QStringLiteral("s") << QStringLiteral("mu");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_gaussian_tail:
			paramNames << QStringLiteral("A") << QStringLiteral("s") << QStringLiteral("a") << QStringLiteral("mu");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ") << QStringLiteral("a") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_exponential:
			paramNames << QStringLiteral("a") << QStringLiteral("l") << QStringLiteral("mu");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("λ") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_exponential_power:
			paramNames << QStringLiteral("a") << QStringLiteral("s") << QStringLiteral("b") << QStringLiteral("mu");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ") << QStringLiteral("b") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_cauchy_lorentz:
		case nsl_sf_stats_levy:
			paramNames << QStringLiteral("a") << QStringLiteral("g") << QStringLiteral("mu");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("γ") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_rayleigh:
			paramNames << QStringLiteral("a") << QStringLiteral("s");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ");
			break;
		case nsl_sf_stats_landau:
			paramNames << QStringLiteral("a");
			paramNamesUtf8 << QStringLiteral("A");
			break;
		case nsl_sf_stats_levy_alpha_stable: // unused distributions
		case nsl_sf_stats_levy_skew_alpha_stable:
		case nsl_sf_stats_bernoulli:
			break;
		case nsl_sf_stats_gamma:
			paramNames << QStringLiteral("a") << QStringLiteral("k") << QStringLiteral("t");
			paramNamesUtf8 << QStringLiteral("A") << QStringLiteral("k") << UTF8_QSTRING("θ");
			break;
		case nsl_sf_stats_flat:
			paramNames << QStringLiteral("A") << QStringLiteral("b") << QStringLiteral("a");
			break;
		case nsl_sf_stats_chi_squared:
			paramNames << QStringLiteral("a") << QStringLiteral("n");
			paramNamesUtf8 << QStringLiteral("A") << QStringLiteral("n");
			break;
		case nsl_sf_stats_fdist:
			paramNames << QStringLiteral("a") << QStringLiteral("n1") << QStringLiteral("n2");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("ν₁") << UTF8_QSTRING("ν₂");
			break;
		case nsl_sf_stats_tdist:
			paramNames << QStringLiteral("a") << QStringLiteral("n");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("ν");
			break;
		case nsl_sf_stats_beta:
		case nsl_sf_stats_pareto:
			paramNames << QStringLiteral("A") << QStringLiteral("a") << QStringLiteral("b");
			break;
		case nsl_sf_stats_weibull:
			paramNames << QStringLiteral("a") << QStringLiteral("k") << QStringLiteral("l") << QStringLiteral("mu");
			paramNamesUtf8 << QStringLiteral("A") << QStringLiteral("k") << UTF8_QSTRING("λ") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_gumbel1:
			paramNames << QStringLiteral("a") << QStringLiteral("s") << QStringLiteral("mu") << QStringLiteral("b");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ") << UTF8_QSTRING("μ") << UTF8_QSTRING("β");
			break;
		case nsl_sf_stats_gumbel2:
			paramNames << QStringLiteral("A") << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("mu");
			paramNamesUtf8 << QStringLiteral("A") << QStringLiteral("a") << QStringLiteral("b") << UTF8_QSTRING("μ");
			break;
		case nsl_sf_stats_poisson:
			paramNames << QStringLiteral("a") << QStringLiteral("l");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("λ");
			break;
		case nsl_sf_stats_binomial:
		case nsl_sf_stats_negative_binomial:
		case nsl_sf_stats_pascal:
			paramNames << QStringLiteral("a") << QStringLiteral("p") << QStringLiteral("n");
			paramNamesUtf8 << QStringLiteral("A") << QStringLiteral("p") << QStringLiteral("n");
			break;
		case nsl_sf_stats_geometric:
		case nsl_sf_stats_logarithmic:
			paramNames << QStringLiteral("a") << QStringLiteral("p");
			paramNamesUtf8 << QStringLiteral("A") << QStringLiteral("p");
			break;
		case nsl_sf_stats_hypergeometric:
			paramNames << QStringLiteral("a") << QStringLiteral("n1") << QStringLiteral("n2") << QStringLiteral("t");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("n₁") << UTF8_QSTRING("n₂") << QStringLiteral("t");
			break;
		case nsl_sf_stats_maxwell_boltzmann:
			paramNames << QStringLiteral("a") << QStringLiteral("s");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("σ");
			break;
		case nsl_sf_stats_frechet:
			paramNames << QStringLiteral("a") << QStringLiteral("g") << QStringLiteral("s") << QStringLiteral("mu");
			paramNamesUtf8 << QStringLiteral("A") << UTF8_QSTRING("γ") << UTF8_QSTRING("σ") << UTF8_QSTRING("μ");
			break;
		}
		break;
	case nsl_fit_model_custom:
		break;
	}
	DEBUG(Q_FUNC_INFO << ", model: " << STDSTRING(model));
	DEBUG(Q_FUNC_INFO << ", # params: " << paramNames.size());

	if (paramNamesUtf8.isEmpty())
		paramNamesUtf8 << paramNames;

	// resize the vector for the start values and set the elements to 1.0
	// in case a custom model is used, do nothing, we take over the previous values
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

void XYFitCurve::clearFitResult() {
	DEBUG(Q_FUNC_INFO)
	Q_D(XYFitCurve);
	d->fitResult = XYFitCurve::FitResult();
}

void XYFitCurve::FitResult::calculateResult(size_t n, unsigned int np) {
	if (dof != 0) {
		rms = sse / dof;
		rsd = std::sqrt(rms);
	}

	mse = sse / n;
	rmse = std::sqrt(mse);

	rsquare = nsl_stats_rsquare(sse, sst);
	rsquareAdj = nsl_stats_rsquareAdj(rsquare, np, dof, 1);
	chisq_p = nsl_stats_chisq_p(sse, dof);
	fdist_F = nsl_stats_fdist_F(rsquare, np, dof);
	fdist_p = nsl_stats_fdist_p(fdist_F, np, dof);
	logLik = nsl_stats_logLik(sse, n);
	aic = nsl_stats_aic(sse, n, np, 1);
	bic = nsl_stats_bic(sse, n, np, 1);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFitCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve"));
}

void XYFitCurve::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	const auto* column = dynamic_cast<const Column*>(aspect);
	if (!column)
		return;

	XYAnalysisCurve::handleAspectUpdated(aspectPath, aspect);

	setUndoAware(true);
	if (xErrorColumnPath() == aspectPath)
		setXErrorColumn(column);
	if (yErrorColumnPath() == aspectPath)
		setYErrorColumn(column);
	setUndoAware(false);
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
const AbstractColumn* XYFitCurve::residualsColumn() const {
	Q_D(const XYFitCurve);
	return d->residualsColumn;
}

BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, xErrorColumn, xErrorColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, yErrorColumn, yErrorColumn)
const QString& XYFitCurve::xErrorColumnPath() const {
	Q_D(const XYFitCurve);
	return d->xErrorColumnPath;
}
const QString& XYFitCurve::yErrorColumnPath() const {
	Q_D(const XYFitCurve);
	return d->yErrorColumnPath;
}

BASIC_SHARED_D_READER_IMPL(XYFitCurve, const Histogram*, dataSourceHistogram, dataSourceHistogram)
const QString& XYFitCurve::dataSourceHistogramPath() const {
	Q_D(const XYFitCurve);
	return d->dataSourceHistogramPath;
}

BASIC_SHARED_D_READER_IMPL(XYFitCurve, XYFitCurve::FitData, fitData, fitData)

const XYFitCurve::FitResult& XYFitCurve::fitResult() const {
	Q_D(const XYFitCurve);
	return d->fitResult;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYFitCurve, SetDataSourceHistogram, const Histogram*, dataSourceHistogram, retransform)
void XYFitCurve::setDataSourceHistogram(const Histogram* histogram) {
	Q_D(XYFitCurve);
	if (histogram != d->dataSourceHistogram) {
		exec(new XYFitCurveSetDataSourceHistogramCmd(d, histogram, ki18n("%1: data source histogram changed")));
		handleSourceDataChanged();

		connect(histogram, &Histogram::dataChanged, this, &XYFitCurve::handleSourceDataChanged);
		// TODO: add disconnect in the undo-function
	}
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetXErrorColumn, const AbstractColumn*, xErrorColumn)
void XYFitCurve::setXErrorColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->xErrorColumn) {
		exec(new XYFitCurveSetXErrorColumnCmd(d, column, ki18n("%1: assign x-error")));
		handleSourceDataChanged();
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, [=]() {
				handleSourceDataChanged();
			});
			// TODO: disconnect on undo
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
			connect(column, &AbstractColumn::dataChanged, this, [=]() {
				handleSourceDataChanged();
			});
			// TODO: disconnect on undo
		}
	}
}

// do not recalculate (allow preview)
// STD_SETTER_CMD_IMPL_F_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData, recalculate)
STD_SETTER_CMD_IMPL_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData)
void XYFitCurve::setFitData(const XYFitCurve::FitData& fitData) {
	Q_D(XYFitCurve);
	exec(new XYFitCurveSetFitDataCmd(d, fitData, ki18n("%1: set fit options and perform the fit")));
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYFitCurvePrivate::XYFitCurvePrivate(XYFitCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYFitCurvePrivate::~XYFitCurvePrivate() = default;

// data structure to pass parameter to fit functions
struct data {
	size_t n; // number of data points
	double* x; // pointer to the vector with x-data values
	double* y; // pointer to the vector with y-data values
	double* weight; // pointer to the vector with weight values
	nsl_fit_model_category modelCategory;
	int modelType;
	int degree;
	QString* func; // string containing the formula of the model/function
	QStringList* paramNames; // names of parameter
	double* paramMin; // lower parameter limits
	double* paramMax; // upper parameter limits
	bool* paramFixed; // are the parameter fixed?
};

/*!
 * \param paramValues vector containing current values of the fit parameters
 * \param params
 * \param f vector with the weighted residuals weight[i]*(Yi - y[i])
 */
int func_f(const gsl_vector* paramValues, void* params, gsl_vector* f) {
	// DEBUG(Q_FUNC_INFO);
	size_t n = ((struct data*)params)->n;
	double* x = ((struct data*)params)->x;
	double* y = ((struct data*)params)->y;
	double* weight = ((struct data*)params)->weight;
	nsl_fit_model_category modelCategory = ((struct data*)params)->modelCategory;
	unsigned int modelType = ((struct data*)params)->modelType;
	QStringList* paramNames = ((struct data*)params)->paramNames;
	double* min = ((struct data*)params)->paramMin;
	double* max = ((struct data*)params)->paramMax;

	Parser::Parser parser;

	// set current values of the parameters
	for (int i = 0; i < paramNames->size(); i++) {
		double v = gsl_vector_get(paramValues, (size_t)i);
		// bound values if limits are set
		parser.assign_symbol(qPrintable(paramNames->at(i)), nsl_fit_map_bound(v, min[i], max[i]));
		QDEBUG(Q_FUNC_INFO << ", Parameter" << i << " (' " << paramNames->at(i) << "')" << '[' << min[i] << ',' << max[i]
						   << "] free/bound:" << QString::number(v, 'g', 15) << ' ' << QString::number(nsl_fit_map_bound(v, min[i], max[i]), 'g', 15));
	}

	QString func{*(((struct data*)params)->func)};
	for (size_t i = 0; i < n; i++) {
		if (std::isnan(x[i]) || std::isnan(y[i]))
			continue;

		// checks for allowed values of x for different models
		// TODO: more to check
		if (modelCategory == nsl_fit_model_distribution && modelType == nsl_sf_stats_lognormal) {
			if (x[i] < 0)
				x[i] = 0;
		}

		parser.assign_symbol("x", x[i]);
		// DEBUG("evaluate function \"" << STDSTRING(func) << "\" @ x = " << x[i] << ":");
		double Yi = parser.parse(qPrintable(func), qPrintable(QLocale().name()));
		if (parser.parseErrors() > 0) // fallback to default locale
			Yi = parser.parse(qPrintable(func), "en_US");
		// DEBUG("	f(x["<< i <<"]) = " << Yi);

		if (parser.parseErrors() > 0)
			return GSL_EINVAL;

		// DEBUG("	weight["<< i <<"]) = " << weight[i]);
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
	// DEBUG(Q_FUNC_INFO);
	const size_t n = ((struct data*)params)->n;
	double* xVector = ((struct data*)params)->x;
	double* weight = ((struct data*)params)->weight;
	auto modelCategory = ((struct data*)params)->modelCategory;
	unsigned int modelType = ((struct data*)params)->modelType;
	unsigned int degree = ((struct data*)params)->degree;
	auto* paramNames = ((struct data*)params)->paramNames;
	double* min = ((struct data*)params)->paramMin;
	double* max = ((struct data*)params)->paramMax;
	bool* fixed = ((struct data*)params)->paramFixed;

	// calculate the Jacobian matrix:
	// Jacobian matrix J(i,j) = df_i / dx_j
	// where f_i = w_i*(Y_i - y_i),
	// Y_i = model and the x_j are the parameters
	double x;

	switch (modelCategory) {
	case nsl_fit_model_basic:
		switch (modelType) {
		case nsl_fit_model_polynomial: // Y(x) = c0 + c1*x + ... + cn*x^n
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
		case nsl_fit_model_power: // Y(x) = a*x^b or Y(x) = a + b*x^c.
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
		case nsl_fit_model_exponential: { // Y(x) = a*exp(b*x) + c*exp(d*x) + ...
			double* p = new double[2 * degree];
			for (unsigned int i = 0; i < 2 * degree; i++)
				p[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, i), min[i], max[i]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < 2 * degree; j++) {
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
					else
						gsl_matrix_set(J, (size_t)i, (size_t)j, nsl_fit_model_exponentialn_param_deriv(j, x, p, weight[i]));
				}
			}
			delete[] p;

			break;
		}
		case nsl_fit_model_inverse_exponential: { // Y(x) = a*(1-exp(b*x))+c
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
		case nsl_fit_model_fourier: { // Y(x) = a0 + (a1*cos(w*x) + b1*sin(w*x)) + ... + (an*cos(n*w*x) + bn*sin(n*w*x)
			// parameters: w, a0, a1, b1, ... an, bn
			double* a = new double[degree];
			double* b = new double[degree];
			double w = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			a[0] = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			b[0] = 0;
			for (unsigned int i = 1; i < degree; ++i) {
				a[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, 2 * i), min[2 * i], max[2 * i]);
				b[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, 2 * i + 1), min[2 * i + 1], max[2 * i + 1]);
			}
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				double wd = 0; // first derivative with respect to the w parameter
				for (unsigned int j = 1; j < degree; ++j) {
					wd += -a[j] * j * x * sin(j * w * x) + b[j] * j * x * cos(j * w * x);
				}

				gsl_matrix_set(J, i, 0, weight[i] * wd);
				gsl_matrix_set(J, i, 1, weight[i]);
				for (unsigned int j = 1; j <= degree; ++j) {
					gsl_matrix_set(J, (size_t)i, (size_t)(2 * j), nsl_fit_model_fourier_param_deriv(0, j, x, w, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(2 * j + 1), nsl_fit_model_fourier_param_deriv(1, j, x, w, weight[i]));
				}

				for (unsigned int j = 0; j <= 2 * degree + 1; j++)
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
					const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3 * j), min[3 * j], max[3 * j]);
					const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 3 * j + 1), min[3 * j + 1], max[3 * j + 1]);
					const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3 * j + 2), min[3 * j + 2], max[3 * j + 2]);

					switch (modelType) {
					case nsl_fit_model_gaussian:
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j), nsl_fit_model_gaussian_param_deriv(0, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j + 1), nsl_fit_model_gaussian_param_deriv(1, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j + 2), nsl_fit_model_gaussian_param_deriv(2, x, a, s, mu, weight[i]));
						break;
					case nsl_fit_model_lorentz: // a,s,t
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j), nsl_fit_model_lorentz_param_deriv(0, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j + 1), nsl_fit_model_lorentz_param_deriv(1, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j + 2), nsl_fit_model_lorentz_param_deriv(2, x, a, s, mu, weight[i]));
						break;
					case nsl_fit_model_sech:
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j), nsl_fit_model_sech_param_deriv(0, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j + 1), nsl_fit_model_sech_param_deriv(1, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j + 2), nsl_fit_model_sech_param_deriv(2, x, a, s, mu, weight[i]));
						break;
					case nsl_fit_model_logistic:
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j), nsl_fit_model_logistic_param_deriv(0, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j + 1), nsl_fit_model_logistic_param_deriv(1, x, a, s, mu, weight[i]));
						gsl_matrix_set(J, (size_t)i, (size_t)(3 * j + 2), nsl_fit_model_logistic_param_deriv(2, x, a, s, mu, weight[i]));
						break;
					}
				}

				for (unsigned int j = 0; j < 3 * degree; j++)
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
			}
			break;
		case nsl_fit_model_voigt:
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < degree; ++j) {
					const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 4 * j), min[4 * j], max[4 * j]);
					const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 4 * j + 1), min[4 * j + 1], max[4 * j + 1]);
					const double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 4 * j + 2), min[4 * j + 2], max[4 * j + 2]);
					const double g = nsl_fit_map_bound(gsl_vector_get(paramValues, 4 * j + 3), min[4 * j + 3], max[4 * j + 3]);

					gsl_matrix_set(J, (size_t)i, (size_t)(4 * j), nsl_fit_model_voigt_param_deriv(0, x, a, mu, s, g, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4 * j + 1), nsl_fit_model_voigt_param_deriv(1, x, a, mu, s, g, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4 * j + 2), nsl_fit_model_voigt_param_deriv(2, x, a, mu, s, g, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4 * j + 3), nsl_fit_model_voigt_param_deriv(3, x, a, mu, s, g, weight[i]));
				}
				for (unsigned int j = 0; j < 4 * degree; j++)
					if (fixed[j])
						gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
			}
			break;
		case nsl_fit_model_pseudovoigt1:
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];

				for (unsigned int j = 0; j < degree; ++j) {
					const double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 4 * j), min[4 * j], max[4 * j]);
					const double eta = nsl_fit_map_bound(gsl_vector_get(paramValues, 4 * j + 1), min[4 * j + 1], max[4 * j + 1]);
					const double w = nsl_fit_map_bound(gsl_vector_get(paramValues, 4 * j + 2), min[4 * j + 2], max[4 * j + 2]);
					const double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 4 * j + 3), min[4 * j + 3], max[4 * j + 3]);

					gsl_matrix_set(J, (size_t)i, (size_t)(4 * j), nsl_fit_model_pseudovoigt1_param_deriv(0, x, a, eta, w, mu, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4 * j + 1), nsl_fit_model_pseudovoigt1_param_deriv(1, x, a, eta, w, mu, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4 * j + 2), nsl_fit_model_pseudovoigt1_param_deriv(2, x, a, eta, w, mu, weight[i]));
					gsl_matrix_set(J, (size_t)i, (size_t)(4 * j + 3), nsl_fit_model_pseudovoigt1_param_deriv(3, x, a, eta, w, mu, weight[i]));
				}
				for (unsigned int j = 0; j < 4 * degree; j++)
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
	} break;
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
		case nsl_sf_stats_maxwell_boltzmann: { // Y(x) = a*sqrt(2/pi) * x^2/s^3 * exp(-(x/s)^2/2)
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
		double value;
		const auto np = paramNames->size();
		QString func{*(((struct data*)params)->func)};

		const auto numberLocale = QLocale();
		Parser::Parser parser;
		for (size_t i = 0; i < n; i++) {
			x = xVector[i];
			parser.assign_symbol("x", x);

			for (auto j = 0; j < np; j++) {
				for (auto k = 0; k < np; k++) {
					if (k != j) {
						value = nsl_fit_map_bound(gsl_vector_get(paramValues, k), min[k], max[k]);
						parser.assign_symbol(qPrintable(paramNames->at(k)), value);
					}
				}

				value = nsl_fit_map_bound(gsl_vector_get(paramValues, j), min[j], max[j]);
				parser.assign_symbol(qPrintable(paramNames->at(j)), value);
				double f_p = parser.parse(qPrintable(func), qPrintable(numberLocale.name()));
				if (parser.parseErrors() > 0) // fallback to default locale
					f_p = parser.parse(qPrintable(func), "en_US");

				double eps = 1.e-9;
				if (std::abs(f_p) > 0)
					eps *= std::abs(f_p); // scale step size with function value
				value += eps;
				parser.assign_symbol(qPrintable(paramNames->at(j)), value);
				double f_pdp = parser.parse(qPrintable(func), qPrintable(numberLocale.name()));
				if (parser.parseErrors() > 0) // fallback to default locale
					f_pdp = parser.parse(qPrintable(func), "en_US");

				//				DEBUG("evaluate deriv"<<func<<": f(x["<<i<<"]) ="<<QString::number(f_p, 'g', 15));
				//				DEBUG("evaluate deriv"<<func<<": f(x["<<i<<"]+dx) ="<<QString::number(f_pdp, 'g', 15));
				//				DEBUG("	deriv = " << STDSTRING(QString::number(sqrt(weight[i])*(f_pdp-f_p)/eps, 'g', 15));

				if (fixed[j])
					gsl_matrix_set(J, (size_t)i, (size_t)j, 0.);
				else // calculate finite difference
					gsl_matrix_set(J, (size_t)i, (size_t)j, sqrt(weight[i]) * (f_pdp - f_p) / eps);
			}
		}
	}

	return GSL_SUCCESS;
}

int func_fdf(const gsl_vector* x, void* params, gsl_vector* f, gsl_matrix* J) {
	// DEBUG(Q_FUNC_INFO);
	func_f(x, params, f);
	func_df(x, params, J);

	return GSL_SUCCESS;
}

//////////////////////////////////////////////////////////////////

/* prepare the fit result columns and note */
void XYFitCurvePrivate::prepareResultColumns() {
	// DEBUG(Q_FUNC_INFO)
	// create fit result columns if not available yet, clear them otherwise

	// Done also in XYAnalysisCurve, but this function will be also called directly() from evaluate()
	// and not via recalculate(). So this is also needed here!
	if (!xColumn) { // all columns are treated together
		DEBUG(Q_FUNC_INFO << ", Creating columns")
		xColumn = new Column(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
		yColumn = new Column(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);

		xVector = static_cast<QVector<double>*>(xColumn->data());
		yVector = static_cast<QVector<double>*>(yColumn->data());

		xColumn->setHidden(true);
		q->addChild(xColumn);

		yColumn->setHidden(true);
		q->addChild(yColumn);

		q->setUndoAware(false);
		q->setXColumn(xColumn);
		q->setYColumn(yColumn);
		q->setUndoAware(true);
	} else {
		DEBUG(Q_FUNC_INFO << ", Clear columns")
		xColumn->invalidateProperties();
		yColumn->invalidateProperties();
		if (xVector)
			xVector->clear();
		if (yVector)
			yVector->clear();
		// TODO: residualsVector->clear();
	}

	if (!resultsNote) {
		resultsNote = new Note(i18nc("Curve fitting", "Fit Results"));
		resultsNote->setFixed(true); // visible in the project explorer but cannot be modified (renamed, deleted, etc.)
		q->addChild(resultsNote);
	}
	if (!residualsColumn) {
		residualsColumn = new Column(QStringLiteral("Residuals"), AbstractColumn::ColumnMode::Double);
		residualsVector = static_cast<QVector<double>*>(residualsColumn->data());
		residualsColumn->setFixed(true); // visible in the project explorer but cannot be modified (renamed, deleted, etc.)
		q->addChild(residualsColumn);
	}
}

void XYFitCurvePrivate::resetResults() {
	fitResult = XYFitCurve::FitResult();
}

void XYFitCurvePrivate::updateResultsNote() {
	DEBUG(Q_FUNC_INFO)
	if (!resultsNote)
		return;

	QString text;
	const auto numberLocale = QLocale();

	text += i18n("Nonlinear Fit to X/Y data");
	if (xDataColumn && yDataColumn)
		text += QStringLiteral(": ") + xDataColumn->name() + QStringLiteral(" / ") + yDataColumn->name();
	text += NEWLINE + NEWLINE;

	// model
	text += i18n("MODEL") + NEWLINE + NEWLINE;
	switch (fitData.modelCategory) {
	case nsl_fit_model_basic:
		text += QLatin1String(nsl_fit_model_basic_name[fitData.modelType]);
		text += QStringLiteral(", ") + i18n("Degree") + QStringLiteral(" ") + numberLocale.toString(fitData.degree);
		break;
	case nsl_fit_model_peak:
		text += QLatin1String(nsl_fit_model_peak_name[fitData.modelType]);
		if (fitData.degree > 1)
			text += QStringLiteral(", ") + numberLocale.toString(fitData.degree) + QStringLiteral(" ") + i18n("Peaks");
		break;
	case nsl_fit_model_growth:
		text += QLatin1String(nsl_fit_model_growth_name[fitData.modelType]);
		break;
	case nsl_fit_model_distribution:
		text += QLatin1String(nsl_sf_stats_distribution_name[fitData.modelType]);
		break;
	case nsl_fit_model_custom:
		text += i18n("Custom");
	}

	text += QStringLiteral(": ") + fitData.model + NEWLINE + NEWLINE;
	DEBUG(Q_FUNC_INFO << ", model: " << fitData.model.toStdString())

	// TODO: errors? (see Origin)
	// TODO: weighting? (see Origin)

	// parameter
	text += i18n("PARAMETERS") + NEWLINE + NEWLINE;

	int np = fitResult.paramValues.size();

	text += TAB + i18n("Value") + TAB + i18n("Uncertainty") + TAB + i18n("Uncertainty,%") + TAB + i18n("t Statistic") + TAB + QStringLiteral("P > |t|") + TAB
		+ i18n("Lower") + TAB + i18n("Upper") + NEWLINE;
	for (int i = 0; i < np; i++) {
		text += fitData.paramNames.at(i) + TAB + numberLocale.toString(fitResult.paramValues.at(i)) + TAB + numberLocale.toString(fitResult.errorValues.at(i))
			+ TAB + TAB + numberLocale.toString(fitResult.errorValues.at(i) / fitResult.paramValues.at(i) * 100.) + TAB + TAB
			+ numberLocale.toString(fitResult.tdist_tValues.at(i)) + TAB
			+ numberLocale.toString(fitResult.tdist_pValues.at(i))
			// TODO: margin2Values?
			+ TAB + numberLocale.toString(fitResult.paramValues.at(i) - fitResult.marginValues.at(i)) + TAB
			+ numberLocale.toString(fitResult.paramValues.at(i) + fitResult.marginValues.at(i)) + NEWLINE;

		// for (unsigned int j = 0; j <= i; j++)
		//	d->fitResult.correlationMatrix << gsl_matrix_get(cov, i, j) / sqrt(gsl_matrix_get(cov, i, i)) / sqrt(gsl_matrix_get(cov, j, j));
	}
	text += NEWLINE;

	// goodness of fit
	text += i18n("GOODNESS OF FIT") + NEWLINE + NEWLINE;

	text += i18n("Sum of squared residuals") + UTF8_QSTRING(" (χ²)") + TAB + TAB + numberLocale.toString(fitResult.sse) + NEWLINE;
	text += i18n("Residuals mean square") + UTF8_QSTRING(" (χ²/dof)") + TAB + TAB + numberLocale.toString(fitResult.rms) + NEWLINE;
	text += i18n("Root mean square deviation") + QStringLiteral(" (RMSD/SD)") + TAB + numberLocale.toString(fitResult.rsd) + NEWLINE;
	text += i18n("Coefficient of determination") + QStringLiteral(" (R²)") + TAB + numberLocale.toString(fitResult.rsquare) + NEWLINE;
	text += i18n("Adj. coefficient of determination") + QStringLiteral(" (R̄²)") + TAB + numberLocale.toString(fitResult.rsquareAdj) + NEWLINE;
	text += UTF8_QSTRING("χ²-") + i18n("Test") + UTF8_QSTRING(" (P > χ²)") + TAB + TAB + TAB + numberLocale.toString(fitResult.chisq_p, 'g', 3) + NEWLINE;
	text += i18n("F-Test") + TAB + TAB + TAB + TAB + numberLocale.toString(fitResult.fdist_F, 'g', 3) + NEWLINE;
	text += QStringLiteral("P > F") + TAB + TAB + TAB + TAB + numberLocale.toString(fitResult.fdist_p, 'g', 3) + NEWLINE;
	text += i18n("Mean absolute error") + QStringLiteral(" (MAE)") + TAB + TAB + numberLocale.toString(fitResult.mae) + NEWLINE;
	text += i18n("Akaike information criterion") + QStringLiteral(" (AIC)") + TAB + numberLocale.toString(fitResult.aic, 'g', 3) + NEWLINE;
	text += i18n("Bayesian information criterion") + QStringLiteral(" (BIC)") + TAB + numberLocale.toString(fitResult.bic, 'g', 3) + NEWLINE;

	resultsNote->setText(text);

	DEBUG("NOTE TEXT: " << resultsNote->text().toStdString())
}

void XYFitCurvePrivate::prepareTmpDataColumn(const AbstractColumn** tmpXDataColumn, const AbstractColumn** tmpYDataColumn) const {
	// prepare source data columns
	DEBUG(Q_FUNC_INFO << ", data source: " << ENUM_TO_STRING(XYAnalysisCurve, DataSourceType, dataSourceType))
	switch (dataSourceType) {
	case XYAnalysisCurve::DataSourceType::Spreadsheet:
		if (!xDataColumn || !yDataColumn)
			break;
		*tmpXDataColumn = xDataColumn;
		*tmpYDataColumn = yDataColumn;
		break;
	case XYAnalysisCurve::DataSourceType::Curve:
		if (!dataSourceCurve)
			break;
		*tmpXDataColumn = dataSourceCurve->xColumn();
		*tmpYDataColumn = dataSourceCurve->yColumn();
		break;
	case XYAnalysisCurve::DataSourceType::Histogram:
		if (!dataSourceHistogram)
			break;
		switch (fitData.algorithm) {
		case nsl_fit_algorithm_lm:
			*tmpXDataColumn = dataSourceHistogram->bins(); // bins
			switch (dataSourceHistogram->normalization()) { // TODO: not exactly
			case Histogram::Normalization::Count:
			case Histogram::Normalization::CountDensity:
				*tmpYDataColumn = dataSourceHistogram->binValues(); // values
				break;
			case Histogram::Normalization::Probability:
			case Histogram::Normalization::ProbabilityDensity:
				*tmpYDataColumn = dataSourceHistogram->binPDValues(); // normalized values
			}
			break;
		case nsl_fit_algorithm_ml:
			*tmpXDataColumn = dataSourceHistogram->dataColumn(); // data
			*tmpYDataColumn = dataSourceHistogram->binPDValues(); // normalized values
			break;
		}
		// debug
		/*for (int i = 0; i < dataSourceHistogram->bins()->rowCount(); i++)
			DEBUG("BINS @ " << i << ": " << dataSourceHistogram->bins()->valueAt(i))
		for (int i = 0; i < dataSourceHistogram->binValues()->rowCount(); i++)
			DEBUG("BINValues @ " << i << ": " << dataSourceHistogram->binValues()->valueAt(i))
		for (int i = 0; i < dataSourceHistogram->binPDValues()->rowCount(); i++)
			DEBUG("BINPDValues @ " << i << ": " << dataSourceHistogram->binPDValues()->valueAt(i))
		*/
		break;
	}
}

bool XYFitCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	QElapsedTimer timer;
	timer.start();

	// determine range of data
	Range<double> xRange{tmpXDataColumn->minimum(), tmpXDataColumn->maximum()};
	if (fitData.autoRange) { // auto x range of data to fit
		fitData.fitRange = xRange;
	} else { // custom x range of data to fit
		if (!fitData.fitRange.isZero()) // avoid problems with user specified zero range
			xRange.setRange(fitData.fitRange.start(), fitData.fitRange.end());
	}
	DEBUG(Q_FUNC_INFO << ", fit data range = " << xRange.start() << " .. " << xRange.end());

	prepareResultColumns();
	const size_t rowCount = tmpXDataColumn->rowCount();

	// fill residuals vector. To get residuals on the correct x values, fill the rest with zeros.
	residualsVector->resize(rowCount);
	// DEBUG("	Residual vector size: " << residualsVector->size())

	DEBUG(Q_FUNC_INFO << ", ALGORITHM: " << nsl_fit_algorithm_name[fitData.algorithm])
	switch (fitData.algorithm) {
	case nsl_fit_algorithm_lm:
		runLevenbergMarquardt(tmpXDataColumn, tmpYDataColumn, xRange);
		break;
	case nsl_fit_algorithm_ml: {
		const double width = xRange.size() / tmpYDataColumn->rowCount();
		double norm = 1.;
		if (dataSourceHistogram) {
			switch (dataSourceHistogram->normalization()) {
			case Histogram::Normalization::Count:
				norm = rowCount * width;
				break;
			case Histogram::Normalization::Probability:
				norm = width;
				break;
			case Histogram::Normalization::CountDensity:
				norm = rowCount;
				break;
			case Histogram::Normalization::ProbabilityDensity:
				break;
			}
		} else { // spreadsheet or curve
			norm = ((Column*)tmpYDataColumn)->statistics().arithmeticMean * xRange.size(); // integral
		}
		runMaximumLikelihood(tmpXDataColumn, norm);
	}
	}

	const bool update = evaluate(); // calculate the fit function (vectors)

	// ML uses dataSourceHistogram->bins() as x for residuals
	if (dataSourceType == XYAnalysisCurve::DataSourceType::Histogram && dataSourceHistogram && fitData.algorithm == nsl_fit_algorithm_ml)
		tmpXDataColumn = dataSourceHistogram->bins();

	if (fitData.autoRange || fitData.algorithm == nsl_fit_algorithm_ml) { // evaluate residuals
		QVector<double> v;
		v.resize(rowCount);
		for (size_t i = 0; i < rowCount; i++)
			if (tmpXDataColumn->isNumeric())
				v[i] = tmpXDataColumn->valueAt(i);
			else if (tmpXDataColumn->columnMode() == AbstractColumn::ColumnMode::DateTime)
				v[i] = tmpXDataColumn->dateTimeAt(i).toMSecsSinceEpoch();

		auto* parser = ExpressionParser::getInstance();
		// fill residualsVector with model values
		// QDEBUG("xVector: " << v)
		bool valid = parser->tryEvaluateCartesian(fitData.model, &v, residualsVector, fitData.paramNames, fitResult.paramValues);
		// QDEBUG("residualsVector: " << *residualsVector)
		if (valid) {
			switch (fitData.algorithm) {
			case nsl_fit_algorithm_lm:
				for (size_t i = 0; i < rowCount; i++)
					(*residualsVector)[i] = tmpYDataColumn->valueAt(i) - (*residualsVector).at(i);
				break;
			case nsl_fit_algorithm_ml:
				for (size_t i = 0; i < rowCount; i++) {
					// DEBUG("y data / column @" << i << ":" << tmpXDataColumn->valueAt(i))
					if (xRange.contains(tmpXDataColumn->valueAt(i)))
						(*residualsVector)[i] = tmpYDataColumn->valueAt(i) - (*residualsVector).at(i);
					else
						(*residualsVector)[i] = 0.;
				}
			}
		} else {
			WARN("	ERROR: Failed parsing residuals")
			residualsVector->clear();
		}
	} // else: see LM method

	residualsColumn->setChanged();

	fitResult.elapsedTime = timer.elapsed();

	return update;
}

void XYFitCurvePrivate::runMaximumLikelihood(const AbstractColumn* tmpXDataColumn, const double norm) {
	const size_t n = tmpXDataColumn->rowCount();

	fitResult.available = true;
	fitResult.valid = true;
	fitResult.status = i18n("Success"); // can it fail in any way?

	const unsigned int np = fitData.paramNames.size(); // number of fit parameters
	fitResult.dof = n - np;
	fitResult.paramValues.resize(np);
	fitResult.errorValues.resize(np);
	fitResult.tdist_tValues.resize(np);
	fitResult.tdist_pValues.resize(np);
	fitResult.marginValues.resize(np);
	fitResult.correlationMatrix.resize(np * (np + 1) / 2);

	DEBUG(Q_FUNC_INFO << ", DISTRIBUTION: " << fitData.modelType)
	fitResult.paramValues[0] = norm; // A - normalization
	// TODO: parameter values (error, etc.)
	// TODO: currently all values are used (data range not changeable)
	const double alpha = 1.0 - fitData.confidenceInterval / 100.;
	const auto& statistics = ((Column*)tmpXDataColumn)->statistics();
	const double mean = statistics.arithmeticMean;
	const double var = statistics.variance;
	const double median = statistics.median;
	const double madmed = statistics.meanDeviationAroundMedian;
	const double iqr = statistics.iqr;
	switch (fitData.modelType) { // only these are supported
	case nsl_sf_stats_gaussian: {
		const double sigma = std::sqrt(var);
		const double mu = mean;
		fitResult.paramValues[1] = sigma;
		fitResult.paramValues[2] = mu;
		DEBUG(Q_FUNC_INFO << ", mu = " << mu << ", sigma = " << sigma)

		fitResult.errorValues[2] = sigma / std::sqrt(n);
		double margin = nsl_stats_tdist_margin(alpha, fitResult.dof, fitResult.errorValues.at(2));
		// DEBUG("z = " << nsl_stats_tdist_z(alpha, fitResult.dof))
		fitResult.marginValues[2] = margin;

		fitResult.errorValues[1] = sigma * sigma / std::sqrt(2 * n);
		margin = nsl_stats_tdist_margin(alpha, fitResult.dof, fitResult.errorValues.at(1));
		// WARN("sigma CONFIDENCE INTERVAL: " << fitResult.paramValues[1] - margin << " .. " << fitResult.paramValues[1] + margin)
		fitResult.marginValues[1] = margin;

		// normalization for spreadsheet or curve
		if (dataSourceType != XYAnalysisCurve::DataSourceType::Histogram)
			fitResult.paramValues[0] /=
				(gsl_sf_erf((tmpXDataColumn->maximum() - mu) / sigma) - gsl_sf_erf((tmpXDataColumn->minimum() - mu) / sigma)) / (2. * std::sqrt(2.));
		break;
	}
	case nsl_sf_stats_exponential: {
		const double mu = tmpXDataColumn->minimum();
		const double lambda = 1. / (mean - mu); // 1/(<x>-\mu)
		fitResult.paramValues[1] = lambda * (1 - 1. / (n - 1)); // unbiased
		fitResult.paramValues[2] = mu;

		fitResult.errorValues[1] = lambda / std::sqrt(n);
		// exact method
		double margin = lambda * (1. - gsl_cdf_chisq_Pinv(alpha / 2., 2 * n) / (2. * n));
		fitResult.marginValues[1] = margin;
		margin = lambda * (gsl_cdf_chisq_Pinv(1. - alpha / 2., 2 * n) / (2. * n) - 1.);
		fitResult.margin2Values.resize(2);
		fitResult.margin2Values[1] = margin;

		DEBUG("error = " << fitResult.errorValues.at(1))
		DEBUG("CI: " << gsl_cdf_chisq_Pinv(alpha / 2., 2 * n) / (2. * n) * lambda << " .. " << gsl_cdf_chisq_Pinv(1. - alpha / 2., 2 * n) / (2. * n) * lambda)
		DEBUG("normal approx.: " << lambda * (1. - 1.96 / std::sqrt(n)) << " .. " << lambda * (1. + 1.96 / std::sqrt(n)))
		DEBUG("1/l = " << 1. / fitResult.paramValues.at(1))
		DEBUG("1/l CI: " << 2. * n / gsl_cdf_chisq_Pinv(1. - alpha / 2., 2 * n) / lambda << " .. " << 2. * n / gsl_cdf_chisq_Pinv(alpha / 2., 2 * n) / lambda)
		DEBUG("1/l normal approx.: " << 1. / lambda / (1. + 1.96 / std::sqrt(n)) << " .. " << 1. / lambda / (1. - 1.96 / std::sqrt(n)))

		// normalization for spreadsheet or curve
		if (dataSourceType != XYAnalysisCurve::DataSourceType::Histogram)
			fitResult.paramValues[0] /= std::exp(-lambda * (tmpXDataColumn->minimum() - mu)) - std::exp(-lambda * (tmpXDataColumn->maximum() - mu));
		break;
	}
	case nsl_sf_stats_laplace: {
		double sigma = madmed;
		if (n > 2) // bias correction
			sigma *= n / (n - 2.);
		const double mu = median;
		fitResult.paramValues[1] = sigma;
		fitResult.paramValues[2] = mu;

		// TODO: error + CI

		// normalization for spreadsheet or curve
		if (dataSourceType != XYAnalysisCurve::DataSourceType::Histogram) {
			double Fmin, Fmax;
			const double xmin = tmpXDataColumn->minimum(), xmax = tmpXDataColumn->maximum();
			if (xmin < mu)
				Fmin = .5 * std::exp((xmin - mu) / sigma);
			else
				Fmin = 1. - .5 * std::exp(-(xmin - mu) / sigma);
			if (xmax < mu)
				Fmax = .5 * std::exp((xmax - mu) / sigma);
			else
				Fmax = 1. - .5 * std::exp(-(xmax - mu) / sigma);

			fitResult.paramValues[0] /= Fmax - Fmin;
		}
		break;
	}
	case nsl_sf_stats_cauchy_lorentz: {
		const double gamma = iqr / 2.;
		const double mu = median; // better truncated mean of middle 24%
		fitResult.paramValues[1] = gamma;
		fitResult.paramValues[2] = mu;

		// TODO: error + CI

		// normalization for spreadsheet or curve
		if (dataSourceType != XYAnalysisCurve::DataSourceType::Histogram)
			fitResult.paramValues[0] /= 1. / M_PI * (atan((tmpXDataColumn->maximum() - mu) / gamma) - atan((tmpXDataColumn->minimum() - mu) / gamma));
		break;
	}
	case nsl_sf_stats_lognormal: {
		// calculate mu and sigma
		double mu = 0.;
		for (size_t i = 0; i < n; i++)
			mu += std::log(tmpXDataColumn->valueAt(i));
		mu /= n;
		double var = 0.;
		for (size_t i = 0; i < n; i++)
			var += gsl_pow_2(std::log(tmpXDataColumn->valueAt(i)) - mu);
		var /= (n - 1);
		const double sigma = std::sqrt(var);
		fitResult.paramValues[1] = sigma;
		fitResult.paramValues[2] = mu;

		// TODO: error + CI

		// normalization for spreadsheet or curve
		if (dataSourceType != XYAnalysisCurve::DataSourceType::Histogram)
			fitResult.paramValues[0] /= gsl_sf_erf((std::log(tmpXDataColumn->maximum()) - mu) / sigma)
				- gsl_sf_erf((std::log(tmpXDataColumn->minimum()) - mu) / sigma) / (2. * std::sqrt(2.));
		break;
	}
	case nsl_sf_stats_poisson: {
		const double lambda = mean;
		fitResult.paramValues[1] = lambda;
		fitResult.errorValues[1] = std::sqrt(lambda / n);

		// double margin = 1.96 * fitResult.errorValues[1];	// normal approx.
		// DEBUG("low / high = " << nsl_stats_chisq_low(alpha, n * lambda) << ", " << nsl_stats_chisq_high(alpha, n * lambda)) // exact formula
		fitResult.marginValues[1] = lambda - nsl_stats_chisq_low(alpha, n * lambda) / n;
		fitResult.margin2Values.resize(2);
		fitResult.margin2Values[1] = nsl_stats_chisq_high(alpha, n * lambda) / n - lambda;

		// normalization for spreadsheet or curve
		if (dataSourceType != XYAnalysisCurve::DataSourceType::Histogram)
			fitResult.paramValues[0] /=
				gsl_sf_gamma_inc_Q(floor(tmpXDataColumn->maximum() + 1), lambda) - gsl_sf_gamma_inc_Q(floor(tmpXDataColumn->minimum() + 1), lambda);
		break;
	}
	case nsl_sf_stats_binomial: {
		const double p = mean / n;
		fitResult.paramValues[1] = p;
		fitResult.paramValues[2] = n;
		fitResult.errorValues[1] = std::sqrt(p * (1 - p) / n);

		// Clopper-Pearson exact method
		const double k = p * n;
		fitResult.marginValues[1] = (p - gsl_cdf_beta_Pinv(alpha / 2., k, n - k + 1)) / std::sqrt(n);
		fitResult.margin2Values.resize(2);
		fitResult.margin2Values[1] = (gsl_cdf_beta_Pinv(1. - alpha / 2., k + 1, n - k) - p) / std::sqrt(n);

		// normalization for spreadsheet or curve
		const double kmin = tmpXDataColumn->minimum(), kmax = tmpXDataColumn->maximum();
		if (dataSourceType != XYAnalysisCurve::DataSourceType::Histogram)
			fitResult.paramValues[0] /= gsl_sf_beta_inc(n - kmax, kmax + 1, 1 - p) - gsl_sf_beta_inc(n - kmin, kmin + 1, 1 - p);
	}
	}

	fitResult.calculateResult(n, np);

	if (fitData.useResults) // set start values
		for (unsigned int i = 0; i < np; i++)
			fitData.paramStartValues.data()[i] = fitResult.paramValues.at(i);
}

void XYFitCurvePrivate::runLevenbergMarquardt(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn, const Range<double> xRange) {
	// fit settings
	const unsigned int maxIters = fitData.maxIterations; // maximal number of iterations
	const double delta = fitData.eps; // fit tolerance
	const auto np = fitData.paramNames.size(); // number of fit parameters
	if (np == 0) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no parameter found.")
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Model has no parameters!");
		return;
	}

	if (yErrorColumn && yErrorColumn->rowCount() < tmpXDataColumn->rowCount()) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Not sufficient weight data points provided!");
		return;
	}

	// copy all valid data point for the fit to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	QVector<double> xerrorVector;
	QVector<double> yerrorVector;

	// logic from XYAnalysisCurve::copyData(), extended by the handling of error columns.
	// TODO: decide how to deal with non-numerical error columns
	int rowCount = std::min(tmpXDataColumn->rowCount(), tmpYDataColumn->rowCount());
	for (int row = 0; row < rowCount; ++row) {
		// omit invalid data
		if (!tmpXDataColumn->isValid(row) || tmpXDataColumn->isMasked(row) || !tmpYDataColumn->isValid(row) || tmpYDataColumn->isMasked(row))
			continue;

		double x = NAN;
		switch (tmpXDataColumn->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
		case AbstractColumn::ColumnMode::Integer:
		case AbstractColumn::ColumnMode::BigInt:
			x = tmpXDataColumn->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::Text: // not valid
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			x = tmpXDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
		}

		double y = NAN;
		switch (tmpYDataColumn->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
		case AbstractColumn::ColumnMode::Integer:
		case AbstractColumn::ColumnMode::BigInt:
			y = tmpYDataColumn->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::Text: // not valid
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			y = tmpYDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
		}

		if (x >= xRange.start() && x <= xRange.end()) { // only when inside given range
			if ((!xErrorColumn && !yErrorColumn) || !fitData.useDataErrors) { // x-y
				xdataVector.append(x);
				ydataVector.append(y);
			} else if (!xErrorColumn && yErrorColumn) { // x-y-dy
				if (!std::isnan(yErrorColumn->valueAt(row))) {
					xdataVector.append(x);
					ydataVector.append(y);
					yerrorVector.append(yErrorColumn->valueAt(row));
				}
			} else if (xErrorColumn && yErrorColumn) { // x-y-dx-dy
				if (!std::isnan(xErrorColumn->valueAt(row)) && !std::isnan(yErrorColumn->valueAt(row))) {
					xdataVector.append(x);
					ydataVector.append(y);
					xerrorVector.append(xErrorColumn->valueAt(row));
					yerrorVector.append(yErrorColumn->valueAt(row));
				}
			}
		}
	}

	// QDEBUG(Q_FUNC_INFO << ", data: " << ydataVector)

	// number of data points to fit
	const auto n = xdataVector.size();
	DEBUG(Q_FUNC_INFO << ", number of data points: " << n);
	if (n == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("No X data available!");
		return;
	}

	if (n < np) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("The number of data points (%1) must be greater than or equal to the number of parameters (%2)!", n, np);
		return;
	}

	if (fitData.model.simplified().isEmpty()) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Fit model not specified!");
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* xerror = xerrorVector.data(); // size may be 0
	double* yerror = yerrorVector.data(); // size may be 0
	DEBUG(Q_FUNC_INFO << ", x error vector size: " << xerrorVector.size());
	DEBUG(Q_FUNC_INFO << ", y error vector size: " << yerrorVector.size());
	double* weight = new double[n];

	for (auto i = 0; i < n; i++)
		weight[i] = 1.;

	const double minError = 1.e-199; // minimum error for weighting

	switch (fitData.yWeightsType) {
	case nsl_fit_weight_no:
	case nsl_fit_weight_statistical_fit:
	case nsl_fit_weight_relative_fit:
		break;
	case nsl_fit_weight_instrumental: // yerror are sigmas
		for (int i = 0; i < (int)n; i++)
			if (i < yerrorVector.size())
				weight[i] = 1. / gsl_pow_2(std::max(yerror[i], std::max(sqrt(minError), std::abs(ydata[i]) * 1.e-15)));
		break;
	case nsl_fit_weight_direct: // yerror are weights
		for (int i = 0; i < (int)n; i++)
			if (i < yerrorVector.size())
				weight[i] = yerror[i];
		break;
	case nsl_fit_weight_inverse: // yerror are inverse weights
		for (int i = 0; i < (int)n; i++)
			if (i < yerrorVector.size())
				weight[i] = 1. / std::max(yerror[i], std::max(minError, std::abs(ydata[i]) * 1.e-15));
		break;
	case nsl_fit_weight_statistical:
		for (int i = 0; i < (int)n; i++)
			weight[i] = 1. / std::max(ydata[i], minError);
		break;
	case nsl_fit_weight_relative:
		for (int i = 0; i < (int)n; i++)
			weight[i] = 1. / std::max(gsl_pow_2(ydata[i]), minError);
		break;
	}

	/////////////////////// GSL >= 2 has a complete new interface! But the old one is still supported. ///////////////////////////
	// GSL >= 2 : "the 'fdf' field of gsl_multifit_function_fdf is now deprecated and does not need to be specified for nonlinear least squares problems"
	int nf = 0; // number of fixed parameter
	// size of paramFixed may be smaller than np
	if (fitData.paramFixed.size() < np)
		fitData.paramFixed.resize(np);
	for (auto i = 0; i < np; i++) {
		const bool fixed = fitData.paramFixed.at(i);
		if (fixed)
			nf++;
		DEBUG("	parameter " << i << " fixed: " << fixed);
	}

	// function to fit
	gsl_multifit_function_fdf f;
	DEBUG(Q_FUNC_INFO << ", model = " << STDSTRING(fitData.model));
	struct data params = {static_cast<size_t>(n),
						  xdata,
						  ydata,
						  weight,
						  fitData.modelCategory,
						  fitData.modelType,
						  fitData.degree,
						  &fitData.model,
						  &fitData.paramNames,
						  fitData.paramLowerLimits.data(),
						  fitData.paramUpperLimits.data(),
						  fitData.paramFixed.data()};
	f.f = &func_f;
	f.df = &func_df;
	f.fdf = &func_fdf;
	f.n = n;
	f.p = np;
	f.params = &params;

	DEBUG(Q_FUNC_INFO << ", initialize the derivative solver (using Levenberg-Marquardt robust solver)");
	const auto* T = gsl_multifit_fdfsolver_lmsder;
	auto* s = gsl_multifit_fdfsolver_alloc(T, n, np);

	DEBUG(Q_FUNC_INFO << ", set start values");
	double* x_init = fitData.paramStartValues.data();
	double* x_min = fitData.paramLowerLimits.data();
	double* x_max = fitData.paramUpperLimits.data();
	DEBUG(Q_FUNC_INFO << ", scale start values if limits are set");
	for (auto i = 0; i < np; i++)
		x_init[i] = nsl_fit_map_unbound(x_init[i], x_min[i], x_max[i]);
	DEBUG(Q_FUNC_INFO << ",	DONE");
	auto x = gsl_vector_view_array(x_init, np);
	DEBUG(Q_FUNC_INFO << ", Turning off GSL error handler to avoid overflow/underflow");
	gsl_set_error_handler_off();
	DEBUG(Q_FUNC_INFO << ", Initialize solver with function f and initial guess x");
	gsl_multifit_fdfsolver_set(s, &f, &x.vector);

	DEBUG(Q_FUNC_INFO << ", Iterate ...");
	int status = GSL_SUCCESS;
	unsigned int iter = 0;
	fitResult.solverOutput.clear();
	writeSolverState(s);
	do {
		iter++;
		DEBUG(Q_FUNC_INFO << ",	iter " << iter);

		// update weights for Y-depending weights (using function values from residuals)
		if (fitData.yWeightsType == nsl_fit_weight_statistical_fit) {
			for (auto i = 0; i < n; i++)
				weight[i] = 1. / (gsl_vector_get(s->f, i) / sqrt(weight[i]) + ydata[i]); // 1/Y_i
		} else if (fitData.yWeightsType == nsl_fit_weight_relative_fit) {
			for (auto i = 0; i < n; i++)
				weight[i] = 1. / gsl_pow_2(gsl_vector_get(s->f, i) / sqrt(weight[i]) + ydata[i]); // 1/Y_i^2
		}

		if (nf == np) { // all fixed parameter
			DEBUG(Q_FUNC_INFO << ", all parameter fixed. Stop iteration.")
			break;
		}
		DEBUG(Q_FUNC_INFO << ", run fdfsolver_iterate");
		status = gsl_multifit_fdfsolver_iterate(s);
		DEBUG(Q_FUNC_INFO << ", fdfsolver_iterate DONE");
		double chi = gsl_blas_dnrm2(s->f);
		writeSolverState(s, chi);
		if (status) {
			DEBUG(Q_FUNC_INFO << ",	iter " << iter << ", status = " << gsl_strerror(status));
			if (status == GSL_ETOLX) // change in the position vector falls below machine precision: no progress
				status = GSL_SUCCESS;
			break;
		}
		if (qFuzzyIsNull(chi)) {
			DEBUG(Q_FUNC_INFO << ", chi is zero! Finishing.")
			status = GSL_SUCCESS;
		} else {
			status = gsl_multifit_test_delta(s->dx, s->x, delta, delta);
		}
		DEBUG(Q_FUNC_INFO << ",	iter " << iter << ", test status = " << gsl_strerror(status));
	} while (status == GSL_CONTINUE && iter < maxIters);

	// second run for x-error fitting
	if (xerrorVector.size() > 0) {
		DEBUG(Q_FUNC_INFO << ", Rerun fit with x errors");

		unsigned int iter2 = 0;
		double chi = 0, chiOld = 0;
		double* fun = new double[n];
		do {
			iter2++;
			chiOld = chi;
			// printf("iter2 = %d\n", iter2);

			// calculate function from residuals
			for (auto i = 0; i < n; i++)
				fun[i] = gsl_vector_get(s->f, i) * 1. / sqrt(weight[i]) + ydata[i];

			// calculate weight[i]
			for (auto i = 0; i < n; i++) {
				// calculate df[i]
				size_t index = i - 1;
				if (i == 0)
					index = i;
				if (i == n - 1)
					index = i - 2;
				double df = (fun[index + 1] - fun[index]) / (xdata[index + 1] - xdata[index]);
				// printf("df = %g\n", df);

				double sigmasq = 1.;
				switch (fitData.xWeightsType) { // x-error type: f'(x)^2*s_x^2 = f'(x)/w_x
				case nsl_fit_weight_no:
					break;
				case nsl_fit_weight_direct: // xerror = w_x
					sigmasq = df * df / std::max(xerror[i], minError);
					break;
				case nsl_fit_weight_instrumental: // xerror = s_x
					sigmasq = df * df * xerror[i] * xerror[i];
					break;
				case nsl_fit_weight_inverse: // xerror = 1/w_x = s_x^2
					sigmasq = df * df * xerror[i];
					break;
				case nsl_fit_weight_statistical: // s_x^2 = 1/w_x = x
					sigmasq = xdata[i];
					break;
				case nsl_fit_weight_relative: // s_x^2 = 1/w_x = x^2
					sigmasq = xdata[i] * xdata[i];
					break;
				case nsl_fit_weight_statistical_fit: // unused
				case nsl_fit_weight_relative_fit:
					break;
				}

				if (yerrorVector.size() > 0) {
					switch (fitData.yWeightsType) { // y-error types: s_y^2 = 1/w_y
					case nsl_fit_weight_no:
						break;
					case nsl_fit_weight_direct: // yerror = w_y
						sigmasq += 1. / std::max(yerror[i], minError);
						break;
					case nsl_fit_weight_instrumental: // yerror = s_y
						sigmasq += yerror[i] * yerror[i];
						break;
					case nsl_fit_weight_inverse: // yerror = 1/w_y
						sigmasq += yerror[i];
						break;
					case nsl_fit_weight_statistical: // unused
					case nsl_fit_weight_relative:
						break;
					case nsl_fit_weight_statistical_fit: // s_y^2 = 1/w_y = Y_i
						sigmasq += fun[i];
						break;
					case nsl_fit_weight_relative_fit: // s_y^2 = 1/w_y = Y_i^2
						sigmasq += fun[i] * fun[i];
						break;
					}
				}

				// printf ("sigma[%d] = %g\n", i, sqrt(sigmasq));
				weight[i] = 1. / std::max(sigmasq, minError);
			}

			// update weights
			gsl_multifit_fdfsolver_set(s, &f, &x.vector);

			do { // fit
				iter++;
				writeSolverState(s);
				status = gsl_multifit_fdfsolver_iterate(s);
				// printf ("status = %s\n", gsl_strerror (status));
				if (nf == np) // stop if all parameters fix
					break;

				if (status) {
					DEBUG("		iter " << iter << ", status = " << gsl_strerror(status));
					if (status == GSL_ETOLX) // change in the position vector falls below machine precision: no progress
						status = GSL_SUCCESS;
					break;
				}
				status = gsl_multifit_test_delta(s->dx, s->x, delta, delta);
			} while (status == GSL_CONTINUE && iter < maxIters);

			chi = gsl_blas_dnrm2(s->f);
		} while (iter2 < maxIters && fabs(chi - chiOld) > fitData.eps);

		delete[] fun;
	}

	delete[] weight;

	// unscale start parameter
	for (auto i = 0; i < np; i++)
		x_init[i] = nsl_fit_map_bound(x_init[i], x_min[i], x_max[i]);

	// get the covariance matrix
	// TODO: scale the Jacobian when limits are used before constructing the covar matrix?
	auto* covar = gsl_matrix_alloc(np, np);
#if GSL_MAJOR_VERSION >= 2
	// the Jacobian is not part of the solver anymore
	auto* J = gsl_matrix_alloc(s->fdf->n, s->fdf->p);
	gsl_multifit_fdfsolver_jac(s, J);
	gsl_multifit_covar(J, 0.0, covar);
#else
	gsl_multifit_covar(s->J, 0.0, covar);
#endif

	// write the result
	fitResult.available = true;
	fitResult.valid = true;
	fitResult.status = gslErrorToString(status);
	fitResult.iterations = iter;
	fitResult.dof = n - (np - nf); // samples - (parameter - fixed parameter)

	// gsl_blas_dnrm2() - computes the Euclidian norm (||r||_2 = \sqrt {\sum r_i^2}) of the vector with the elements weight[i]*(Yi - y[i])
	// gsl_blas_dasum() - computes the absolute sum \sum |r_i| of the elements of the vector with the elements weight[i]*(Yi - y[i])
	fitResult.sse = gsl_pow_2(gsl_blas_dnrm2(s->f));
	fitResult.mae = gsl_blas_dasum(s->f) / n;

	// SST needed for coefficient of determination, R-squared and F test
	fitResult.sst = gsl_stats_tss(ydata, 1, n);
	// for a linear model without intercept R-squared is calculated differently
	// see
	// https://cran.r-project.org/doc/FAQ/R-FAQ.html#Why-does-summary_0028_0029-report-strange-results-for-the-R_005e2-estimate-when-I-fit-a-linear-model-with-no-intercept_003f
	if (fitData.modelCategory == nsl_fit_model_basic && fitData.modelType == nsl_fit_model_polynomial && fitData.degree == 1 && x_init[0] == 0) {
		DEBUG("	Using alternative R^2 for linear model without intercept");
		fitResult.sst = gsl_stats_tss_m(ydata, 1, n, 0);
	}
	if (fitResult.sst < fitResult.sse) {
		DEBUG("	Using alternative R^2 since R^2 would be negative (probably custom model without intercept)");
		fitResult.sst = gsl_stats_tss_m(ydata, 1, n, 0);
	}
	fitResult.calculateResult(n, np);

	// parameter values
	fitResult.paramValues.resize(np);
	fitResult.errorValues.resize(np);
	fitResult.tdist_tValues.resize(np);
	fitResult.tdist_pValues.resize(np);
	fitResult.marginValues.resize(np);
	// GSL: cerr = GSL_MAX_DBL(1., sqrt(fitResult.rms)); // increase error for poor fit
	// NIST: cerr = sqrt(fitResult.rms); // increase error for poor fit, decrease for good fit
	const double cerr = sqrt(fitResult.rms);
	// CI = 100 * (1 - alpha)
	const double alpha = 1.0 - fitData.confidenceInterval / 100.;
	for (auto i = 0; i < np; i++) {
		// scale resulting values if they are bounded
		fitResult.paramValues[i] = nsl_fit_map_bound(gsl_vector_get(s->x, i), x_min[i], x_max[i]);
		// use results as start values if desired
		if (fitData.useResults) {
			fitData.paramStartValues.data()[i] = fitResult.paramValues.at(i);
			DEBUG("	saving parameter " << i << ": " << fitResult.paramValues[i] << ' ' << fitData.paramStartValues.data()[i]);
		}
		fitResult.errorValues[i] = cerr * sqrt(gsl_matrix_get(covar, i, i));
		fitResult.tdist_tValues[i] = nsl_stats_tdist_t(fitResult.paramValues.at(i), fitResult.errorValues.at(i));
		fitResult.tdist_pValues[i] = nsl_stats_tdist_p(fitResult.tdist_tValues.at(i), fitResult.dof);
		fitResult.marginValues[i] = nsl_stats_tdist_margin(alpha, fitResult.dof, fitResult.errorValues.at(i));
		for (auto j = 0; j <= i; j++)
			fitResult.correlationMatrix << gsl_matrix_get(covar, i, j) / sqrt(gsl_matrix_get(covar, i, i)) / sqrt(gsl_matrix_get(covar, j, j));
	}

	// residuals for selected range
	if (!fitData.autoRange) {
		size_t j = 0;
		for (int i = 0; i < tmpXDataColumn->rowCount(); i++) {
			if (xRange.contains(tmpXDataColumn->valueAt(i)))
				residualsVector->data()[i] = -gsl_vector_get(s->f, j++);
			else // outside range
				residualsVector->data()[i] = 0;
		}
	}

	gsl_multifit_fdfsolver_free(s);
	gsl_matrix_free(covar);
}

/* evaluate fit function (preview == true: use start values, default: false) */
bool XYFitCurvePrivate::evaluate(bool preview) {
	DEBUG(Q_FUNC_INFO << ", preview = " << preview);

	// prepare source data columns
	DEBUG(Q_FUNC_INFO << ", data source: " << ENUM_TO_STRING(XYAnalysisCurve, DataSourceType, dataSourceType))
	const AbstractColumn* tmpXDataColumn = nullptr;
	switch (dataSourceType) {
	case XYAnalysisCurve::DataSourceType::Spreadsheet:
		tmpXDataColumn = xDataColumn;
		break;
	case XYAnalysisCurve::DataSourceType::Curve:
		if (dataSourceCurve)
			tmpXDataColumn = dataSourceCurve->xColumn();
		break;
	case XYAnalysisCurve::DataSourceType::Histogram:
		if (dataSourceHistogram)
			tmpXDataColumn = dataSourceHistogram->bins();
	}

	if (!tmpXDataColumn) {
		DEBUG(Q_FUNC_INFO << ", ERROR: Preparing source data column failed!");
		return true;
	}

	// only needed for preview (else we have all columns)
	//  should not harm even if not in preview now that residuals are not cleared
	if (preview)
		prepareResultColumns();

	if (!xVector || !yVector) {
		DEBUG(Q_FUNC_INFO << ", xVector or yVector not defined!");
		return true;
	}

	if (fitData.model.simplified().isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", no fit-model specified.");
		return true;
	}

	auto* parser = ExpressionParser::getInstance();
	Range<double> xRange{tmpXDataColumn->minimum(), tmpXDataColumn->maximum()}; // full data range
	if (!fitData.autoEvalRange) { // use custom range for evaluation
		if (!fitData.evalRange.isZero()) // avoid zero range
			xRange = fitData.evalRange;
	}
	DEBUG(Q_FUNC_INFO << ", eval range = " << xRange.toStdString());

	int nrPoints = (int)fitData.evaluatedPoints;
	xVector->resize(nrPoints);
	yVector->resize(nrPoints);
	DEBUG(Q_FUNC_INFO << ", vector size = " << xVector->size());

	if (fitResult.paramValues.size() == 0) { // fit result not initialized yet
		fitResult.paramValues = fitData.paramStartValues;
		const int np = fitData.paramStartValues.size();
		fitResult.errorValues.resize(np);
		fitResult.tdist_tValues.resize(np);
		fitResult.tdist_pValues.resize(np);
		fitResult.marginValues.resize(np);
	}

	auto paramValues = fitResult.paramValues;
	if (preview) // results not available yet
		paramValues = fitData.paramStartValues;

	bool valid = parser->tryEvaluateCartesian(fitData.model, xRange, nrPoints, xVector, yVector, fitData.paramNames, paramValues);

	if (!valid) {
		DEBUG(Q_FUNC_INFO << ", ERROR: Parsing fit function failed")
		xVector->clear();
		yVector->clear();
		residualsVector->clear();
	}

	if (!preview)
		updateResultsNote();

	return true;
}

/*!
 * writes out the current state of the solver \c s
 */
void XYFitCurvePrivate::writeSolverState(gsl_multifit_fdfsolver* s, double chi) {
	QString state;

	// current parameter values, semicolon separated
	double* min = fitData.paramLowerLimits.data();
	double* max = fitData.paramUpperLimits.data();
	for (int i = 0; i < fitData.paramNames.size(); ++i) {
		const double x = gsl_vector_get(s->x, i);
		// map parameter if bounded
		state += QString::number(nsl_fit_map_bound(x, min[i], max[i])) + QStringLiteral("\t");
	}

	// current value of chi
	if (std::isnan(chi))
		chi = gsl_blas_dnrm2(s->f);
	state += QString::number(chi * chi);
	state += QStringLiteral(";");
	DEBUG(Q_FUNC_INFO << ", chi^2 = " << chi * chi);

	fitResult.solverOutput += state;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYFitCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFitCurve);

	writer->writeStartElement(QStringLiteral("xyFitCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	// write xy-fit-curve specific information

	// fit data - only save model expression and parameter names for custom model, otherwise they are set in XYFitCurve::initFitData()
	writer->writeStartElement(QStringLiteral("fitData"));
	WRITE_COLUMN(d->xErrorColumn, xErrorColumn);
	WRITE_COLUMN(d->yErrorColumn, yErrorColumn);
	WRITE_PATH(d->dataSourceHistogram, dataSourceHistogram);
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->fitData.autoRange));
	writer->writeAttribute(QStringLiteral("fitRangeMin"), QString::number(d->fitData.fitRange.start(), 'g', 15));
	writer->writeAttribute(QStringLiteral("fitRangeMax"), QString::number(d->fitData.fitRange.end(), 'g', 15));
	writer->writeAttribute(QStringLiteral("modelCategory"), QString::number(d->fitData.modelCategory));
	writer->writeAttribute(QStringLiteral("modelType"), QString::number(d->fitData.modelType));
	writer->writeAttribute(QStringLiteral("xWeightsType"), QString::number(d->fitData.xWeightsType));
	writer->writeAttribute(QStringLiteral("weightsType"), QString::number(d->fitData.yWeightsType));
	writer->writeAttribute(QStringLiteral("degree"), QString::number(d->fitData.degree));
	if (d->fitData.modelCategory == nsl_fit_model_custom)
		writer->writeAttribute(QStringLiteral("model"), d->fitData.model);
	writer->writeAttribute(QStringLiteral("algorithm"), QString::number(d->fitData.algorithm));
	writer->writeAttribute(QStringLiteral("maxIterations"), QString::number(d->fitData.maxIterations));
	writer->writeAttribute(QStringLiteral("eps"), QString::number(d->fitData.eps, 'g', 15));
	writer->writeAttribute(QStringLiteral("evaluatedPoints"), QString::number(d->fitData.evaluatedPoints));
	writer->writeAttribute(QStringLiteral("autoEvalRange"), QString::number(d->fitData.autoEvalRange));
	writer->writeAttribute(QStringLiteral("evalRangeMin"), QString::number(d->fitData.evalRange.start(), 'g', 15));
	writer->writeAttribute(QStringLiteral("evalRangeMax"), QString::number(d->fitData.evalRange.end(), 'g', 15));
	writer->writeAttribute(QStringLiteral("useDataErrors"), QString::number(d->fitData.useDataErrors));
	writer->writeAttribute(QStringLiteral("useResults"), QString::number(d->fitData.useResults));
	writer->writeAttribute(QStringLiteral("previewEnabled"), QString::number(d->fitData.previewEnabled));
	writer->writeAttribute(QStringLiteral("confidenceInterval"), QString::number(d->fitData.confidenceInterval));

	if (d->fitData.modelCategory == nsl_fit_model_custom) {
		writer->writeStartElement(QStringLiteral("paramNames"));
		for (const QString& name : d->fitData.paramNames)
			writer->writeTextElement(QStringLiteral("name"), name);
		writer->writeEndElement();
	}

	writer->writeStartElement(QStringLiteral("paramStartValues"));
	for (const double& value : d->fitData.paramStartValues)
		writer->writeTextElement(QStringLiteral("startValue"), QString::number(value, 'g', 15));
	writer->writeEndElement();

	// use 16 digits to handle -DBL_MAX
	writer->writeStartElement(QStringLiteral("paramLowerLimits"));
	for (const double& limit : d->fitData.paramLowerLimits)
		writer->writeTextElement(QStringLiteral("lowerLimit"), QString::number(limit, 'g', 16));
	writer->writeEndElement();

	// use 16 digits to handle DBL_MAX
	writer->writeStartElement(QStringLiteral("paramUpperLimits"));
	for (const double& limit : d->fitData.paramUpperLimits)
		writer->writeTextElement(QStringLiteral("upperLimit"), QString::number(limit, 'g', 16));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("paramFixed"));
	for (const bool& fixed : d->fitData.paramFixed)
		writer->writeTextElement(QStringLiteral("fixed"), QString::number(fixed));
	writer->writeEndElement();

	writer->writeEndElement(); //"fitData"

	// fit results (generated columns and goodness of the fit)
	writer->writeStartElement(QStringLiteral("fitResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->fitResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->fitResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->fitResult.status);
	writer->writeAttribute(QStringLiteral("iterations"), QString::number(d->fitResult.iterations));
	writer->writeAttribute(QStringLiteral("time"), QString::number(d->fitResult.elapsedTime));
	writer->writeAttribute(QStringLiteral("dof"), QString::number(d->fitResult.dof));
	writer->writeAttribute(QStringLiteral("sse"), QString::number(d->fitResult.sse, 'g', 15));
	writer->writeAttribute(QStringLiteral("sst"), QString::number(d->fitResult.sst, 'g', 15));
	writer->writeAttribute(QStringLiteral("rms"), QString::number(d->fitResult.rms, 'g', 15));
	writer->writeAttribute(QStringLiteral("rsd"), QString::number(d->fitResult.rsd, 'g', 15));
	writer->writeAttribute(QStringLiteral("mse"), QString::number(d->fitResult.mse, 'g', 15));
	writer->writeAttribute(QStringLiteral("rmse"), QString::number(d->fitResult.rmse, 'g', 15));
	writer->writeAttribute(QStringLiteral("mae"), QString::number(d->fitResult.mae, 'g', 15));
	writer->writeAttribute(QStringLiteral("rsquare"), QString::number(d->fitResult.rsquare, 'g', 15));
	writer->writeAttribute(QStringLiteral("rsquareAdj"), QString::number(d->fitResult.rsquareAdj, 'g', 15));
	writer->writeAttribute(QStringLiteral("chisq_p"), QString::number(d->fitResult.chisq_p, 'g', 15));
	writer->writeAttribute(QStringLiteral("fdist_F"), QString::number(d->fitResult.fdist_F, 'g', 15));
	writer->writeAttribute(QStringLiteral("fdist_p"), QString::number(d->fitResult.fdist_p, 'g', 15));
	writer->writeAttribute(QStringLiteral("aic"), QString::number(d->fitResult.aic, 'g', 15));
	writer->writeAttribute(QStringLiteral("bic"), QString::number(d->fitResult.bic, 'g', 15));
	writer->writeAttribute(QStringLiteral("solverOutput"), d->fitResult.solverOutput);

	writer->writeStartElement(QStringLiteral("paramValues"));
	for (const double& value : d->fitResult.paramValues)
		writer->writeTextElement(QStringLiteral("value"), QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("errorValues"));
	for (const double& value : d->fitResult.errorValues)
		writer->writeTextElement(QStringLiteral("error"), QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("tdist_tValues"));
	for (const double& value : d->fitResult.tdist_tValues)
		writer->writeTextElement(QStringLiteral("tdist_t"), QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("tdist_pValues"));
	for (const double& value : d->fitResult.tdist_pValues)
		writer->writeTextElement(QStringLiteral("tdist_p"), QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("tdist_marginValues"));
	for (const double& value : d->fitResult.marginValues)
		writer->writeTextElement(QStringLiteral("tdist_margin"), QString::number(value, 'g', 15));
	writer->writeEndElement();
	writer->writeStartElement(QStringLiteral("tdist_margin2Values"));
	for (const double& value : d->fitResult.margin2Values)
		writer->writeTextElement(QStringLiteral("tdist_margin2"), QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("correlationMatrix"));
	for (const double& value : d->fitResult.correlationMatrix)
		writer->writeTextElement(QStringLiteral("correlation"), QString::number(value, 'g', 15));
	writer->writeEndElement();

	// save calculated columns if available
	if (saveCalculations() && d->xColumn && d->yColumn && d->residualsColumn) {
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

	QXmlStreamAttributes attribs;
	QString str, model;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyFitCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (!preview && reader->name() == QLatin1String("fitData")) {
			attribs = reader->attributes();

			READ_COLUMN(xErrorColumn);
			READ_COLUMN(yErrorColumn);
			READ_PATH(dataSourceHistogram);

			READ_INT_VALUE("autoRange", fitData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", fitData.fitRange.start()); // old name
			READ_DOUBLE_VALUE("xRangeMax", fitData.fitRange.end()); // old name
			READ_DOUBLE_VALUE("fitRangeMin", fitData.fitRange.start());
			READ_DOUBLE_VALUE("fitRangeMax", fitData.fitRange.end());
			READ_INT_VALUE("modelCategory", fitData.modelCategory, nsl_fit_model_category);
			READ_INT_VALUE("modelType", fitData.modelType, int);
			READ_INT_VALUE("xWeightsType", fitData.xWeightsType, nsl_fit_weight_type);
			READ_INT_VALUE("weightsType", fitData.yWeightsType, nsl_fit_weight_type);
			READ_INT_VALUE("degree", fitData.degree, int);
			// older projects have custom models with category == type == 0! So read the model
			READ_STRING_VALUE("model", fitData.model);
			model = d->fitData.model;
			DEBUG("got model = " << STDSTRING(model));
			READ_INT_VALUE("algorithm", fitData.algorithm, nsl_fit_algorithm);

			READ_INT_VALUE("maxIterations", fitData.maxIterations, int);
			READ_DOUBLE_VALUE("eps", fitData.eps);
			READ_INT_VALUE("fittedPoints", fitData.evaluatedPoints, size_t); // old name
			READ_INT_VALUE("evaluatedPoints", fitData.evaluatedPoints, size_t);
			READ_INT_VALUE("evaluateFullRange", fitData.autoEvalRange, bool); // old name
			READ_INT_VALUE("autoEvalRange", fitData.autoEvalRange, bool);
			READ_DOUBLE_VALUE("evalRangeMin", fitData.evalRange.start());
			READ_DOUBLE_VALUE("evalRangeMax", fitData.evalRange.end());
			READ_INT_VALUE("useDataErrors", fitData.useDataErrors, bool);
			READ_INT_VALUE("useResults", fitData.useResults, bool);
			READ_INT_VALUE("previewEnabled", fitData.previewEnabled, bool);
			READ_DOUBLE_VALUE("confidenceInterval", fitData.confidenceInterval);

		} else if (!preview && reader->name() == QLatin1String("paramNames")) { // needed for custom model
			d->fitData.paramNames.clear();
		} else if (!preview && reader->name() == QLatin1String("name")) {
			d->fitData.paramNames << reader->readElementText();
		} else if (!preview && reader->name() == QLatin1String("paramStartValues")) {
			d->fitData.paramStartValues.clear();
		} else if (!preview && reader->name() == QLatin1String("startValue")) {
			d->fitData.paramStartValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == QLatin1String("paramLowerLimits")) {
			d->fitData.paramLowerLimits.clear();
		} else if (!preview && reader->name() == QLatin1String("lowerLimit")) {
			bool ok;
			double x = reader->readElementText().toDouble(&ok);
			if (ok) // -DBL_MAX results in conversion error
				d->fitData.paramLowerLimits << x;
			else
				d->fitData.paramLowerLimits << -std::numeric_limits<double>::max();
		} else if (!preview && reader->name() == QLatin1String("paramUpperLimits")) {
			d->fitData.paramUpperLimits.clear();
		} else if (!preview && reader->name() == QLatin1String("upperLimit")) {
			bool ok;
			double x = reader->readElementText().toDouble(&ok);
			if (ok) // DBL_MAX results in conversion error
				d->fitData.paramUpperLimits << x;
			else
				d->fitData.paramUpperLimits << std::numeric_limits<double>::max();
		} else if (!preview && reader->name() == QLatin1String("paramFixed")) {
			d->fitData.paramFixed.clear();
		} else if (!preview && reader->name() == QLatin1String("fixed")) {
			d->fitData.paramFixed << (bool)reader->readElementText().toInt();
			// end fitData
		} else if (!preview && reader->name() == QLatin1String("paramValues")) {
			d->fitResult.paramValues.clear();
		} else if (!preview && reader->name() == QLatin1String("value")) {
			d->fitResult.paramValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == QLatin1String("errorValues")) {
			d->fitResult.errorValues.clear();
		} else if (!preview && reader->name() == QLatin1String("error")) {
			d->fitResult.errorValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == QLatin1String("tdist_tValues")) {
			d->fitResult.tdist_tValues.clear();
		} else if (!preview && reader->name() == QLatin1String("tdist_t")) {
			d->fitResult.tdist_tValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == QLatin1String("tdist_pValues")) {
			d->fitResult.tdist_pValues.clear();
		} else if (!preview && reader->name() == QLatin1String("tdist_p")) {
			d->fitResult.tdist_pValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == QLatin1String("tdist_marginValues")) {
			d->fitResult.marginValues.clear();
		} else if (!preview && reader->name() == QLatin1String("tdist_margin")) {
			d->fitResult.marginValues << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == QLatin1String("tdist_margin2Values")) {
			d->fitResult.margin2Values.clear();
		} else if (!preview && reader->name() == QLatin1String("tdist_margin2")) {
			d->fitResult.margin2Values << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == QLatin1String("correlationMatrix")) {
			d->fitResult.correlationMatrix.clear();
		} else if (!preview && reader->name() == QLatin1String("correlation")) {
			d->fitResult.correlationMatrix << reader->readElementText().toDouble();
		} else if (!preview && reader->name() == QLatin1String("fitResult")) {
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
		} else if (reader->name() == QLatin1String("column")) {
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Double);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			DEBUG(Q_FUNC_INFO << ", reading column " << STDSTRING(column->name()))
			if (column->name() == QLatin1String("x"))
				d->xColumn = column;
			else if (column->name() == QLatin1String("y"))
				d->yColumn = column;
			else if (column->name() == QLatin1String("Residuals") || column->name() == QLatin1String("residuals"))
				d->residualsColumn = column;
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// set the model expression and the parameter names (can be derived from the saved values for category, type and degree)
	XYFitCurve::initFitData(d->fitData);

	// add result note (not saved in projects)
	d->resultsNote = new Note(i18nc("Curve Fitting", "Fit Results"));
	d->resultsNote->setFixed(true); // visible in the project explorer but cannot be modified (renamed, deleted, etc.)
	addChild(d->resultsNote);

	////////////////////////////// fix old projects /////////////////////////

	// reset model type of old projects due to new model style
	if (d->fitData.modelCategory == nsl_fit_model_basic && d->fitData.modelType >= NSL_FIT_MODEL_BASIC_COUNT) {
		DEBUG(Q_FUNC_INFO << ", RESET old fit model");
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
		d->fitResult.marginValues.resize(2);
		d->fitResult.margin2Values.resize(2);
	}

	// older projects also save the param names for non-custom models: remove them
	while (d->fitData.paramNames.size() > d->fitData.paramStartValues.size())
		d->fitData.paramNames.removeLast();

	// Utf8 names missing in old projects
	if (d->fitData.paramNamesUtf8.isEmpty())
		d->fitData.paramNamesUtf8 << d->fitData.paramNames;

	// if we have more paramNames than the saved model type, we have a custom model
	if (d->fitData.paramNamesUtf8.size() < d->fitData.paramNames.size()) {
		d->fitData.modelCategory = nsl_fit_model_custom;
		d->fitData.model = std::move(model);
		d->fitData.paramNamesUtf8 = d->fitData.paramNames;
	}

	// not present in old projects
	int np = d->fitResult.paramValues.size();
	if (d->fitResult.tdist_tValues.size() == 0)
		d->fitResult.tdist_tValues.resize(np);
	if (d->fitResult.tdist_pValues.size() == 0)
		d->fitResult.tdist_pValues.resize(np);
	if (d->fitResult.marginValues.size() == 0)
		d->fitResult.marginValues.resize(np);
	if (d->fitResult.margin2Values.size() == 0)
		d->fitResult.margin2Values.resize(np);
	if (d->fitResult.correlationMatrix.size() == 0)
		d->fitResult.correlationMatrix.resize(np * (np + 1) / 2);

	///////////////////////////////////////////////////////////////////////////

	// Loading done. Check some parameter
	DEBUG(Q_FUNC_INFO << ", model category = " << d->fitData.modelCategory);
	DEBUG(Q_FUNC_INFO << ", model type = " << d->fitData.modelType);
	DEBUG(Q_FUNC_INFO << ", # params = " << d->fitData.paramNames.size());
	DEBUG(Q_FUNC_INFO << ", # start values = " << d->fitData.paramStartValues.size());
	// for (const auto& value : d->fitData.paramStartValues)
	//	DEBUG(Q_FUNC_INFO << ", start value = " << value);

	// fill results note now all values are loaded
	d->updateResultsNote();

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

		d->xVector = static_cast<QVector<double>*>(d->xColumn->data());
		d->yVector = static_cast<QVector<double>*>(d->yColumn->data());
		d->residualsVector = static_cast<QVector<double>*>(d->residualsColumn->data());

		static_cast<XYCurvePrivate*>(d_ptr)->xColumn = d->xColumn;
		static_cast<XYCurvePrivate*>(d_ptr)->yColumn = d->yColumn;

		recalc();
	}

	return true;
}
