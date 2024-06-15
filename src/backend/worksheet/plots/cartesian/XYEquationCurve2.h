/*
	File                 : XYEquationCurve2.h
	Project              : LabPlot
	Description          : A xy-curve defined by a mathematical equation from other curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYEQUATIONCURVE2_H
#define XYEQUATIONCURVE2_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

struct ConstLatin1String : public QLatin1String {
	constexpr ConstLatin1String(const char* const s)
		: QLatin1String(s, static_cast<int>(std::char_traits<char>::length(s))) {
	}
};

class XYEquationCurve2Private;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYEquationCurve2 : public XYCurve {
#else
class XYEquationCurve2 : public XYCurve {
#endif
	Q_OBJECT
	Q_ENUMS(EquationType)

public:
	enum class EquationType { Cartesian, Polar, Parametric, Implicit, Neutral };

	struct EquationData {
		EquationData()
			: min(QLatin1String("0"))
			, max(QLatin1String("1")){};

		EquationType type{EquationType::Cartesian};
		QString expression1; // Expression for Cartesian, Polar, ...
		QString expression2; // Second expression for Parametric
		QString min; // localized strings to support expressions
		QString max;
		int count{1000}; // number of points of the curve
	};

	explicit XYEquationCurve2(const QString& name);
	~XYEquationCurve2() override;

	void recalculate();
	bool dataAvailable() const;
	QIcon icon() const override;
	inline static constexpr ConstLatin1String saveName = "xyEquationCurve2";
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(EquationData, equationData, EquationData)

	typedef XYEquationCurve2Private Private;

protected:
	XYEquationCurve2(const QString& name, XYEquationCurve2Private* dd);

private:
	Q_DECLARE_PRIVATE(XYEquationCurve2)
	void init();

public Q_SLOTS:
	void createDataSpreadsheet();

Q_SIGNALS:
	void equationDataChanged(const XYEquationCurve2::EquationData&);
};

#endif
