/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot/SciDAVis
    Description          : A plot containing decoration elements.
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

#include "CartesianPlot.h"
#include "CartesianCoordinateSystem.h"
#include "Axis.h"
#include "XYCurve.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include <math.h>

#include <QDebug>
#include <QMenu>
#include <QToolBar>
 
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#else
#include <KIcon>
#include <KAction>
#include <KLocale>
#endif

#define SCALE_MIN CartesianCoordinateSystem::Scale::LIMIT_MIN
#define SCALE_MAX CartesianCoordinateSystem::Scale::LIMIT_MAX

/**
 * \class CartesianPlot
 * \brief A xy-plot.
 *
 * 
 */

class CartesianPlotPrivate:public AbstractPlotPrivate{
    public:
		CartesianPlotPrivate(CartesianPlot *owner);
		QString name() const;
		void setRect(const QRectF& r);
		QVariant itemChange(GraphicsItemChange change, const QVariant &value);
		virtual void retransform();
		void retransformScales();
		float round(float value, int precision);

		float xMin, xMax, yMin, yMax;
		float xMinPrev, xMaxPrev, yMinPrev, yMaxPrev;
		bool autoScaleX, autoScaleY;
		float autoScaleOffsetFactor;
		CartesianPlot::Scale xScale, yScale;
};

CartesianPlot::CartesianPlot(const QString &name):AbstractPlot(name, new CartesianPlotPrivate(this)){
	init();
}

CartesianPlot::CartesianPlot(const QString &name, CartesianPlotPrivate *dd):AbstractPlot(name, dd){
	init();
}

CartesianPlot::~CartesianPlot(){
	//TODO
// 	delete d_ptr;
// 	delete m_title;
// 	delete m_coordinateSystem;
// 	delete m_plotArea;
// 	delete addNewMenu;
}

/*!
	initializes all member variables of \c CartesianPlot
 */
void CartesianPlot::init(){
	Q_D(CartesianPlot);

	m_coordinateSystem = new CartesianCoordinateSystem(this);
	d->xMin = 0;
	d->xMax = 1;
	d->yMin = 0;
	d->yMax = 1;
	d->xMinPrev = d->xMin;
	d->xMaxPrev = d->xMax;
	d->yMinPrev = d->yMin;
	d->yMaxPrev = d->yMax;
	d->autoScaleX = true;
	d->autoScaleY = true;
	d->xScale = ScaleLinear;
	d->yScale = ScaleLinear;

	//the following factor determines the size of the offset between the min/max points of the curves
	//and the coordinate system ranges, when doing auto scaling
	//Factor 1 corresponds to the exact match - min/max values of the curves correspond to the start/end values of the ranges.
	d->autoScaleOffsetFactor = 0.05; 

	//TODO: make this factor optional.
	//Provide in the UI the possibility to choose between "exact" or 0% offset, 2%, 5% and 10% for the auto fit option
	
	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);

	//offset between the plot area and the area defining the coordinate system, in scene units.
	d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);
	d->verticalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);
	
	initActions();
	initMenus();
}

/*!
	initializes all children of \c CartesianPlot and 
	setups a default plot with for axes and plot title.
 */
void CartesianPlot::initDefault(){
	Q_D(CartesianPlot);
	
	//Axes
	Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
	addChild(axis);
	axis->setPosition(Axis::AxisBottom);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);

	axis = new Axis("x axis 2", Axis::AxisHorizontal);
	addChild(axis);
	axis->setPosition(Axis::AxisTop);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	axis->setLabelsPosition(Axis::NoLabels);
	axis->title()->setText("");

	axis = new Axis("y axis 1", Axis::AxisVertical);
	addChild(axis);
	axis->setPosition(Axis::AxisLeft);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	
	axis = new Axis("y axis 2", Axis::AxisVertical);
	addChild(axis);
	axis->setPosition(Axis::AxisRight);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setOffset(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	axis->setLabelsPosition(Axis::NoLabels);
	axis->title()->setText("");
	
	//Plot title
 	m_title = new TextLabel(this->name());
	m_title->setText(this->name());
	addChild(m_title);
	m_title->setHidden(true);
	m_title->graphicsItem()->setParentItem(m_plotArea->graphicsItem()); //set the parent before doing any positioning
	m_title->setHorizontalPosition(TextLabel::hPositionCenter);
	m_title->setVerticalPosition(TextLabel::vPositionTop);
	m_title->setHorizontalAlignment(TextLabel::hAlignCenter);
	m_title->setVerticalAlignment(TextLabel::vAlignBottom);
	
	//Geometry, specify the plot rect in scene coordinates.
	//TODO: Use default settings for left, top, width, height and for min/max for the coordinate system
	float x = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float y = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float w = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);
	float h = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);
	
	//all plot children are initialized -> set the geometry of the plot in scene coordinates.
	d->setRect(QRectF(x,y,w,h));
}

void CartesianPlot::initActions(){
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	addCurveAction = new QAction(tr("xy-curve"), this);
	addHorizontalAxisAction = new QAction(tr("horizontal axis"), this);
	addVerticalAxisAction = new QAction(tr("vertical axis"), this);

	scaleAutoAction = new QAction(i18n("auto scale"), this);
	scaleAutoXAction = new QAction(i18n("auto scale X"), this);
	scaleAutoYAction = new QAction(i18n("auto scale Y"), this);
	zoomInAction = new QAction(i18n("zoom in"), this);
	zoomOutAction = new QAction(i18n("zoom out"), this);
	zoomInXAction = new QAction(i18n("zoom in X"), this);
	zoomOutXAction = new QAction(i18n("zoom out X"), this);
	zoomInYAction = new QAction(i18n("zoom in Y"), this);
	zoomOutYAction = new QAction(i18n("zoom out Y"), this);
    shiftLeftXAction = new QAction(i18n("shift left X"), this);
	shiftRightXAction = new QAction(i18n("shift right X"), this);
	shiftUpYAction = new QAction(i18n("shift up Y"), this);
	shiftDownYAction = new QAction(i18n("shift down Y"), this);
#else
	addCurveAction = new KAction(KIcon("xy-curve"), i18n("xy-curve"), this);
	addHorizontalAxisAction = new KAction(KIcon("axis-horizontal"), i18n("horizontal axis"), this);
	addVerticalAxisAction = new KAction(KIcon("axis-vertical"), i18n("vertical axis"), this);
	
	scaleAutoAction = new KAction(KIcon("auto-scale-all"), i18n("auto scale"), this);
	scaleAutoXAction = new KAction(KIcon("auto-scale-x"), i18n("auto scale X"), this);
	scaleAutoYAction = new KAction(KIcon("auto-scale-y"), i18n("auto scale Y"), this);
	zoomInAction = new KAction(KIcon("zoom-in"), i18n("zoom in"), this);
	zoomOutAction = new KAction(KIcon("zoom-out"), i18n("zoom out"), this);
	zoomInXAction = new KAction(KIcon("zoom-in-x"), i18n("zoom in X"), this);
	zoomOutXAction = new KAction(KIcon("zoom-out-x"), i18n("zoom out X"), this);
	zoomInYAction = new KAction(KIcon("zoom-in-y"), i18n("zoom in Y"), this);
	zoomOutYAction = new KAction(KIcon("zoom-out-y"), i18n("zoom out Y"), this);
    shiftLeftXAction = new KAction(KIcon("shift-left-x"), i18n("shift left X"), this);
	shiftRightXAction = new KAction(KIcon("shift-right-x"), i18n("shift right X"), this);
	shiftUpYAction = new KAction(KIcon("shift-up-y"), i18n("shift up Y"), this);
	shiftDownYAction = new KAction(KIcon("shift-down-y"), i18n("shift down Y"), this);
#endif
	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
	connect(addHorizontalAxisAction, SIGNAL(triggered()), SLOT(addAxis()));
	connect(addVerticalAxisAction, SIGNAL(triggered()), SLOT(addAxis()));
	
	//zoom actions
	connect(scaleAutoAction, SIGNAL(triggered()), SLOT(scaleAuto()));
	connect(scaleAutoXAction, SIGNAL(triggered()), SLOT(scaleAutoX()));
	connect(scaleAutoYAction, SIGNAL(triggered()), SLOT(scaleAutoY()));
	connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));
	connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));
	connect(zoomInXAction, SIGNAL(triggered()), SLOT(zoomInX()));
	connect(zoomOutXAction, SIGNAL(triggered()), SLOT(zoomOutX()));
	connect(zoomInYAction, SIGNAL(triggered()), SLOT(zoomInY()));
	connect(zoomOutYAction, SIGNAL(triggered()), SLOT(zoomOutY()));
	connect(shiftLeftXAction, SIGNAL(triggered()), SLOT(shiftLeftX()));
	connect(shiftRightXAction, SIGNAL(triggered()), SLOT(shiftRightX()));
	connect(shiftUpYAction, SIGNAL(triggered()), SLOT(shiftUpY()));
	connect(shiftDownYAction, SIGNAL(triggered()), SLOT(shiftDownY()));
	
}

void CartesianPlot::initMenus(){
	addNewMenu = new QMenu(tr("Add new"));
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addHorizontalAxisAction);
	addNewMenu->addAction(addVerticalAxisAction);
	
	zoomMenu = new QMenu(tr("Zoom"));
	zoomMenu->addAction(scaleAutoAction);
	zoomMenu->addAction(scaleAutoXAction);
	zoomMenu->addAction(scaleAutoYAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInAction);
	zoomMenu->addAction(zoomOutAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInXAction);
	zoomMenu->addAction(zoomOutXAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInYAction);
	zoomMenu->addAction(zoomOutYAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftLeftXAction);
	zoomMenu->addAction(shiftRightXAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftUpYAction);
	zoomMenu->addAction(shiftDownYAction);
}

QMenu *CartesianPlot::createContextMenu(){
	QMenu *menu = AbstractWorksheetElement::createContextMenu();

	QAction* firstAction = menu->actions().first();
	menu->insertMenu(firstAction, addNewMenu);
	menu->insertMenu(firstAction, zoomMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

void CartesianPlot::fillToolBar(QToolBar* toolBar) const{
	toolBar->addAction(addCurveAction);
	toolBar->addSeparator();
	toolBar->addAction(addHorizontalAxisAction);
	toolBar->addAction(addVerticalAxisAction);
	
	toolBar->addAction(scaleAutoAction);
	toolBar->addAction(scaleAutoXAction);
	toolBar->addAction(scaleAutoYAction);
	toolBar->addAction(zoomInAction);
	toolBar->addAction(zoomOutAction);
	toolBar->addAction(zoomInXAction);
	toolBar->addAction(zoomOutXAction);
	toolBar->addAction(zoomInYAction);
	toolBar->addAction(zoomOutYAction);
	toolBar->addAction(shiftLeftXAction);
	toolBar->addAction(shiftRightXAction);
	toolBar->addAction(shiftUpYAction);
	toolBar->addAction(shiftDownYAction);	
}
/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlot::icon() const{
	QIcon ico;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	ico.addPixmap(QPixmap(":/graph.xpm"));
#else
	ico = KIcon("office-chart-line");
#endif
	return ico;
}

/*!
	set the rectangular, defined in scene coordinates
 */
void CartesianPlot::setRect(const QRectF& r){
	Q_D(CartesianPlot);
	d->setRect(r);
}

/* =================================== getter methods ============================ */
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleX, autoScaleX)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, xMin, xMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, xMax, xMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, xScale, xScale)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleY, autoScaleY)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, yMin, yMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, yMax, yMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, yScale, yScale)

/* ============================ setter methods and undo commands ================= */
//TODO: provide an undo-aware version
// STD_SETTER_CMD_IMPL_F(CartesianPlot, SetAutoScaleX, bool, autoScaleX, retransformScales)
void CartesianPlot::setAutoScaleX(bool autoScaleX){
	Q_D(CartesianPlot);
	if (autoScaleX != d->autoScaleX){
// 		exec(new CartesianPlotSetAutoScaleXCmd(d, autoScaleX, tr("%1: change auto scale x")));
		d->autoScaleX = autoScaleX;
		if (autoScaleX)
			this->scaleAutoX();
	}
}

STD_SETTER_CMD_IMPL_F(CartesianPlot, SetXMin, float, xMin, retransformScales)
void CartesianPlot::setXMin(float xMin){
	Q_D(CartesianPlot);
	if (xMin != d->xMin)
		exec(new CartesianPlotSetXMinCmd(d, xMin, tr("%1: set min x")));
}

STD_SETTER_CMD_IMPL_F(CartesianPlot, SetXMax, float, xMax, retransformScales);
void CartesianPlot::setXMax(float xMax){
	Q_D(CartesianPlot);
	if (xMax != d->xMax)
		exec(new CartesianPlotSetXMaxCmd(d, xMax, tr("%1: set max x")));
}

STD_SETTER_CMD_IMPL_F(CartesianPlot, SetXScale, CartesianPlot::Scale, xScale, retransformScales);
void CartesianPlot::setXScale(Scale scale){
	Q_D(CartesianPlot);
	if (scale != d->xScale)
		exec(new CartesianPlotSetXScaleCmd(d, scale, tr("%1: set x scale")));
}

//TODO: provide an undo-aware version
// STD_SETTER_CMD_IMPL_F(CartesianPlot, SetAutoScaleY, bool, autoScaleY, retransformScales)
void CartesianPlot::setAutoScaleY(bool autoScaleY){
	Q_D(CartesianPlot);
	if (autoScaleY != d->autoScaleY){
// 		exec(new CartesianPlotSetAutoScaleYCmd(d, autoScaleY, tr("%1: change auto scale y")));
		d->autoScaleY = autoScaleY;
		if (autoScaleY)
			this->scaleAutoY();
	}
}

STD_SETTER_CMD_IMPL_F(CartesianPlot, SetYMin, float, yMin, retransformScales);
void CartesianPlot::setYMin(float yMin){
	Q_D(CartesianPlot);
	if (yMin != d->yMin)
		exec(new CartesianPlotSetYMinCmd(d, yMin, tr("%1: set min y")));
}

STD_SETTER_CMD_IMPL_F(CartesianPlot, SetYMax, float, yMax, retransformScales);
void CartesianPlot::setYMax(float yMax){
	Q_D(CartesianPlot);
	if (yMax != d->yMax)
		exec(new CartesianPlotSetYMaxCmd(d, yMax, tr("%1: set max y")));
}

STD_SETTER_CMD_IMPL_F(CartesianPlot, SetYScale, CartesianPlot::Scale, yScale, retransformScales);
void CartesianPlot::setYScale(Scale scale){
	Q_D(CartesianPlot);
	if (scale != d->yScale)
		exec(new CartesianPlotSetYScaleCmd(d, scale, tr("%1: set y scale")));
}

//################################################################
//########################## Slots ###############################
//################################################################
void CartesianPlot::addAxis(){
	if (QObject::sender() == addHorizontalAxisAction)
		addChild(new Axis("x-axis", Axis::AxisHorizontal));
	else
		addChild(new Axis("y-axis", Axis::AxisVertical));
}

void CartesianPlot::addCurve(){
	XYCurve* curve = new XYCurve("xy-curve");
	this->addChild(curve);
	connect(curve, SIGNAL(xDataChanged()), this, SLOT(xDataChanged()));
	connect(curve, SIGNAL(yDataChanged()), this, SLOT(yDataChanged()));		
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::xDataChanged(){
	Q_D(CartesianPlot);
	if (!d->autoScaleX)
		return;
	
	this->scaleAutoX();
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::yDataChanged(){
	Q_D(CartesianPlot);
	if (!d->autoScaleY)
		return;
	
	this->scaleAutoY();
}

void CartesianPlot::scaleAutoX(){
	Q_D(CartesianPlot);
	
	//loop over all xy-curves and determine the maximum x-value
	double min = INFINITY;
	double max = -INFINITY;
	XYCurve* curve;
	QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
    foreach(AbstractWorksheetElement *elem, childElements){
		curve = qobject_cast<XYCurve*>(elem);
		if (!curve)
			continue;

		if (!curve->xColumn())
			continue;

		if (curve->xColumn()->minimum() != INFINITY){
			if (curve->xColumn()->minimum() < min)
				min = curve->xColumn()->minimum();
		}
		
		if (curve->xColumn()->maximum() != -INFINITY){
			if (curve->xColumn()->maximum() > max)
				max = curve->xColumn()->maximum();
		}
	}

	bool update = false;
	if (min != d->xMin && min != INFINITY){
		d->xMin = min;
		update = true;
	}
	
	if (max != d->xMax && max != -INFINITY){
		d->xMax = max;
		update = true;
	}

	if(update){
		float offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
		d->xMin -= offset;
		d->xMax += offset;
		d->retransformScales();
	}
		
}

void CartesianPlot::scaleAutoY(){
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum y-value
	double min = INFINITY;
	double max = -INFINITY;
	XYCurve* curve;
	QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
    foreach(AbstractWorksheetElement *elem, childElements){
		curve = qobject_cast<XYCurve*>(elem);
		if (!curve)
			continue;
		
		if (!curve->yColumn())
			continue;
		
		if (curve->yColumn()->minimum() != INFINITY){
			if (curve->yColumn()->minimum() < min)
				min = curve->yColumn()->minimum();
		}
		
		if (curve->yColumn()->maximum() != -INFINITY){
			if (curve->yColumn()->maximum() > max)
				max = curve->yColumn()->maximum();
		}
	}

	bool update = false;
	if (min != d->yMin && min != INFINITY){
		d->yMin = min;
		update = true;
	}
	
	if (max != d->yMax && max != -INFINITY){
		d->yMax = max;
		update = true;
	}
	
	if(update){
		float offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
		d->yMin -= offset;
		d->yMax += offset;
		d->retransformScales();
	}
}

void CartesianPlot::scaleAuto(){
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum x-value
	double xMin = INFINITY;
	double xMax = -INFINITY;
	double yMin = INFINITY;
	double yMax = -INFINITY;
	XYCurve* curve;
	QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
    foreach(AbstractWorksheetElement *elem, childElements){
		curve = qobject_cast<XYCurve*>(elem);
		if (!curve)
			continue;

		if (!curve->xColumn())
			continue;

		if (curve->xColumn()->minimum() != INFINITY){
			if (curve->xColumn()->minimum() < xMin)
				xMin = curve->xColumn()->minimum();
		}
		
		if (curve->xColumn()->maximum() != -INFINITY){
			if (curve->xColumn()->maximum() > xMax)
				xMax = curve->xColumn()->maximum();
		}

		if (!curve->yColumn())
			continue;
		
		if (curve->yColumn()->minimum() != INFINITY){
			if (curve->yColumn()->minimum() < yMin)
				yMin = curve->yColumn()->minimum();
		}
		
		if (curve->yColumn()->maximum() != -INFINITY){
			if (curve->yColumn()->maximum() > yMax)
				yMax = curve->yColumn()->maximum();
		}		
	}

	bool updateX = false;
	bool updateY = false;
	if (xMin != d->xMin && xMin != INFINITY){
		d->xMin = xMin;
		updateX = true;
	}

	if (xMax != d->xMax && xMax != -INFINITY){
		d->xMax = xMax;
		updateX = true;
	}

	if (yMin != d->yMin && yMin != INFINITY){
		d->yMin = yMin;
		updateY = true;
	}

	if (yMax != d->yMax && yMax != -INFINITY){
		d->yMax = yMax;
		updateY = true;
	}

	if(updateX || updateY){
		if (updateX){
			float offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
			d->xMin -= offset;
			d->xMax += offset;
		}
		if (updateY){
			float offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
			d->yMin -= offset;
			d->yMax += offset;
		}
		d->retransformScales();
	}
}
		
void CartesianPlot::zoomIn(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax -= offsetX;
	d->xMin += offsetX;
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax -= offsetY;
	d->yMin += offsetY;
	d->retransformScales();
}

void CartesianPlot::zoomOut(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax += offsetX;
	d->xMin -= offsetX;
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax += offsetY;
	d->yMin -= offsetY;
	d->retransformScales();
}

void CartesianPlot::zoomInX(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax -= offsetX;
	d->xMin += offsetX;
	d->retransformScales();
}

void CartesianPlot::zoomOutX(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax += offsetX;
	d->xMin -= offsetX;
	d->retransformScales();
}

void CartesianPlot::zoomInY(){
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax -= offsetY;
	d->yMin += offsetY;
	d->retransformScales();
}

void CartesianPlot::zoomOutY(){
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax += offsetY;
	d->yMin -= offsetY;
	d->retransformScales();
}

void CartesianPlot::shiftLeftX(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax -= offsetX;
	d->xMin -= offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftRightX(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax += offsetX;
	d->xMin += offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftUpY(){
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax += offsetY;
	d->yMin += offsetY;
	d->retransformScales();
}

void CartesianPlot::shiftDownY(){
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax -= offsetY;
	d->yMin -= offsetY;
	d->retransformScales();
}
		
//#####################################################################
//################### Private implementation ##########################
//#####################################################################
CartesianPlotPrivate::CartesianPlotPrivate(CartesianPlot *owner) : AbstractPlotPrivate(owner){
}

QString CartesianPlotPrivate::name() const{
	return q->name();
}

/*!
	sets the rectangular of the plot in scene coordinates to \c r.
	The size of the plot corresponds to the size of the plot area, the area which is filled with the background color etc.
	and which can pose the parent item for several sub-items (like TextLabel).
	Note, the size of the area used to define the coordinate system doesn't need to be equal to this plot area.
	Also, the size (=bounding box) of CartesianPlot can be greater than the size of the plot area.
 */
void CartesianPlotPrivate::setRect(const QRectF& r){
	prepareGeometryChange();
	setPos( r.x()+r.width()/2, r.y()+r.height()/2);
	rect = r;
	this->retransform();
}

void CartesianPlotPrivate::retransform(){
	retransformScales();
	
	AbstractPlot* plot = dynamic_cast<AbstractPlot*>(q);
	
	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	plot->plotArea()->setRect(rect);

	//call retransform() for the title:
	//when a predefined title position (Left, Centered etc.) is used, 
	//the actual title position needs to be updated on plot's geometry changes.
	plot->title()->retransform();

	q->retransform();
}

void CartesianPlotPrivate::retransformScales(){
	CartesianPlot* plot = dynamic_cast<CartesianPlot*>(q);
	CartesianCoordinateSystem *cSystem = dynamic_cast<CartesianCoordinateSystem *>(plot->coordinateSystem());
	QList<CartesianCoordinateSystem::Scale*> scales;
	
	//perform the mapping from the scene coordinates to the plot's coordinates here.
	QRectF itemRect = mapRectFromScene( rect );

	//create x-scales
	//TODO: for negative values of xMin and yMin use a value smaller that xMax/yMax and not 0.1
	if (xScale == CartesianPlot::ScaleLinear){
		scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.x()+horizontalPadding,
																	itemRect.x()+itemRect.width()-horizontalPadding,
																	xMin, xMax);
	}else if (xScale == CartesianPlot::ScaleLog10){
		if (xMin<=0) xMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.x()+horizontalPadding,
																	itemRect.x()+itemRect.width()-horizontalPadding,
																	xMin, xMax, 10.0);
	}else if (xScale == CartesianPlot::ScaleLog2){
		if (xMin<=0) xMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.x()+horizontalPadding,
																	itemRect.x()+itemRect.width()-horizontalPadding,
																	xMin, xMax, 2.0);
// 	}else if (xScale == CartesianPlot::ScaleLn){
	}else{
		if (xMin<=0) xMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.x()+horizontalPadding,
																	itemRect.x()+itemRect.width()-horizontalPadding,
																	xMin, xMax, 2.71828);
	}

	cSystem ->setXScales(scales);

	//create y-scales
	scales.clear();
	if (yScale == CartesianPlot::ScaleLinear){
		scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.y()+itemRect.height()-verticalPadding,
																	itemRect.y()+verticalPadding, 
																	yMin, yMax);
	}else if (yScale == CartesianPlot::ScaleLog10){
		if (yMin<=0) yMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.y()+itemRect.height()-verticalPadding,
																	itemRect.y()+verticalPadding, 
																	yMin, yMax, 10.0);
	}else if (yScale == CartesianPlot::ScaleLog2){
		if (yMin<=0) yMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.y()+itemRect.height()-verticalPadding,
																	itemRect.y()+verticalPadding,
																	yMin, yMax, 2.0);
	}else{
		if (yMin<=0) yMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.y()+itemRect.height()-verticalPadding,
																	itemRect.y()+verticalPadding,
																	yMin, yMax, 2.71828);
	}
	
	cSystem ->setYScales(scales);

	//calculate the changes in x and y and save the current values for xMin, xMax, yMin, yMax
	float deltaX = 0;
	float deltaY = 0;

	if (xMin!=xMinPrev){
		deltaX = xMin - xMinPrev;
		emit plot->xMinChanged(xMin);
	}

	if (xMax!=xMaxPrev)
		emit plot->xMaxChanged(xMax);

	if (yMin!=yMinPrev){
		deltaY = yMin - yMinPrev;
		emit plot->yMinChanged(yMin);
	}

	if (yMax!=yMaxPrev)
		emit plot->yMaxChanged(yMax);
	
	xMinPrev = xMin;
	xMaxPrev = xMax;
	yMinPrev = yMin;
	yMaxPrev = yMax;

	//adjust auto-scale axes
	QList<AbstractWorksheetElement *> childElements = q->children<AbstractWorksheetElement>();
    foreach(AbstractWorksheetElement *elem, childElements){
		Axis* axis = qobject_cast<Axis*>(elem);
		if (axis){
			if (!axis->autoScale())
				continue;
			
			if (axis->orientation() == Axis::AxisHorizontal){
				axis->setEnd(xMax, false);
				axis->setStart(xMin, false);
				if (axis->position() == Axis::AxisCustom){
					axis->setOffset(axis->offset() + deltaY, false);
// 					qDebug()<<" new offset "<<axis->offset() + deltaY;
				}
			}else{
				axis->setEnd(yMax, false);
				axis->setStart(yMin, false);
				if (axis->position() == Axis::AxisCustom){
					axis->setOffset(axis->offset() + deltaX, false);
// 					qDebug()<<" new offset "<<axis->offset() + deltaX;
				}
			}
		}
	}
	
	// call retransform() on the parent to trigger the update of all axes and curves
	q->retransform();
}

float CartesianPlotPrivate::round(float value, int precision){
	return int(value*pow(10, precision) + (value<0 ? -0.5 : 0.5))/pow(10, precision);
}

/*!
 * Reimplemented from QGraphicsItem.
 */
QVariant CartesianPlotPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
	if (change == QGraphicsItem::ItemPositionChange) {
		QPointF itemPos = value.toPointF();//item's center point in parent's coordinates;
		float x = itemPos.x();
		float y = itemPos.y();
		
		//update rect
		float w = rect.width();
		float h = rect.height();
		rect.setX(x-w/2);
		rect.setY(y-h/2);
		rect.setWidth(w);
		rect.setHeight(h);
		emit dynamic_cast<CartesianPlot*>(q)->positionChanged();
     }
	return QGraphicsItem::itemChange(change, value);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void CartesianPlot::save(QXmlStreamWriter* writer) const{
	Q_D(const CartesianPlot);

    writer->writeStartElement( "cartesianPlot" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);

	//geometry
    writer->writeStartElement( "geometry" );
    writer->writeAttribute( "x", QString::number(d->rect.x()) );
    writer->writeAttribute( "y", QString::number(d->rect.y()) );
    writer->writeAttribute( "width", QString::number(d->rect.width()) );
    writer->writeAttribute( "height", QString::number(d->rect.height()) );
    writer->writeEndElement();
	
	//coordinate system and padding
// 	m_coordinateSystem->save(writer); //TODO save scales
	writer->writeStartElement( "coordinateSystem" );
    writer->writeAttribute( "xMin", QString::number(d->xMin) );
	writer->writeAttribute( "xMax", QString::number(d->xMax) );
	writer->writeAttribute( "yMin", QString::number(d->yMin) );
	writer->writeAttribute( "yMax", QString::number(d->yMax) );
	writer->writeAttribute( "horizontalPadding", QString::number(d->horizontalPadding) );
	writer->writeAttribute( "verticalPadding", QString::number(d->verticalPadding) );
	writer->writeEndElement();
	
    //serialize all children (plot area, title text label, axes and curves)
    QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
    foreach(AbstractWorksheetElement *elem, childElements)
        elem->save(writer);

    writer->writeEndElement(); // close "cartesianPlot" section
}


//! Load from XML
bool CartesianPlot::load(XmlStreamReader* reader){
	Q_D(CartesianPlot);

    if(!reader->isStartElement() || reader->name() != "cartesianPlot"){
        reader->raiseError(tr("no cartesianPlot element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = tr("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;
    QRectF sceneRect;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "cartesianPlot")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader))
				return false;
		}else if(reader->name() == "geometry"){
            attribs = reader->attributes();
	
            str = attribs.value("x").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'x'"));
            else
                sceneRect.setX( str.toDouble() );

			str = attribs.value("y").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'y'"));
            else
                sceneRect.setY( str.toDouble() );
			
			str = attribs.value("width").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'width'"));
            else
                sceneRect.setWidth( str.toDouble() );
			
			str = attribs.value("height").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'height'"));
            else
                sceneRect.setHeight( str.toDouble() );
		}else if(reader->name() == "coordinateSystem"){
// 			m_coordinateSystem->load(reader); //TODO read scales
            attribs = reader->attributes();
	
            str = attribs.value("xMin").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'xMin'"));
            else
                d->xMin = str.toDouble();

            str = attribs.value("xMax").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'xMax'"));
            else
                d->xMax = str.toDouble();
			
            str = attribs.value("yMin").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'yMin'"));
            else
                d->yMin = str.toDouble();
			
            str = attribs.value("yMax").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'yMax'"));
            else
                d->yMax = str.toDouble();
			
            str = attribs.value("horizontalPadding").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'horizontalPadding'"));
            else
                d->horizontalPadding = str.toDouble();
			
            str = attribs.value("verticalPadding").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'verticalPadding'"));
            else
                d->verticalPadding = str.toDouble();	
        }else if(reader->name() == "textLabel"){
            m_title = new TextLabel("");
            if (!m_title->load(reader)){
                delete m_title;
				m_title=0;
                return false;
            }else{
                addChild(m_title);
            }
		}else if(reader->name() == "plotArea"){
			m_plotArea->load(reader);
		}else if(reader->name() == "axis"){
            Axis* axis = new Axis("");
            if (!axis->load(reader)){
                delete axis;
                return false;
            }else{
                addChild(axis);
            }
		}else if(reader->name() == "xyCurve"){
            XYCurve* curve = new XYCurve("");
            if (!curve->load(reader)){
                delete curve;
                return false;
            }else{
                addChild(curve);
            }             
        }else{ // unknown element
            reader->raiseWarning(tr("unknown cartesianPlot element '%1'").arg(reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

	d->setRect(sceneRect);
	m_title->setHidden(true);
	m_title->graphicsItem()->setParentItem(m_plotArea->graphicsItem());
	d->retransformScales();
	retransform();

    return true;
}
