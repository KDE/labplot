/*
    File                 : PlotArea.h
    Project              : LabPlot
    Description          : Plot area (for background filling and clipping).
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2015 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef PLOTAREA_H
#define PLOTAREA_H

#include "backend/worksheet/WorksheetElement.h"
#include "backend/lib/macros.h"

class CartesianPlot;
class PlotAreaPrivate;

class PlotArea : public WorksheetElement {
	Q_OBJECT

public:
	explicit PlotArea(const QString& name, CartesianPlot *parent);
	~PlotArea() override;

	enum class BackgroundType {Color, Image, Pattern};
	enum class BackgroundColorStyle {SingleColor, HorizontalLinearGradient, VerticalLinearGradient,
			TopLeftDiagonalLinearGradient, BottomLeftDiagonalLinearGradient, RadialGradient};
	enum class BackgroundImageStyle {ScaledCropped, Scaled, ScaledAspectRatio, Centered, Tiled, CenterTiled};
	enum class BorderTypeFlags {
			NoBorder = 0x0,
			BorderLeft = 0x1,
			BorderTop = 0x2,
			BorderRight = 0x4,
			BorderBottom = 0x8
	};
	Q_DECLARE_FLAGS(BorderType, BorderTypeFlags)

	QGraphicsItem* graphicsItem() const override;
	void setVisible(bool on) override;
	bool isVisible() const override;
	void loadThemeConfig(const KConfig& config) override;
	void saveThemeConfig(const KConfig& config) override;
	bool isHovered() const;
	bool isSelected() const;

	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
	CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)
	BASIC_D_ACCESSOR_DECL(qreal, backgroundOpacity, BackgroundOpacity)

	BASIC_D_ACCESSOR_DECL(PlotArea::BorderType, borderType, BorderType)
	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(qreal, borderCornerRadius, BorderCornerRadius)
	BASIC_D_ACCESSOR_DECL(qreal, borderOpacity, BorderOpacity)

	BASIC_D_ACCESSOR_DECL(bool, clippingEnabled, ClippingEnabled)
	CLASS_D_ACCESSOR_DECL(QRectF, rect, Rect)

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef PlotAreaPrivate Private;

protected:
	PlotArea(const QString& name, CartesianPlot *parent, PlotAreaPrivate* dd);
	PlotAreaPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(PlotArea)
	void init();

signals:
	void backgroundTypeChanged(PlotArea::BackgroundType);
	void backgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
	void backgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
	void backgroundBrushStyleChanged(Qt::BrushStyle);
	void backgroundFirstColorChanged(QColor&);
	void backgroundSecondColorChanged(QColor&);
	void backgroundFileNameChanged(QString&);
	void backgroundOpacityChanged(float);
	void borderTypeChanged(PlotArea::BorderType);
	void borderPenChanged(QPen&);
	void borderCornerRadiusChanged(float);
	void borderOpacityChanged(float);

private:
	CartesianPlot* m_parent;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlotArea::BorderType)

#endif
