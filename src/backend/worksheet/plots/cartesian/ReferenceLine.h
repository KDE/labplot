/*
    File                 : ReferenceLine.h
    Project              : LabPlot
    Description          : Reference line on the plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke (alexander.semke@web.de)
*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef REFERENCELINE_H
#define REFERENCELINE_H

#include <QPen>

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class ReferenceLinePrivate;
class CartesianPlot;
class QActionGroup;

class ReferenceLine : public WorksheetElement {
	Q_OBJECT

public:
	explicit ReferenceLine(CartesianPlot*, const QString&);
	~ReferenceLine() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	BASIC_D_ACCESSOR_DECL(double, position, Position)
	BASIC_D_ACCESSOR_DECL(Orientation, orientation, Orientation)
	CLASS_D_ACCESSOR_DECL(QPen, pen, Pen)
	BASIC_D_ACCESSOR_DECL(qreal, opacity, Opacity)

	void setVisible(bool on) override;
	bool isVisible() const override;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef ReferenceLinePrivate Private;

protected:
	ReferenceLinePrivate* const d_ptr;
	ReferenceLine(const QString& name, ReferenceLinePrivate* dd);

private:
	Q_DECLARE_PRIVATE(ReferenceLine)
	void init();
	void initActions();
	void initMenus();

	QAction* visibilityAction{nullptr};
	QAction* orientationHorizontalAction{nullptr};
	QAction* orientationVerticalAction{nullptr};

	QActionGroup* lineStyleActionGroup{nullptr};
	QActionGroup* lineColorActionGroup{nullptr};

	QMenu* orientationMenu{nullptr};
	QMenu* lineMenu{nullptr};
	QMenu* lineStyleMenu{nullptr};
	QMenu* lineColorMenu{nullptr};

private slots:
	//SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);
	void lineStyleChanged(QAction*);
	void lineColorChanged(QAction*);
	void visibilityChangedSlot();

signals:
	friend class ReferenceLineSetPositionCmd;
	void orientationChanged(Orientation);
	void positionChanged(double);
	void penChanged(const QPen&);
	void opacityChanged(qreal);
	void visibleChanged(bool);
};

#endif
