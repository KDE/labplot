/***************************************************************************
    File                 : WorksheetPrivate.h
    Project              : LabPlot
    Description          : Private members of Worksheet.
    --------------------------------------------------------------------
    Copyright            : (C) 2012 by Alexander Semke (alexander.semke@web.de)

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

#ifndef WORKSHEETPRIVATE_H
#define WORKSHEETPRIVATE_H

#include <backend/worksheet/Worksheet.h>

#include <QColor>

class QBrush;
class Worksheet;
class WorksheetElementContainer;
class QGraphicsScene;

class WorksheetPrivate {
public:
	explicit WorksheetPrivate(Worksheet*);
	virtual ~WorksheetPrivate();

	Worksheet* const q;
	QRectF pageRect;
	QGraphicsScene* m_scene;
	bool useViewSize{false};
	bool scaleContent{false};

	QString name() const;
	void update();
	void updateLayout(bool undoable = true);
	void setContainerRect(WorksheetElementContainer*, float x, float y, float h, float w, bool undoable);
	void updatePageRect();

	PlotArea::BackgroundType backgroundType;
	PlotArea::BackgroundColorStyle backgroundColorStyle;
	PlotArea::BackgroundImageStyle backgroundImageStyle;
	Qt::BrushStyle backgroundBrushStyle;
	QColor backgroundFirstColor;
	QColor backgroundSecondColor;
	QString backgroundFileName;
	float backgroundOpacity{1.0};

	Worksheet::Layout layout{Worksheet::VerticalLayout};
	bool suppressLayoutUpdate{false};
	float layoutTopMargin{0.0};
	float layoutBottomMargin{0.0};
	float layoutLeftMargin{0.0};
	float layoutRightMargin{0.0};
	float layoutVerticalSpacing{0.0};
	float layoutHorizontalSpacing{0.0};
	int layoutColumnCount{2};
	int layoutRowCount{2};
	QString theme;
	bool plotsLocked{false};
	Worksheet::CartesianPlotActionMode cartesianPlotActionMode{Worksheet::CartesianPlotActionMode::ApplyActionToSelection};
};

#endif
