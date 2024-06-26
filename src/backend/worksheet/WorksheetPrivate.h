/*
	File                 : WorksheetPrivate.h
	Project              : LabPlot
	Description          : Private members of Worksheet.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETPRIVATE_H
#define WORKSHEETPRIVATE_H

#include <backend/worksheet/Worksheet.h>

#include <QColor>

class Background;
class TreeModel;
class Worksheet;
class WorksheetElementContainer;

class QBrush;
class QGraphicsScene;

class WorksheetPrivate {
public:
	explicit WorksheetPrivate(Worksheet*);
	virtual ~WorksheetPrivate();

	Worksheet* const q;
	QRectF pageRect;
	QGraphicsScene* m_scene;
	Worksheet::ZoomFit zoomFit{Worksheet::ZoomFit::Fit};
	bool useViewSize{false};
	bool scaleContent{false};

	QString name() const;
	void update();
	void updateLayout(bool undoable = true);
	void setContainerRect(WorksheetElementContainer*, double x, double y, double h, double w, bool undoable);
	void updatePageRect();

	Background* background{nullptr};
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
	bool plotsInteractive{true};
	bool updateCompleteCursorModel{true};
	Worksheet::CartesianPlotActionMode cartesianPlotActionMode{Worksheet::CartesianPlotActionMode::ApplyActionToSelection};
	Worksheet::CartesianPlotActionMode cartesianPlotCursorMode{Worksheet::CartesianPlotActionMode::ApplyActionToAll};

	int cursorTreeModelCurveBackgroundAlpha{50};
	enum class TreeModelColumn { PLOTNAME = 0, SIGNALNAME = 0, CURSOR0, CURSOR1, CURSORDIFF };

	TreeModel* cursorData{nullptr};

	friend class WorksheetTest;
};

#endif
