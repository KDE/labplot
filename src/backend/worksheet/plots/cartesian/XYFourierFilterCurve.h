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
extern "C" {
#include "backend/nsl/nsl_filter.h"
}

class XYFourierFilterCurvePrivate;
class XYFourierFilterCurve: public XYCurve {
	Q_OBJECT

	public:
		struct FilterData {
			FilterData() : type(nsl_filter_type_low_pass), form(nsl_filter_form_ideal), order(1),
				cutoff(0), unit(nsl_filter_cutoff_unit_frequency), cutoff2(0), unit2(nsl_filter_cutoff_unit_frequency) {};

			nsl_filter_type type;
			nsl_filter_form form;
			unsigned int order;
			double cutoff;			// (low) cutoff
			nsl_filter_cutoff_unit unit;	// (low) value unit
			double cutoff2;			// high cutoff
			nsl_filter_cutoff_unit unit2;	// high value unit
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
