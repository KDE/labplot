/*
    File                 : XYFourierFilterCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
		FilterData() {};

		nsl_filter_type type{nsl_filter_type_low_pass};
		nsl_filter_form form{nsl_filter_form_ideal};
		int order{1};
		double cutoff{0.0};		// (low) cutoff
		nsl_filter_cutoff_unit unit{nsl_filter_cutoff_unit_frequency};	// (low) value unit
		double cutoff2{0.0};			// high cutoff
		nsl_filter_cutoff_unit unit2{nsl_filter_cutoff_unit_frequency};	// high value unit
		bool autoRange{true};			// use all data?
		//TODO: use Range
		QVector<double> xRange{0., 0.};		// x range for integration
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

Q_SIGNALS:
	void filterDataChanged(const XYFourierFilterCurve::FilterData&);
};

#endif
