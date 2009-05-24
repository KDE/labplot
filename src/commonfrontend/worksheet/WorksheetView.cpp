/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot/SciDAVis
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#include "worksheet/WorksheetView.h"
#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetModel.h"
#include "worksheet/WorksheetElementGroup.h"
#include "worksheet/DecorationPlot.h"
#include "worksheet/CartesianCoordinateSystem.h"
#include "worksheet/WorksheetRectangleElement.h"
#include "worksheet/LinearAxis.h"
#include "worksheet/LineSymbolCurve.h"
#include "worksheet/PlotArea.h"
#include "lib/ActionManager.h"
#include "core/column/Column.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QWheelEvent>
#include <QtGlobal>
#include <QShortcut>
#include <QTimer>
#include <QDebug>

#define SHADOW_SIZE 80

/**
 * \class WorksheetGraphicsView
 * \brief Internal class for WorksheetView.
 */

WorksheetGraphicsView::WorksheetGraphicsView(QWidget * parent)
		: QGraphicsView(parent), m_scale_factor(1.0) {
	// TODO: make these global options
	horizontal_screen_dpi = 96;
	vertical_screen_dpi = 96;
	setRenderHint(QPainter::Antialiasing);
}

WorksheetGraphicsView::~WorksheetGraphicsView() {
}

void WorksheetGraphicsView::setScene(QGraphicsScene * scene) {
	QGraphicsView::setScene(scene);
	setSceneRect(scene->sceneRect());
	setScaleFactor(1.0);
}

void WorksheetGraphicsView::drawBackground(QPainter * painter, const QRectF & rect) {
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);
    QRectF scene_rect = sceneRect();

	// background
	if (!scene_rect.contains(rect))
		painter->fillRect(rect, Qt::lightGray);

	// shadow
    QRectF right_shadow(scene_rect.right(), scene_rect.top() + SHADOW_SIZE/vertical_screen_dpi/m_scale_factor, 
		SHADOW_SIZE/horizontal_screen_dpi/m_scale_factor, scene_rect.height());
    QRectF bottom_shadow(scene_rect.left() + SHADOW_SIZE/horizontal_screen_dpi/m_scale_factor, scene_rect.bottom(), 
		scene_rect.width(), SHADOW_SIZE/vertical_screen_dpi/m_scale_factor);
//    if (right_shadow.intersects(rect) || right_shadow.contains(rect))

	painter->fillRect(right_shadow.intersected(rect), Qt::black);
//    if (bottom_shadow.intersects(rect) || bottom_shadow.contains(rect))
	painter->fillRect(bottom_shadow.intersected(rect), Qt::black);

	// canvas
	painter->fillRect(scene_rect.intersected(rect), Qt::white);

	painter->restore();
}

void WorksheetGraphicsView::wheelEvent(QWheelEvent *event) {
	if (event->delta() > 0)
		setScaleFactor(scaleFactor() * 2.0);
	else if (event->delta() < 0)
		setScaleFactor(scaleFactor() / 2.0);
	// update();
}

void WorksheetGraphicsView::setScaleFactor(qreal factor) {
    if (factor < 0.01 || factor > 100)
        return;
	m_scale_factor = factor;

	setTransform(QTransform(QMatrix().scale(1.0/25.4*horizontal_screen_dpi*m_scale_factor, 
		1.0/25.4*vertical_screen_dpi*m_scale_factor)));
	emit scaleFactorChanged(factor);
}

qreal WorksheetGraphicsView::scaleFactor() const {
	return m_scale_factor;
}


/**
 * \class WorksheetView
 * \brief Worksheet view
 *
 *
 */

WorksheetView::WorksheetView(Worksheet *worksheet)
 : m_worksheet(worksheet)
{
	m_model = new WorksheetModel(worksheet);
	init();
}

WorksheetView::~WorksheetView() 
{
	delete m_model;
}

//! Private ctor for initActionManager() only
WorksheetView::WorksheetView()
{
	m_model = NULL;
	createActions();
}

void WorksheetView::init() {
	createActions();

	m_main_layout = new QHBoxLayout(this);
	m_main_layout->setSpacing(0);
	m_main_layout->setContentsMargins(0, 0, 0, 0);
	
	m_view_widget = new WorksheetGraphicsView(this);
	m_view_widget->setScene(m_model->scene());
	m_main_layout->addWidget(m_view_widget);
	connect(m_view_widget, SIGNAL(scaleFactorChanged(qreal)), this, SLOT(handleScaleFactorChange(qreal)));

	m_view_widget->setInteractive(true);
	m_view_widget->setDragMode(QGraphicsView::RubberBandDrag);
	m_view_widget->setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
    m_view_widget->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view_widget->setResizeAnchor(QGraphicsView::AnchorViewCenter);
	m_view_widget->setMinimumSize(16, 16);
	
	m_hide_button = new QToolButton();
	m_hide_button->setArrowType(Qt::RightArrow);
	m_hide_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	m_hide_button->setCheckable(false);
	m_main_layout->addWidget(m_hide_button);
	connect(m_hide_button, SIGNAL(pressed()), this, SLOT(toggleControlTabBar()));
	m_control_tabs = new QWidget();
// TODO    ui.setupUi(m_control_tabs);
	m_main_layout->addWidget(m_control_tabs);

	m_view_widget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	m_main_layout->setStretchFactor(m_view_widget, 1);

	setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

	m_view_widget->setFocusPolicy(Qt::StrongFocus);
	setFocusPolicy(Qt::StrongFocus);
	setFocus();

	retranslateStrings();

	connectActions();

// TODO

	// TODO: remove test code
	QShortcut * start_test = new QShortcut(QKeySequence(tr("Ctrl+Shift+T")), m_view_widget);
	connect(start_test, SIGNAL(activated()), this, SLOT(startTestCode()));

	QTimer::singleShot(0, this, SLOT(startTestCode()));
}

#include "worksheet/PlotAreaPrivate.h"

void WorksheetView::startTestCode() {
	QRectF pageRect = m_model->scene()->sceneRect();

	DecorationPlot *plot = new DecorationPlot("plot1");
	m_worksheet->addChild(plot);
	CartesianCoordinateSystem *coordSys = new CartesianCoordinateSystem("coords1");
	coordSys->setPosition(QPointF(pageRect.width() * 0.2, pageRect.height() * 0.8));
	coordSys->setScaleX(0.1);
	coordSys->setScaleY(-0.1);
	plot->addChild(coordSys);
	PlotArea *plotArea = new PlotArea("plot area");
	plotArea->setRect(QRectF(0, 0, 10, 10));
	plotArea->setClippingEnabled(true);
	coordSys->addChild(plotArea);
	
	WorksheetRectangleElement *rect = new WorksheetRectangleElement("rect1");
	rect->setRect(QRectF(12, 12, 300, 3));
	coordSys->addChild(rect);
	WorksheetRectangleElement *rect2 = new WorksheetRectangleElement("rect2");
	rect2->setRect(QRectF(0, 0, 40, 30));
//	m_worksheet->addChild(rect2);
	WorksheetRectangleElement *rect3 = new WorksheetRectangleElement("rect3");
	rect3->setRect(QRectF(pageRect.width() / 2 - 2, pageRect.height() / 2 - 2, 10 + 4, 120 + 4));
//	m_worksheet->addChild(rect3);
	
	WorksheetElementGroup *group1 = new WorksheetElementGroup("some items");
//	group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(5, 5, 20, 20)));
//	group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(4, 5, 25, 15)));
//	group1->addChild(new WorksheetRectangleElement("rect 1", QRectF(5, 3, 26, 25)));
	plotArea->addChild(group1);

	LinearAxis *xAxis1 = new LinearAxis("x axis 1", LinearAxis::axisBottom);
//	plot->addChild(xAxis1);
	LinearAxis *yAxis1 = new LinearAxis("y axis 1", LinearAxis::axisLeft);
//	plot->addChild(yAxis1);

	LinearAxis *xAxis2 = new LinearAxis("x axis 1", LinearAxis::axisBottom);
	coordSys->addChild(xAxis2);
	xAxis2->setMajorTicksLength(3);
	xAxis2->setMinorTicksLength(1);
	xAxis2->setMinorTickCount(3);
	LinearAxis *yAxis2 = new LinearAxis("y axis 1", LinearAxis::axisLeft);
	yAxis2->setMajorTicksLength(3);
	yAxis2->setMinorTicksLength(1);
	yAxis2->setMinorTickCount(4);
	coordSys->addChild(yAxis2);

	LinearAxis *xAxis3 = new LinearAxis("x axis 2", LinearAxis::axisTop);
	xAxis3->setOffset(10);
	coordSys->addChild(xAxis3);
	LinearAxis *yAxis3 = new LinearAxis("y axis 2", LinearAxis::axisRight);
	yAxis3->setOffset(10);
	yAxis3->setMajorTicksDirection(LinearAxis::ticksBoth);
	yAxis3->setMinorTicksDirection(LinearAxis::ticksBoth);
	yAxis3->setTickStart(0.5);
	yAxis3->setTickEnd(9.5);
	yAxis3->setMajorTickCount(9);
	coordSys->addChild(yAxis3);
//	plotArea->addChild(new WorksheetRectangleElement("rect 1", QRectF(2, 2, 2, 2)));

	Column *xc = new Column("xc", SciDAVis::Numeric);
	Column *yc = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<20; i++)	{
		xc->setValueAt(i, i*0.25);
		yc->setValueAt(i, i*i*0.01+1);
	}

	LineSymbolCurve *curve1 = new LineSymbolCurve("curve 1");
	curve1->setXColumn(xc);
	curve1->setYColumn(yc);

	Column *xc2 = new Column("xc", SciDAVis::Numeric);
	Column *yc2 = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<20; i++)	{
		xc2->setValueAt(i, i*0.25);
		yc2->setValueAt(i, i*i*0.01/2+2);
	}
	LineSymbolCurve *curve2 = new LineSymbolCurve("curve 2");
	curve2->setXColumn(xc2);
	curve2->setYColumn(yc2);

	Column *xc3 = new Column("xc", SciDAVis::Numeric);
	Column *yc3 = new Column("yc", SciDAVis::Numeric);
	for (int i=0; i<20; i++)	{
		xc3->setValueAt(i, i*0.25);
		yc3->setValueAt(i, i*i*0.01*2+3);
	}
	LineSymbolCurve *curve3 = new LineSymbolCurve("curve 3");
	curve3->setXColumn(xc3);
	curve3->setYColumn(yc3);

	WorksheetElementContainer *group2 = new WorksheetElementContainer("some more items");
	group2->addChild(curve3);
	plotArea->addChild(group2);


	plotArea->addChild(curve2);
	plotArea->addChild(curve1);
	
//	coordSys->addChild(plotArea);
//	plot->addChild(coordSys);
}

void WorksheetView::createActions() {
// TODO
}
void WorksheetView::connectActions() {
// TODO
}

void WorksheetView::createContextMenu(QMenu *menu) {
// TODO
}

void WorksheetView::fillProjectMenu(QMenu *menu, bool *rc) {
// TODO
}

void WorksheetView::toggleControlTabBar() { 
	m_control_tabs->setVisible(!m_control_tabs->isVisible());
	if (m_control_tabs->isVisible())
		m_hide_button->setArrowType(Qt::RightArrow);
	else
		m_hide_button->setArrowType(Qt::LeftArrow);
}

void WorksheetView::retranslateStrings() {
	m_hide_button->setToolTip(tr("Show/hide control tabs"));
// TODO    ui.retranslateUi(m_control_tabs);

 // TODO
}
	
void WorksheetView::handleScaleFactorChange(qreal factor) {
	qreal percent = factor*100.0;
	emit statusInfo(tr("Scale: %1%").arg(qRound(percent)));
}

/* ========================= static methods ======================= */
ActionManager *WorksheetView::action_manager = 0;

ActionManager *WorksheetView::actionManager()
{
	if (!action_manager)
		initActionManager();
	
	return action_manager;
}

void WorksheetView::initActionManager()
{
	if (!action_manager)
		action_manager = new ActionManager();

	action_manager->setTitle(tr("Worksheet"));
	volatile WorksheetView *action_creator = new WorksheetView(); // initialize the action texts
	delete action_creator;
}

