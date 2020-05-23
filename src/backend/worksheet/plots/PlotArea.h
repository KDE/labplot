/***************************************************************************
    File                 : PlotArea.h
    Project              : LabPlot
    Description          : Plot area (for background filling and clipping).
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2015 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
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
	enum BackgroundColorStyle {SingleColor, HorizontalLinearGradient, VerticalLinearGradient,
			TopLeftDiagonalLinearGradient, BottomLeftDiagonalLinearGradient, RadialGradient};
	enum BackgroundImageStyle {ScaledCropped, Scaled, ScaledAspectRatio, Centered, Tiled, CenterTiled};

	QGraphicsItem* graphicsItem() const override;
	void setVisible(bool on) override;
	bool isVisible() const override;
	void setPrinting(bool) override {};
	void loadThemeConfig(const KConfig& config) override;
	void saveThemeConfig(const KConfig& config) override;
	bool isHovered() const;
	bool isPrinted() const;
	bool isSelected() const;

	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
	CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)
	BASIC_D_ACCESSOR_DECL(qreal, backgroundOpacity, BackgroundOpacity)

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
	void borderPenChanged(QPen&);
	void borderCornerRadiusChanged(float);
	void borderOpacityChanged(float);

private:
	CartesianPlot* m_parent;
};

#endif
