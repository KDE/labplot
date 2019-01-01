/***************************************************************************
    File                 : XYFourierFilterCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier filter
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

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

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
extern "C" {
#include "backend/nsl/nsl_filter.h"
}

class XYFourierFilterCurvePrivate;

class XYFourierFilterCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	struct FilterData {
		FilterData() : xRange(2) {};

		nsl_filter_type type{nsl_filter_type_low_pass};
		nsl_filter_form form{nsl_filter_form_ideal};
		int order{1};
		double cutoff{0.0};		// (low) cutoff
		nsl_filter_cutoff_unit unit{nsl_filter_cutoff_unit_frequency};	// (low) value unit
		double cutoff2{0.0};			// high cutoff
		nsl_filter_cutoff_unit unit2{nsl_filter_cutoff_unit_frequency};	// high value unit
		bool autoRange{true};			// use all data?
		QVector<double> xRange;		// x range for integration
	};
	struct FilterResult {
		FilterResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
	};

	explicit XYFourierFilterCurve(const QString& name);
	~XYFourierFilterCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(FilterData, filterData, FilterData)
	const FilterResult& filterResult() const;

	typedef XYFourierFilterCurvePrivate Private;

protected:
	XYFourierFilterCurve(const QString& name, XYFourierFilterCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYFourierFilterCurve)

signals:
	void filterDataChanged(const XYFourierFilterCurve::FilterData&);
};

#endif
