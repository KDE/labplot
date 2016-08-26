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

class PlotAreaPrivate;

class PlotArea : public WorksheetElement{
	Q_OBJECT

	public:
		explicit PlotArea(const QString& name);
		virtual ~PlotArea();

		enum BackgroundType{Color, Image, Pattern};
		enum BackgroundColorStyle{SingleColor, HorizontalLinearGradient, VerticalLinearGradient,
								  TopLeftDiagonalLinearGradient, BottomLeftDiagonalLinearGradient,
								  RadialGradient};
		enum BackgroundImageStyle{ScaledCropped, Scaled, ScaledAspectRatio, Centered, Tiled, CenterTiled};

		virtual QGraphicsItem* graphicsItem() const;
		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual void setPrinting(bool) {};
		virtual void loadThemeConfig(const KConfig& config);
		virtual void saveThemeConfig(const KConfig& config);

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

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		typedef WorksheetElement BaseClass;
		typedef PlotAreaPrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	protected:
		PlotArea(const QString& name, PlotAreaPrivate* dd);
		PlotAreaPrivate* const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(PlotArea)
		void init();

	signals:
		friend class PlotAreaSetBackgroundTypeCmd;
		friend class PlotAreaSetBackgroundColorStyleCmd;
		friend class PlotAreaSetBackgroundImageStyleCmd;
		friend class PlotAreaSetBackgroundBrushStyleCmd;
		friend class PlotAreaSetBackgroundFirstColorCmd;
		friend class PlotAreaSetBackgroundSecondColorCmd;
		friend class PlotAreaSetBackgroundFileNameCmd;
		friend class PlotAreaSetBackgroundOpacityCmd;
		friend class PlotAreaSetBorderPenCmd;
		friend class PlotAreaSetBorderCornerRadiusCmd;
		friend class PlotAreaSetBorderOpacityCmd;
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
};

#endif
