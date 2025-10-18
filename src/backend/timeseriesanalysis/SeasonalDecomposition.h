/*
	File                 : SeasonalDecomposition.h
	Project              : LabPlot
	Description          : Seasonal Decomposition of Time Series
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEASONALDECOMPOSITION_H
#define SEASONALDECOMPOSITION_H

#include "backend/core/AbstractPart.h"
#include "backend/lib/macros.h"

class AbstractColumn;
class SeasonalDecompositionPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT SeasonalDecomposition : public AbstractPart {
#else
class SeasonalDecomposition : public AbstractPart {
#endif
	Q_OBJECT

public:
	explicit SeasonalDecomposition(const QString& name, const bool loading = false);
	~SeasonalDecomposition() override;

	enum class Method { STL, MSTL };

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yColumnPath, YColumnPath)
	BASIC_D_ACCESSOR_DECL(Method, method, Method)

	// STL parameters
	BASIC_D_ACCESSOR_DECL(int, stlPeriod, STLPeriod)
	BASIC_D_ACCESSOR_DECL(bool, stlRobust, STLRobust)
	BASIC_D_ACCESSOR_DECL(int, stlSeasonalLength, STLSeasonalLength)
	BASIC_D_ACCESSOR_DECL(int, stlTrendLength, STLTrendLength)
	BASIC_D_ACCESSOR_DECL(bool, stlTrendLengthAuto, STLTrendLengthAuto)
	BASIC_D_ACCESSOR_DECL(int, stlLowPassLength, STLLowPassLength)
	BASIC_D_ACCESSOR_DECL(bool, stlLowPassLengthAuto, STLLowPassLengthAuto)
	BASIC_D_ACCESSOR_DECL(int, stlSeasonalDegree, STLSeasonalDegree)
	BASIC_D_ACCESSOR_DECL(int, stlTrendDegree, STLTrendDegree)
	BASIC_D_ACCESSOR_DECL(int, stlLowPassDegree, STLLowPassDegree)
	BASIC_D_ACCESSOR_DECL(int, stlSeasonalJump, STLSeasonalJump)
	BASIC_D_ACCESSOR_DECL(int, stlTrendJump, STLTrendJump)
	BASIC_D_ACCESSOR_DECL(int, stlLowPassJump, STLLowPassJump)

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void recalc();

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	typedef SeasonalDecompositionPrivate Private;

private:
	Q_DECLARE_PRIVATE(SeasonalDecomposition)
	SeasonalDecompositionPrivate* const d_ptr;
	friend class SeasonalDecompositionPrivate;

	void init();
	void connectXColumn(const AbstractColumn*);
	void connectYColumn(const AbstractColumn*);

private Q_SLOTS:
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void methodChanged(SeasonalDecomposition::Method);
	void xDataChanged();
	void yDataChanged();

	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);

	// STL parameters signals
	void stlPeriodChanged(int);
	void stlRobustChanged(bool);
	void stlSeasonalLengthChanged(int);
	void stlTrendLengthChanged(int);
	void stlTrendLengthAutoChanged(bool);
	void stlLowPassLengthChanged(int);
	void stlLowPassLengthAutoChanged(bool);
	void stlSeasonalDegreeChanged(int);
	void stlTrendDegreeChanged(int);
	void stlLowPassDegreeChanged(int);
	void stlSeasonalJumpChanged(int);
	void stlTrendJumpChanged(int);
	void stlLowPassJumpChanged(int);

	friend class SeasonalDecompositionSetXColumnCmd;
	friend class SeasonalDecompositionSetYColumnCmd;
};

#endif
