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

	enum class Method { LOESS };

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yColumnPath, YColumnPath)
	BASIC_D_ACCESSOR_DECL(Method, method, Method)

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

	friend class SeasonalDecompositionSetXColumnCmd;
	friend class SeasonalDecompositionSetYColumnCmd;
};

#endif
