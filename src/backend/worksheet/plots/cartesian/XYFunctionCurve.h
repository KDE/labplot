/*
	File                 : XYFunctionCurve.h
	Project              : LabPlot
	Description          : A xy-curvethat is calculated as a function of other curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFUNCTIONCURVE_H
#define XYFUNCTIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

#include <QString>

struct ConstLatin1String : public QLatin1String {
	constexpr ConstLatin1String(const char* const s)
		: QLatin1String(s, static_cast<int>(std::char_traits<char>::length(s))) {
	}
};

class XYFunctionCurvePrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYFunctionCurve : public XYAnalysisCurve {
#else
class XYFunctionCurve : public XYAnalysisCurve {
#endif
	Q_OBJECT
	Q_ENUMS(FunctionType)

public:
	explicit XYFunctionCurve(const QString& name);
	// no need to delete the d-pointer here - it inherits from QGraphicsItem
	// and is deleted during the cleanup in QGraphicsScene
	virtual ~XYFunctionCurve() override = default;

	void recalculate() override;

	QIcon icon() const override;
	inline static constexpr ConstLatin1String saveName = "xyFunctionCurve";
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	// CLASS_D_ACCESSOR_DECL(Formu, functionData, FunctionData);

	typedef XYFunctionCurvePrivate Private;

	const XYAnalysisCurve::Result& result() const override;

	// void setFunctionData(const XYFunctionCurve::FunctionData& functionData);
	void setFunction(const QString& function, const QStringList& variableNames, const QVector<const XYCurve*>& curve);
	QString function() const;
	void clearFunction();
	struct FunctionData {
		// #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0)) // required to use in QVector
		// 		FunctionData() = default;
		// #endif
		FunctionData(const QString& variableName, const QString& curvePath)
			: m_variableName(variableName)
			, m_curvePath(curvePath) {
		}
		FunctionData(const QString& variableName, const XYCurve* curve)
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
		friend class XYFunctionCurvePrivate;
		friend class CurveSetGlobalFunctionCmd;
	};
	const QVector<FunctionData>& functionData() const;

protected:
	XYFunctionCurve(const QString& name, XYFunctionCurvePrivate* dd);
	virtual void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) override;

private Q_SLOTS:
	void functionVariableCurveRemoved(const AbstractAspect* aspect);
	void functionVariableCurveAdded(const AbstractAspect* aspect);

private:
	Q_DECLARE_PRIVATE(XYFunctionCurve)
	void init();
	void setFunctionVariableCurve(const XYCurve*);
	bool XmlReadFunction(XmlStreamReader* reader, bool preview);

public Q_SLOTS:

Q_SIGNALS:
	void functionDataChanged(const XYFunctionCurve::FunctionData&);

	friend class CurveSetGlobalFunctionCmd;
	friend class Project;
	friend class XYFunctionCurveTest;
};

#endif
