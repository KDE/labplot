/*
	File                 : XYFitCurve.h
	Project              : LabPlot
	Description          : A xy-curve defined by a fit model
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFITCURVE_H
#define XYFITCURVE_H

#include "backend/lib/Range.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_fit.h"
}

class XYFitCurvePrivate;
class Histogram;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYFitCurve : public XYAnalysisCurve {
#else
class XYFitCurve : public XYAnalysisCurve {
#endif
	Q_OBJECT

public:
	struct FitData {
		FitData() {
		}

		nsl_fit_model_category modelCategory{nsl_fit_model_basic};
		int modelType{0};
		nsl_fit_weight_type xWeightsType{nsl_fit_weight_no};
		nsl_fit_weight_type yWeightsType{nsl_fit_weight_no};
		int degree{1};
		QString model;
		QStringList paramNames;
		QStringList paramNamesUtf8; // Utf8 version of paramNames
		QVector<double> paramStartValues;
		QVector<double> paramLowerLimits;
		QVector<double> paramUpperLimits;
		QVector<bool> paramFixed;
		nsl_fit_algorithm algorithm{nsl_fit_algorithm_lm};

		int maxIterations{500};
		double eps{1.e-4};
		size_t evaluatedPoints{1000};
		bool useDataErrors{true}; // use given data errors when fitting (default)
		bool useResults{true}; // use results as new start values (default)
		bool previewEnabled{true}; // preview fit function with given start parameters
		double confidenceInterval{95.}; // confidence interval for fit result

		bool autoRange{true}; // use all data points? (default)
		bool autoEvalRange{true}; // evaluate fit function on full data range (default)
		Range<double> fitRange{0., 0.}; // x range of data to fit
		Range<double> evalRange{0., 0.}; // x range to evaluate fit function
	};

	struct FitResult : public XYAnalysisCurve::Result {
		FitResult() {
		}
		void calculateResult(size_t n, unsigned int np); // calculate depending results (uses dof, sse, sst)

		int iterations{0};
		double dof{0}; // degrees of freedom
		// residuals: r_i = y_i - Y_i
		double sse{0}; // sum of squared errors (SSE) / residual sum of squares (RSS) / sum of sq. residuals (SSR) / S = chi^2 = \sum_i^n r_i^2
		double sst{0}; // total sum of squares (SST) = \sum_i^n (y_i - <y>)^2
		double rms{0}; // residual mean square / reduced chi^2 = SSE/dof
		double rsd{0}; // residual standard deviation = sqrt(SSE/dof)
		double mse{0}; // mean squared error = SSE/n
		double rmse{0}; // root-mean squared error = \sqrt(mse)
		double mae{0}; // mean absolute error = \sum_i^n |r_i|
		double rsquare{0};
		double rsquareAdj{0};
		double chisq_p{0}; // chi^2 distribution p-value
		double fdist_F{0}; // F distribution F-value
		double fdist_p{0}; // F distribution p-value
		double logLik{0}; // log likelihood
		double aic{0}; // Akaike information criterion
		double bic{0}; // Schwarz Bayesian information criterion
		// see also https://www.originlab.com/doc/Origin-Help/NLFit-Theory
		QVector<double> paramValues;
		QVector<double> errorValues;
		QVector<double> tdist_tValues;
		QVector<double> tdist_pValues;
		QVector<double> marginValues; // lower confidence
		QVector<double> margin2Values; // upper confidence
		QVector<double> correlationMatrix;
		QString solverOutput;
	};

	explicit XYFitCurve(const QString& name);
	~XYFitCurve() override;

	POINTER_D_ACCESSOR_DECL(const Histogram, dataSourceHistogram, DataSourceHistogram)
	const QString& dataSourceHistogramPath() const;

	void evaluate(bool preview);
	virtual const XYAnalysisCurve::Result& result() const override;
	void initStartValues(const XYCurve*);
	void initStartValues(XYFitCurve::FitData&, const XYCurve*);
	void initFitData(XYAnalysisCurve::AnalysisAction);
	static void initFitData(XYFitCurve::FitData&);
	void clearFitResult();

	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	const AbstractColumn* residualsColumn() const;
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorColumn, XErrorColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorColumn, YErrorColumn)
	const QString& xErrorColumnPath() const;
	const QString& yErrorColumnPath() const;

	CLASS_D_ACCESSOR_DECL(FitData, fitData, FitData)
	const FitResult& fitResult() const;

	typedef XYFitCurvePrivate Private;

protected:
	XYFitCurve(const QString& name, XYFitCurvePrivate* dd);
	virtual void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) override;

private:
	Q_DECLARE_PRIVATE(XYFitCurve)

Q_SIGNALS:
	void dataSourceHistogramChanged(const Histogram*);
	void xErrorColumnChanged(const AbstractColumn*);
	void yErrorColumnChanged(const AbstractColumn*);
	void fitDataChanged(const XYFitCurve::FitData&);
};

#endif
