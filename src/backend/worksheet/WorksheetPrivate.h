/*
    File                 : WorksheetPrivate.h
    Project              : LabPlot
    Description          : Private members of Worksheet.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2012 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class TreeModel;

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
	void setContainerRect(WorksheetElementContainer*, double x, double y, double h, double w, bool undoable);
	void updatePageRect();

	PlotArea::BackgroundType backgroundType;
	PlotArea::BackgroundColorStyle backgroundColorStyle;
	PlotArea::BackgroundImageStyle backgroundImageStyle;
	Qt::BrushStyle backgroundBrushStyle;
	QColor backgroundFirstColor;
	QColor backgroundSecondColor;
	QString backgroundFileName;
	double backgroundOpacity{1.0};

	Worksheet::Layout layout{Worksheet::Layout::VerticalLayout};
	bool suppressLayoutUpdate{false};
	bool suppressCursorPosChanged{false};
	double layoutTopMargin{0.0};
	double layoutBottomMargin{0.0};
	double layoutLeftMargin{0.0};
	double layoutRightMargin{0.0};
	double layoutVerticalSpacing{0.0};
	double layoutHorizontalSpacing{0.0};
	int layoutColumnCount{2};
	int layoutRowCount{2};
	QString theme;
	bool plotsLocked{false};
	bool updateCompleteCursorModel{true};
	Worksheet::CartesianPlotActionMode cartesianPlotActionMode{Worksheet::CartesianPlotActionMode::ApplyActionToSelection};
	Worksheet::CartesianPlotActionMode cartesianPlotCursorMode{Worksheet::CartesianPlotActionMode::ApplyActionToAll};

	enum class TreeModelColumn {
		PLOTNAME = 0,
		SIGNALNAME = 0,
		CURSOR0,
		CURSOR1,
		CURSORDIFF
	};

	TreeModel* cursorData{nullptr};
};

#endif
