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

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

#include <QString>

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
class XYEquationCurve2 : public XYAnalysisCurve {
#endif
	Q_OBJECT
	Q_ENUMS(EquationType)

public:
	explicit XYEquationCurve2(const QString& name);
	// no need to delete the d-pointer here - it inherits from QGraphicsItem
	// and is deleted during the cleanup in QGraphicsScene
	virtual ~XYEquationCurve2() override = default;

	void recalculate() override;

	QIcon icon() const override;
	inline static constexpr ConstLatin1String saveName = "xyEquationCurve2";
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	// CLASS_D_ACCESSOR_DECL(Formu, equationData, EquationData);

	typedef XYEquationCurve2Private Private;

	const XYAnalysisCurve::Result& result() const override;

	// void setEquationData(const XYEquationCurve2::EquationData& equationData);
	void setEquation(const QString& equation, const QStringList& variableNames, const QVector<const XYCurve*>& curve);
	QString equation() const;
	void clearEquation();
	struct EquationData {
		// #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0)) // required to use in QVector
		// 		EquationData() = default;
		// #endif
		EquationData(const QString& variableName, const QString& curvePath)
			: m_variableName(variableName)
			, m_curvePath(curvePath) {
		}
		EquationData(const QString& variableName, const XYCurve* curve)
			: m_curve(curve)
			, m_variableName(variableName)
			, m_curvePath(curve->path()) {
		}
		QString curvePath() const {
			return (m_curve ? m_curve->path() : m_curvePath);
		}
		bool setCurvePath(const QString& path) {
			if (m_curve && m_curve->path() != path)
				return false;
			else if (!m_curve)
				m_curvePath = path;
			return true;
		}
		void setCurve(const XYCurve* c) {
			m_curve = c;
			if (c)
				m_curvePath = c->path(); // do not clear path
		}
		// column can be changed only with setColumn
		const XYCurve* curve() const {
			return m_curve;
		}
		const QString& variableName() const {
			return m_variableName;
		}

	private:
		// Should be only accessible by the columnName() function
		const XYCurve* m_curve{nullptr};
		QString m_variableName;
		QString m_curvePath;
		friend class XYEquationCurve2Private;
		friend class CurveSetGlobalEquationCmd;
	};
	const QVector<EquationData>& equationData() const;
	// void setEquationVariableCurve(XYCurve*);
	// void setEquationVariableCurvesPath(int index, const QString& path);
	// void setEquationVariableCurve(int index, XYCurve*);

protected:
	XYEquationCurve2(const QString& name, XYEquationCurve2Private* dd);
	virtual void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) override;

private Q_SLOTS:
	void equationVariableCurveRemoved(const AbstractAspect* aspect);
	void equationVariableCurveAdded(const AbstractAspect* aspect);

private:
	Q_DECLARE_PRIVATE(XYEquationCurve2)
	void init();
	bool XmlReadEquation(XmlStreamReader* reader);

public Q_SLOTS:

Q_SIGNALS:
	void equationDataChanged(const XYEquationCurve2::EquationData&);

	friend class CurveSetGlobalEquationCmd;
};

#endif
