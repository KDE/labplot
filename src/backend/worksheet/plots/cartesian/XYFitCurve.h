/***************************************************************************
    File                 : XYFitCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a fit model
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016-2018 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef XYFITCURVE_H
#define XYFITCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h" //for PlotDataDialog::AnalysisAction. TODO: find a better place for this enum.

extern "C" {
#include "backend/nsl/nsl_fit.h"
}

class XYFitCurvePrivate;

class XYFitCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	struct FitData {
		FitData() : modelCategory(nsl_fit_model_basic), modelType(0),
				xWeightsType(nsl_fit_weight_no),
				yWeightsType(nsl_fit_weight_no),
				degree(1),
				maxIterations(500),
				eps(1.e-4),
				evaluatedPoints(1000),
				useDataErrors(true),
				useResults(true),
				previewEnabled(false),
				autoRange(true),
				autoEvalRange(true),
				fitRange(2),
				evalRange(2) {};

		nsl_fit_model_category modelCategory;
		int modelType;
		nsl_fit_weight_type xWeightsType;
		nsl_fit_weight_type yWeightsType;
		int degree;
		QString model;
		QStringList paramNames;
		QStringList paramNamesUtf8;	// Utf8 version of paramNames
		QVector<double> paramStartValues;
		QVector<double> paramLowerLimits;
		QVector<double> paramUpperLimits;
		QVector<bool> paramFixed;

		int maxIterations;
		double eps;
		size_t evaluatedPoints;
		bool useDataErrors;		// use given data errors when fitting (default)
		bool useResults;		// use results as new start values (default)
		bool previewEnabled;		// preview fit function with given start parameters

		bool autoRange;			// use all data points? (default)
		bool autoEvalRange;		// evaluate fit function on full data range (default)
		QVector<double> fitRange;	// x fit range
		QVector<double> evalRange;	// x evaluation range
	};

	struct FitResult {
		FitResult() : available(false), valid(false), iterations(0), elapsedTime(0),
			dof(0), sse(0), sst(0), rms(0), rsd(0), mse(0), rmse(0), mae(0), rsquare(0), rsquareAdj(0),
			chisq_p(0), fdist_F(0), fdist_p(0), logLik(0), aic(0), bic(0) {};

		bool available;
		bool valid;
		QString status;
		int iterations;
		qint64 elapsedTime;
		double dof; //degrees of freedom
		// residuals: r_i = y_i - Y_i
		double sse; // sum of squared errors (SSE) / residual sum of squares (RSS) / sum of sq. residuals (SSR) / S = chi^2 = \sum_i^n r_i^2
		double sst; // total sum of squares (SST) = \sum_i^n (y_i - <y>)^2
		double rms; // residual mean square / reduced chi^2 = SSE/dof
		double rsd; // residual standard deviation = sqrt(SSE/dof)
		double mse; // mean squared error = SSE/n
		double rmse; // root-mean squared error = \sqrt(mse)
		double mae; // mean absolute error = \sum_i^n |r_i|
		double rsquare;
		double rsquareAdj;
		double chisq_p;	// chi^2 distribution p-value
		double fdist_F;	// F distribution F-value
		double fdist_p;	// F distribution p-value
		double logLik;	// log likelihood
		double aic;	// Akaike information criterion
		double bic;	// Schwarz Bayesian information criterion
		// see also http://www.originlab.com/doc/Origin-Help/NLFit-Algorithm
		QVector<double> paramValues;
		QVector<double> errorValues;
		QVector<double> tdist_tValues;
		QVector<double> tdist_pValues;
		QVector<double> tdist_marginValues;
		QString solverOutput;
	};

	explicit XYFitCurve(const QString& name);
	~XYFitCurve() override;

	void recalculate() override;
	void evaluate(bool preview);
	void initFitData(PlotDataDialog::AnalysisAction);
	static void initFitData(XYFitCurve::FitData&);
	void initStartValues(const XYCurve*);
	static void initStartValues(XYFitCurve::FitData&, const XYCurve*);

	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorColumn, XErrorColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorColumn, YErrorColumn)
	const QString& xErrorColumnPath() const;
	const QString& yErrorColumnPath() const;

	CLASS_D_ACCESSOR_DECL(FitData, fitData, FitData)
	const FitResult& fitResult() const;

	typedef XYFitCurvePrivate Private;

protected:
	XYFitCurve(const QString& name, XYFitCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYFitCurve)

signals:
	void xErrorColumnChanged(const AbstractColumn*);
	void yErrorColumnChanged(const AbstractColumn*);
	void fitDataChanged(const XYFitCurve::FitData&);
};

#endif
