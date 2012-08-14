/***************************************************************************
    File                 : Worksheet.h
    Project              : LabPlot/SciDAVis
    Description          : Worksheet (2D visualization) part
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
	Copyright            : (C) 2011-2012 by Alexander Semke (alexander.semke*web.de)
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

#ifndef WORKSHEET_H
#define WORKSHEET_H

#include "core/AbstractPart.h"
#include "core/AbstractScriptingEngine.h"
#include "worksheet/WorksheetModel.h"
#include "worksheet/plots/PlotArea.h"
#include "lib/macros.h"

class QGraphicsItem;
class QRectF;

class WorksheetPrivate;

class Worksheet: public AbstractPart, public scripted{
	Q_OBJECT

	public:
		Worksheet(AbstractScriptingEngine *engine, const QString &name);
		~Worksheet();

		enum Unit {Millimeter, Centimeter, Inch, Point};
		enum Layout {NoLayout, VerticalLayout, HorizontalLayout, GridLayout};
		static float convertToSceneUnits(const float value, const Worksheet::Unit unit);
		static float convertFromSceneUnits(const float value, const Worksheet::Unit unit);

		virtual QIcon icon() const;
		virtual bool fillProjectMenu(QMenu *menu);
		virtual QMenu *createContextMenu();
		virtual QWidget *view() const;

		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

		QRectF pageRect() const;
		void setPageRect(const QRectF &rect, const bool scaleContent=false);
		QGraphicsScene *scene() const;
		void update();

		void setItemSelectedInView(const QGraphicsItem* item, const bool b);
		void setSelectedInView(const bool);
				
        BASIC_D_ACCESSOR_DECL(qreal, backgroundOpacity, BackgroundOpacity)
        BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
        BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
        BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
        BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
        CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
        CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
        CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)
		
        BASIC_D_ACCESSOR_DECL(Worksheet::Layout, layout, Layout)
        BASIC_D_ACCESSOR_DECL(float, layoutTopMargin, LayoutTopMargin)
        BASIC_D_ACCESSOR_DECL(float, layoutBottomMargin, LayoutBottomMargin)
        BASIC_D_ACCESSOR_DECL(float, layoutLeftMargin, LayoutLeftMargin)
        BASIC_D_ACCESSOR_DECL(float, layoutRightMargin, LayoutRightMargin)
        BASIC_D_ACCESSOR_DECL(float, layoutHorizontalSpacing, LayoutHorizontalSpacing)
        BASIC_D_ACCESSOR_DECL(float, layoutVerticalSpacing, LayoutVerticalSpacing)
        BASIC_D_ACCESSOR_DECL(int, layoutRowCount, LayoutRowCount)
        BASIC_D_ACCESSOR_DECL(int, layoutColumnCount, LayoutColumnCount)

		typedef WorksheetPrivate Private;

	private:
		void init();
		AbstractWorksheetElement* aspectFromGraphicsItem(const AbstractWorksheetElement*, const QGraphicsItem*) const;
		WorksheetPrivate* const d;
		friend class WorksheetPrivate;

	 private slots:
		void handleAspectAdded(const AbstractAspect*);
		void handleAspectAboutToBeRemoved(const AbstractAspect*);
		void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
		void childSelected();
		void childDeselected();

	 signals:
		void requestProjectContextMenu(QMenu *menu);
		void itemSelected(QGraphicsItem*);
		void itemDeselected(QGraphicsItem*);
		void requestUpdate();
		void layoutChanged(Worksheet::Layout);
		void layoutRowCountChanged(int);
};

#endif
