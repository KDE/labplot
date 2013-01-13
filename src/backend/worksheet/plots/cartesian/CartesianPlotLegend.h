/***************************************************************************
    File                 : CartesianPlotLegend.h
    Project              : LabPlot/SciDAVis
    Description          : Legend for the cartesian plot
    --------------------------------------------------------------------
    Copyright            : (C) 2013 Alexander Semke (alexander.semke*web.de)
						       (replace * with @ in the email addresses)
                           
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

#ifndef CARTESIANPLOTLEGEND_H
#define CARTESIANPLOTLEGEND_H

#include "backend/worksheet/AbstractWorksheetElement.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/lib/macros.h"

class CartesianPlot;
class CartesianPlotLegendPrivate;

class CartesianPlotLegend: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		CartesianPlotLegend(CartesianPlot* parentPlot, const QString &name);
		virtual ~CartesianPlotLegend();

		virtual QIcon icon() const;
		virtual QGraphicsItem *graphicsItem() const;
		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

		virtual void setVisible(bool on);
		virtual bool isVisible() const;

        BASIC_D_ACCESSOR_DECL(float, backgroundOpacity, BackgroundOpacity)
        BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
        BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
        BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
        BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
        CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
        CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
        CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)

		CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
		BASIC_D_ACCESSOR_DECL(float, borderOpacity, BorderOpacity)

        BASIC_D_ACCESSOR_DECL(float, layoutTopMargin, LayoutTopMargin)
        BASIC_D_ACCESSOR_DECL(float, layoutBottomMargin, LayoutBottomMargin)
        BASIC_D_ACCESSOR_DECL(float, layoutLeftMargin, LayoutLeftMargin)
        BASIC_D_ACCESSOR_DECL(float, layoutRightMargin, LayoutRightMargin)
        BASIC_D_ACCESSOR_DECL(float, layoutHorizontalSpacing, LayoutHorizontalSpacing)
        BASIC_D_ACCESSOR_DECL(float, layoutVerticalSpacing, LayoutVerticalSpacing)
        BASIC_D_ACCESSOR_DECL(int, layoutColumnCount, LayoutColumnCount)

		typedef AbstractWorksheetElement BaseClass;
		typedef CartesianPlotLegendPrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	protected:
		CartesianPlotLegend(CartesianPlot*, const QString &name, CartesianPlotLegendPrivate* dd);
		CartesianPlotLegendPrivate* const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(CartesianPlotLegend)
		void init();
		CartesianPlot* m_plot;

	signals:
		friend class CartesianPlotLegendSetBackgroundTypeCmd;
		friend class CartesianPlotLegendSetBackgroundColorStyleCmd;
		friend class CartesianPlotLegendSetBackgroundImageStyleCmd;
		friend class CartesianPlotLegendSetBackgroundBrushStyleCmd;
		friend class CartesianPlotLegendSetBackgroundFirstColorCmd;
		friend class CartesianPlotLegendSetBackgroundSecondColorCmd;
		friend class CartesianPlotLegendSetBackgroundFileNameCmd;
		friend class CartesianPlotLegendSetBackgroundOpacityCmd;
		friend class CartesianPlotLegendSetBorderPenCmd;
		friend class CartesianPlotLegendSetBorderOpacityCmd;
		friend class CartesianPlotLegendSetLayoutTopMarginCmd;
		friend class CartesianPlotLegendSetLayoutBottomMarginCmd;
		friend class CartesianPlotLegendSetLayoutLeftMarginCmd;
		friend class CartesianPlotLegendSetLayoutRightMarginCmd;
		friend class CartesianPlotLegendSetLayoutVerticalSpacingCmd;
		friend class CartesianPlotLegendSetLayoutHorizontalSpacingCmd;
		friend class CartesianPlotLegendSetLayoutColumnCountCmd;
		void backgroundTypeChanged(PlotArea::BackgroundType);
		void backgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
		void backgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
		void backgroundBrushStyleChanged(Qt::BrushStyle);
		void backgroundFirstColorChanged(QColor&);
		void backgroundSecondColorChanged(QColor&);
		void backgroundFileNameChanged(QString&);
		void backgroundOpacityChanged(float);
		void borderPenChanged(QPen&);
		void borderOpacityChanged(float);
		void layoutTopMarginChanged(float);
		void layoutBottomMarginChanged(float);
		void layoutLeftMarginChanged(float);
		void layoutRightMarginChanged(float);
		void layoutVerticalSpacingChanged(float);
		void layoutHorizontalSpacingChanged(float);
		void layoutColumnCountChanged(int);

		void positionChanged(QPointF&);
};

#endif
