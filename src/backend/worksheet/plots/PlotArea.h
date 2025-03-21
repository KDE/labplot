/*
	File                 : PlotArea.h
	Project              : LabPlot
	Description          : Plot area (for background filling and clipping).
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOTAREA_H
#define PLOTAREA_H

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class Background;
class CartesianPlot;
class Line;
class PlotAreaPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT PlotArea : public WorksheetElement {
#else
class PlotArea : public WorksheetElement {
#endif
	Q_OBJECT

public:
	explicit PlotArea(const QString& name, CartesianPlot* parent);
	~PlotArea() override;

	enum class BorderTypeFlags { NoBorder = 0x0, BorderLeft = 0x1, BorderTop = 0x2, BorderRight = 0x4, BorderBottom = 0x8 };
	Q_DECLARE_FLAGS(BorderType, BorderTypeFlags)

	QGraphicsItem* graphicsItem() const override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;
	bool isHovered() const;
	bool isSelected() const;

	Background* background() const;
	BASIC_D_ACCESSOR_DECL(PlotArea::BorderType, borderType, BorderType)
	Line* borderLine() const;
	BASIC_D_ACCESSOR_DECL(qreal, borderCornerRadius, BorderCornerRadius)

	BASIC_D_ACCESSOR_DECL(bool, clippingEnabled, ClippingEnabled)
	CLASS_D_ACCESSOR_DECL(QRectF, rect, Rect)

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef PlotAreaPrivate Private;

protected:
	PlotArea(const QString& name, CartesianPlot* parent, PlotAreaPrivate* dd);

private:
	Q_DECLARE_PRIVATE(PlotArea)
	void init();

Q_SIGNALS:
	void borderTypeChanged(PlotArea::BorderType);
	void borderCornerRadiusChanged(qreal);

private:
	CartesianPlot* m_parent;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlotArea::BorderType)

#endif
