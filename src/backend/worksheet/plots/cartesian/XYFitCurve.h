/***************************************************************************
    File                 : XYFitCurve.h
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

#ifndef XYFITCURVE_H
#define XYFITCURVE_H

#include "backend/lib/Range.h"
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
		FitData() {};

		nsl_fit_model_category modelCategory{nsl_fit_model_basic};
		int modelType{0};
		nsl_fit_weight_type xWeightsType{nsl_fit_weight_no};
		nsl_fit_weight_type yWeightsType{nsl_fit_weight_no};
		int degree{1};
		QString model;
		QStringList paramNames;
		QStringList paramNamesUtf8;	// Utf8 version of paramNames
		QVector<double> paramStartValues;
		QVector<double> paramLowerLimits;
		QVector<double> paramUpperLimits;
		QVector<bool> paramFixed;

		int maxIterations{500};
		double eps{1.e-4};
		size_t evaluatedPoints{1000};
		bool useDataErrors{true};	// use given data errors when fitting (default)
		bool useResults{true};		// use results as new start values (default)
		bool previewEnabled{true};	// preview fit function with given start parameters
		double confidenceInterval{95.};	// confidence interval for fit result

		bool autoRange{true};		// use all data points? (default)
		bool autoEvalRange{true};	// evaluate fit function on full data range (default)
		Range<double> fitRange{0., 0.};	// x fit range
		Range<double> evalRange{0., 0.};	// x evaluation range
	};

	struct FitResult {
		FitResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		int iterations{0};
		qint64 elapsedTime{0};
		double dof{0}; //degrees of freedom
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
		double chisq_p{0};	// chi^2 distribution p-value
		double fdist_F{0};	// F distribution F-value
		double fdist_p{0};	// F distribution p-value
		double logLik{0};	// log likelihood
		double aic{0};	// Akaike information criterion
		double bic{0};	// Schwarz Bayesian information criterion
		// see also https://www.originlab.com/doc/Origin-Help/NLFit-Theory
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
