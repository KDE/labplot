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
#include "worksheet/DecorationPlot.h"
#include "worksheet/WorksheetRectangleElement.h"
#include "lib/ActionManager.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QWheelEvent>
#include <QtGlobal>
#include <QShortcut>

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
}

WorksheetGraphicsView::~WorksheetGraphicsView() {
}

void WorksheetGraphicsView::setScene(QGraphicsScene * scene) {
	QGraphicsView::setScene(scene);
	setSceneRect(scene->sceneRect());
	setScaleFactor(1.0);
}

void WorksheetGraphicsView::drawBackground(QPainter * painter, const QRectF & rect) {
	// TODO: paint in rect only
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);
	// background
	painter->fillRect(rect, Qt::lightGray);

	// shadow
    QRectF scene_rect = sceneRect();
    QRectF right_shadow(scene_rect.right(), scene_rect.top() + SHADOW_SIZE/vertical_screen_dpi/m_scale_factor, 
		SHADOW_SIZE/horizontal_screen_dpi/m_scale_factor, scene_rect.height());
    QRectF bottom_shadow(scene_rect.left() + SHADOW_SIZE/horizontal_screen_dpi/m_scale_factor, scene_rect.bottom(), 
		scene_rect.width(), SHADOW_SIZE/vertical_screen_dpi/m_scale_factor);
    if (right_shadow.intersects(rect) || right_shadow.contains(rect))
		painter->fillRect(right_shadow, Qt::black);
    if (bottom_shadow.intersects(rect) || bottom_shadow.contains(rect))
		painter->fillRect(bottom_shadow, Qt::black);

	// canvas
	painter->fillRect(scene_rect, Qt::white);

	painter->restore();
}

void WorksheetGraphicsView::wheelEvent(QWheelEvent *event) {
	if (event->delta() > 0)
		setScaleFactor(scaleFactor() * 2.0);
	else if (event->delta() < 0)
		setScaleFactor(scaleFactor() / 2.0);
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
	m_model->scene()->addRect(QRectF(100,100,50,20), QPen(QColor(Qt::red)));

	QShortcut * start_test = new QShortcut(QKeySequence(tr("Ctrl+Shift+T")), m_view_widget);
	connect(start_test, SIGNAL(activated()), this, SLOT(startTestCode()));
}

void WorksheetView::startTestCode() {
	DecorationPlot *plot = new DecorationPlot("plot1");
	m_worksheet->addChild(plot);
	WorksheetRectangleElement *rect = new WorksheetRectangleElement("rect1");
	rect->setRect(QRectF(50, 50, 30, 40));
	plot->addChild(rect);
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

