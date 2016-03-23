/***************************************************************************
    File                 : XYFourierFilterCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier filter
    --------------------------------------------------------------------
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

#ifndef XYFOURIERFILTERCURVE_H
#define XYFOURIERFILTERCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class XYFourierFilterCurvePrivate;
class XYFourierFilterCurve: public XYCurve {
	Q_OBJECT

	public:
		//enum ModelType {Polynomial, Power, Exponential, Inverse_Exponential, Fourier, Gaussian, Lorentz, Maxwell, Custom};
		//enum WeightsType {WeightsFromColumn, WeightsFromErrorColumn};

		/*struct FitData {
			FitData() : modelType(Polynomial),
						weightsType(XYFitCurve::WeightsFromColumn),
						degree(1),
						maxIterations(500),
						eps(1e-4),
						fittedPoints(100) {};

			ModelType modelType;
			WeightsType weightsType;
			int degree;
			QString model;
			QStringList paramNames;
			QVector<double> paramStartValues;

			int maxIterations;
			double eps;
			int fittedPoints;
		};

		struct FitResult {
			FitResult() : available(false), valid(false), iterations(0), elapsedTime(0), dof(0), sse(0), mse(0), rmse(0), mae(0), rms(0), rsd(0), rsquared(0), rsquaredAdj(0) {};

			bool available;
			bool valid;
			QString status;
			int iterations;
			qint64 elapsedTime;
			double dof; //degrees of freedom
			double sse; //sum of squared errors (SSE) / residual sum of errors (RSS) / sum of sq. residuals (SSR) = \sum_i^n (Y_i-y_i)^2
			double mse; //mean squared error = 1/n \sum_i^n  (Y_i-y_i)^2
			double rmse; //root-mean squared error = \sqrt(mse)
			double mae; //mean absolute error = \sum_i^n |Y_i-y_i|
			double rms; //residual mean square = SSE/d.o.f.
			double rsd; //residual standard deviation = sqrt(SSE/d.o.f)
			double rsquared; //Coefficient of determination (R^2)
			double rsquaredAdj; //Adjusted coefficient of determination (R^2)
			QVector<double> paramValues;
			QVector<double> errorValues;
			QString solverOutput;
		};*/

		explicit XYFourierFilterCurve(const QString& name);
		virtual ~XYFourierFilterCurve();

//		void recalculate();
		virtual QIcon icon() const;
/*		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);
*/
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
/*		POINTER_D_ACCESSOR_DECL(const AbstractColumn, weightsColumn, WeightsColumn)
		const QString& xDataColumnPath() const;
		const QString& yDataColumnPath() const;
		const QString& weightsColumnPath() const;

		CLASS_D_ACCESSOR_DECL(FitData, fitData, FitData)
		const FitResult& fitResult() const;
		bool isSourceDataChangedSinceLastFit() const;
*/
		typedef WorksheetElement BaseClass;
		typedef XYFourierFilterCurvePrivate Private;

	protected:
		XYFourierFilterCurve(const QString& name, XYFourierFilterCurvePrivate* dd);

	private:
		Q_DECLARE_PRIVATE(XYFourierFilterCurve)
		void init();

	private slots:
		void handleSourceDataChanged();

	signals:
		friend class XYFourierFilterCurveSetXDataColumnCmd;
		friend class XYFourierFilterCurveSetYDataColumnCmd;
//		friend class XYFitCurveSetWeightsColumnCmd;
		void xDataColumnChanged(const AbstractColumn*);
		void yDataColumnChanged(const AbstractColumn*);
//		void weightsColumnChanged(const AbstractColumn*);

//		friend class XYFitCurveSetFitDataCmd;
//		void fitDataChanged(const XYFitCurve::FitData&);
		void sourceDataChangedSinceLastFilter();
};

#endif
