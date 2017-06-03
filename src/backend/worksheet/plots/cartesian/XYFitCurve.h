/***************************************************************************
    File                 : XYFitCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a fit model
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2016 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "backend/worksheet/plots/cartesian/XYCurve.h"
extern "C" {
#include "backend/nsl/nsl_fit.h"
}

class XYFitCurvePrivate;
class XYFitCurve : public XYCurve {
	Q_OBJECT

	public:
		struct FitData {
			FitData() : modelCategory(nsl_fit_model_basic), modelType(0),
						weightsType(nsl_fit_weight_instrumental),
						degree(1),
						maxIterations(500),
						eps(1e-4),
						evaluatedPoints(100),
						useResults(true),
						evaluateFullRange(true),
						autoRange(true), xRange(2) {};

			nsl_fit_model_category modelCategory;
			unsigned int modelType;
			nsl_fit_weight_type weightsType;
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
			bool useResults;		// use results as new start values
			bool evaluateFullRange;		// evaluate fit function on full data range

			bool autoRange;			// use all data?
			QVector<double> xRange;		// x range for integration
		};

		struct FitResult {
			FitResult() : available(false), valid(false), iterations(0), elapsedTime(0),
				dof(0), sse(0), rms(0), rsd(0), mse(0), rmse(0), mae(0) {};

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
			// see also http://www.originlab.com/doc/Origin-Help/NLFit-Algorithm
			QVector<double> paramValues;
			QVector<double> errorValues;
			QString solverOutput;
		};

		explicit XYFitCurve(const QString& name);
		virtual ~XYFitCurve();

		void recalculate();
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorColumn, XErrorColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorColumn, YErrorColumn)
		const QString& xDataColumnPath() const;
		const QString& yDataColumnPath() const;
		const QString& xErrorColumnPath() const;
		const QString& yErrorColumnPath() const;

		CLASS_D_ACCESSOR_DECL(FitData, fitData, FitData)
		const FitResult& fitResult() const;

		typedef WorksheetElement BaseClass;
		typedef XYFitCurvePrivate Private;

	protected:
		XYFitCurve(const QString& name, XYFitCurvePrivate* dd);

	private:
		Q_DECLARE_PRIVATE(XYFitCurve)
		void init();

	signals:
		friend class XYFitCurveSetXDataColumnCmd;
		friend class XYFitCurveSetYDataColumnCmd;
		friend class XYFitCurveSetXErrorColumnCmd;
		friend class XYFitCurveSetYErrorColumnCmd;
		void xDataColumnChanged(const AbstractColumn*);
		void yDataColumnChanged(const AbstractColumn*);
		void xErrorColumnChanged(const AbstractColumn*);
		void yErrorColumnChanged(const AbstractColumn*);

		friend class XYFitCurveSetFitDataCmd;
		void fitDataChanged(const XYFitCurve::FitData&);
};

#endif
