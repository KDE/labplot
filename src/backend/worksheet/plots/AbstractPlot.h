/*
	File                 : AbstractPlot.h
	Project              : LabPlot
	Description          : Base class for plots of different types
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2011-2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTPLOT_H
#define ABSTRACTPLOT_H

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElementContainer.h"

class AbstractCoordinateSystem;
class TextLabel;
class AbstractPlotPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT AbstractPlot : public WorksheetElementContainer {
#else
class AbstractPlot : public WorksheetElementContainer {
#endif
	Q_OBJECT

public:
	AbstractPlot(const QString& name, AspectType type);
	~AbstractPlot() override = default;

	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	AbstractCoordinateSystem* coordinateSystem(int index) const;
	QVector<AbstractCoordinateSystem*> coordinateSystems() const;
	TextLabel* title();

	BASIC_D_ACCESSOR_DECL(double, horizontalPadding, HorizontalPadding)
	BASIC_D_ACCESSOR_DECL(double, verticalPadding, VerticalPadding)
	BASIC_D_ACCESSOR_DECL(double, rightPadding, RightPadding)
	BASIC_D_ACCESSOR_DECL(double, bottomPadding, BottomPadding)
	BASIC_D_ACCESSOR_DECL(bool, symmetricPadding, SymmetricPadding)

	typedef AbstractPlotPrivate Private;

Q_SIGNALS:
	void horizontalPaddingChanged(double);
	void verticalPaddingChanged(double);
	void rightPaddingChanged(double);
	void bottomPaddingChanged(double);
	void symmetricPaddingChanged(bool);

protected:
	AbstractPlot(const QString&, AbstractPlotPrivate*, AspectType);
	QVector<AbstractCoordinateSystem*> m_coordinateSystems;
	TextLabel* m_title{nullptr};

private:
	void init();
	Q_DECLARE_PRIVATE(AbstractPlot)
};

#endif
