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
class QActionGroup;

class ReferenceRange : public WorksheetElement {
	Q_OBJECT

public:
	explicit ReferenceRange(CartesianPlot*, const QString&);
	~ReferenceRange() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	// position and orientation
	BASIC_D_ACCESSOR_DECL(QPointF, positionLogicalStart, PositionLogicalStart)
	BASIC_D_ACCESSOR_DECL(QPointF, positionLogicalEnd, PositionLogicalEnd)
	BASIC_D_ACCESSOR_DECL(Orientation, orientation, Orientation)

	// background
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundType, backgroundType, BackgroundType)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
	CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)
	BASIC_D_ACCESSOR_DECL(qreal, backgroundOpacity, BackgroundOpacity)

	// border
	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(qreal, borderOpacity, BorderOpacity)

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef ReferenceRangePrivate Private;

protected:
	ReferenceRange(const QString& name, ReferenceRangePrivate* dd);

private:
	Q_DECLARE_PRIVATE(ReferenceRange)
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

private Q_SLOTS:
	// SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);
	void lineStyleChanged(QAction*);
	void lineColorChanged(QAction*);
	void visibilityChangedSlot();

Q_SIGNALS:
	friend class ReferenceRangeSetPositionCmd;
	void positionLogicalStartChanged(QPointF);
	void positionLogicalEndChanged(QPointF);
	void orientationChanged(Orientation);

	void backgroundTypeChanged(WorksheetElement::BackgroundType);
	void backgroundColorStyleChanged(WorksheetElement::BackgroundColorStyle);
	void backgroundImageStyleChanged(WorksheetElement::BackgroundImageStyle);
	void backgroundBrushStyleChanged(Qt::BrushStyle);
	void backgroundFirstColorChanged(QColor&);
	void backgroundSecondColorChanged(QColor&);
	void backgroundFileNameChanged(QString&);
	void backgroundOpacityChanged(float);

	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);
};

#endif
