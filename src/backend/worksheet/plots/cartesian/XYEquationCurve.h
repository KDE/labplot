/*
    File                 : XYEquationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a mathematical equation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYEQUATIONCURVE_H
#define XYEQUATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class XYEquationCurvePrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYEquationCurve : public XYCurve {
#else
class XYEquationCurve : public XYCurve {
#endif
	Q_OBJECT

public:
	enum class EquationType {Cartesian, Polar, Parametric, Implicit, Neutral};

	struct EquationData {
		EquationData() : min(QLatin1String("0")), max(QLatin1String("1")) {};

		EquationType type{EquationType::Cartesian};
		QString expression1;
		QString expression2;
		QString min;
		QString max;
		int count{1000};
	};

	explicit XYEquationCurve(const QString& name);
	~XYEquationCurve() override;

	void recalculate();
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(EquationData, equationData, EquationData)

	typedef XYEquationCurvePrivate Private;

protected:
	XYEquationCurve(const QString& name, XYEquationCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYEquationCurve)
	void init();

Q_SIGNALS:
	void equationDataChanged(const XYEquationCurve::EquationData&);
};

#endif
