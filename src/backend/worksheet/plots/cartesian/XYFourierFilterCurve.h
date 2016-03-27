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
		enum FilterType {LowPass, HighPass, BandPass, BandReject, Threshold};	//TODO
		enum FilterForm {Ideal, Butterworth, Chebyshev};	// TODO
		enum ValueUnit {Index, Hertz, Wavelength, Ratio};	// TODO

		struct FilterData {
			FilterData() : type(LowPass), form(Ideal), value(0), unit(Index), value2(0), unit2(Index) {};

			FilterType type;
			FilterForm form;
			double value;		// (low) value
			ValueUnit unit;		// (low) value unit
			double value2;		// high value
			ValueUnit unit2;	// high value unit
		};
		struct FilterResult {
			FilterResult() : available(false), valid(false), elapsedTime(0) {};

			bool available;
			bool valid;
			QString status;
			qint64 elapsedTime;
		};

		explicit XYFourierFilterCurve(const QString& name);
		virtual ~XYFourierFilterCurve();

		void recalculate();
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
		const QString& xDataColumnPath() const;
		const QString& yDataColumnPath() const;

		CLASS_D_ACCESSOR_DECL(FilterData, filterData, FilterData)
		const FilterResult& filterResult() const;
		bool isSourceDataChangedSinceLastFilter() const;

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
		void xDataColumnChanged(const AbstractColumn*);
		void yDataColumnChanged(const AbstractColumn*);

		friend class XYFourierFilterCurveSetFilterDataCmd;
		void filterDataChanged(const XYFourierFilterCurve::FilterData&);
		void sourceDataChangedSinceLastFilter();
};

#endif
