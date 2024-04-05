/*
	File                 : ReferenceLine.h
	Project              : LabPlot
	Description          : Reference line on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REFERENCELINE_H
#define REFERENCELINE_H

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class CartesianPlot;
class Line;
class ReferenceLinePrivate;
class QActionGroup;

class ReferenceLine : public WorksheetElement {
	Q_OBJECT

public:
	explicit ReferenceLine(CartesianPlot*, const QString&, bool loading = false);
	~ReferenceLine() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	BASIC_D_ACCESSOR_DECL(Orientation, orientation, Orientation)
	Line* line() const;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef ReferenceLinePrivate Private;

protected:
	ReferenceLine(const QString& name, ReferenceLinePrivate* dd);

private:
	Q_DECLARE_PRIVATE(ReferenceLine)
	void init(bool loading = false);
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

Q_SIGNALS:
	friend class ReferenceLineSetPositionCmd;
	void orientationChanged(Orientation);

	friend class WorksheetElementTest;
};

#endif
