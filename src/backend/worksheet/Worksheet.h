/***************************************************************************
    File                 : Worksheet.h
    Project              : LabPlot
    Description          : Worksheet (2D visualization) part
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2011-2016 by Alexander Semke (alexander.semke@web.de)
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

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/lib/macros.h"

class QGraphicsItem;
class QGraphicsScene;
class QRectF;

class WorksheetPrivate;

class Worksheet: public AbstractPart, public scripted {
	Q_OBJECT

	public:
		Worksheet(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
		~Worksheet();

		enum Unit {Millimeter, Centimeter, Inch, Point};
		enum Layout {NoLayout, VerticalLayout, HorizontalLayout, GridLayout};
		static float convertToSceneUnits(const float value, const Worksheet::Unit unit);
		static float convertFromSceneUnits(const float value, const Worksheet::Unit unit);

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QWidget* view() const;

		virtual bool exportView() const;
		virtual bool printView();
		virtual bool printPreview() const;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);
		void loadTheme(KConfig& config);

		QRectF pageRect() const;
		void setPageRect(const QRectF&);
		QGraphicsScene* scene() const;
		void update();
		void setPrinting(bool) const;
		void setThemeName(const QString&);

		void setItemSelectedInView(const QGraphicsItem* item, const bool b);
		void setSelectedInView(const bool);
		void deleteAspectFromGraphicsItem(const QGraphicsItem*);
		void setIsClosing();

		BASIC_D_ACCESSOR_DECL(float, backgroundOpacity, BackgroundOpacity)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
		BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
		CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
		CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
		CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)

		BASIC_D_ACCESSOR_DECL(bool, scaleContent, ScaleContent)
		BASIC_D_ACCESSOR_DECL(bool, useViewSize, UseViewSize)
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

public slots:
		void loadTheme(const QString&);

	private:
		void init();
		WorksheetElement* aspectFromGraphicsItem(const WorksheetElement*, const QGraphicsItem*) const;

		WorksheetPrivate* const d;
		friend class WorksheetPrivate;

	 private slots:
		void handleAspectAdded(const AbstractAspect*);
		void handleAspectAboutToBeRemoved(const AbstractAspect*);
		void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);

		virtual void childSelected(const AbstractAspect*);
		virtual void childDeselected(const AbstractAspect*);

	 signals:
		void requestProjectContextMenu(QMenu*);
		void itemSelected(QGraphicsItem*);
		void itemDeselected(QGraphicsItem*);
		void requestUpdate();
		void useViewSizeRequested();

		friend class WorksheetSetBackgroundTypeCmd;
		friend class WorksheetSetBackgroundColorStyleCmd;
		friend class WorksheetSetBackgroundImageStyleCmd;
		friend class WorksheetSetBackgroundBrushStyleCmd;
		friend class WorksheetSetBackgroundFirstColorCmd;
		friend class WorksheetSetBackgroundSecondColorCmd;
		friend class WorksheetSetBackgroundFileNameCmd;
		friend class WorksheetSetBackgroundOpacityCmd;
		friend class WorksheetSetScaleContentCmd;
		friend class WorksheetSetPageRectCmd;
		friend class WorksheetSetLayoutCmd;
		friend class WorksheetSetLayoutTopMarginCmd;
		friend class WorksheetSetLayoutBottomMarginCmd;
		friend class WorksheetSetLayoutLeftMarginCmd;
		friend class WorksheetSetLayoutRightMarginCmd;
		friend class WorksheetSetLayoutVerticalSpacingCmd;
		friend class WorksheetSetLayoutHorizontalSpacingCmd;
		friend class WorksheetSetLayoutRowCountCmd;
		friend class WorksheetSetLayoutColumnCountCmd;
		void backgroundTypeChanged(PlotArea::BackgroundType);
		void backgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
		void backgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
		void backgroundBrushStyleChanged(Qt::BrushStyle);
		void backgroundFirstColorChanged(const QColor&);
		void backgroundSecondColorChanged(const QColor&);
		void backgroundFileNameChanged(const QString&);
		void backgroundOpacityChanged(float);
		void scaleContentChanged(bool);
		void pageRectChanged(const QRectF&);
		void layoutChanged(Worksheet::Layout);
		void layoutTopMarginChanged(float);
		void layoutBottomMarginChanged(float);
		void layoutLeftMarginChanged(float);
		void layoutRightMarginChanged(float);
		void layoutVerticalSpacingChanged(float);
		void layoutHorizontalSpacingChanged(float);
		void layoutRowCountChanged(int);
		void layoutColumnCountChanged(int);
};

#endif
