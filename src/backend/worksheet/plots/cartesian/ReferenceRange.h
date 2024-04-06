/*
	File                 : ReferenceRange.h
	Project              : LabPlot
	Description          : Reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REFERENCERANGE_H
#define REFERENCERANGE_H

#include <QPen>

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class ReferenceRangePrivate;
class CartesianPlot;
class Background;
class Line;
class QActionGroup;

class ReferenceRange : public WorksheetElement {
	Q_OBJECT

public:
	explicit ReferenceRange(CartesianPlot*, const QString&, bool loading = false);
	~ReferenceRange() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	// position and orientation
	BASIC_D_ACCESSOR_DECL(QPointF, positionLogicalStart, PositionLogicalStart)
	BASIC_D_ACCESSOR_DECL(QPointF, positionLogicalEnd, PositionLogicalEnd)
	BASIC_D_ACCESSOR_DECL(Orientation, orientation, Orientation)

	// background and border
	Background* background() const;
	Line* line() const;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef ReferenceRangePrivate Private;

protected:
	ReferenceRange(const QString& name, ReferenceRangePrivate* dd);

private:
	Q_DECLARE_PRIVATE(ReferenceRange)
	void init(bool loading);
	void initActions();
	void initMenus();

	QAction* orientationHorizontalAction{nullptr};
	QAction* orientationVerticalAction{nullptr};

	QActionGroup* lineStyleActionGroup{nullptr};
	QActionGroup* lineColorActionGroup{nullptr};

	QMenu* orientationMenu{nullptr};
	QMenu* lineMenu{nullptr};
	QMenu* lineStyleMenu{nullptr};
	QMenu* lineColorMenu{nullptr};

private Q_SLOTS:
	// SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);
	void lineStyleChanged(QAction*);
	void lineColorChanged(QAction*);

	void updateStartEndPositions();

Q_SIGNALS:
	friend class ReferenceRangeSetPositionCmd;
	void positionLogicalStartChanged(QPointF);
	void positionLogicalEndChanged(QPointF);
	void orientationChanged(Orientation);

	friend class WorksheetElementTest;
};

#endif
