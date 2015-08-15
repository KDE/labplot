/***************************************************************************
    File                 : Plot3D.cpp
    Project              : LabPlot
    Description          : 3D plot
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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

#include "Plot3D.h"
#include "Plot3DPrivate.h"
#include "Surface3D.h"
#include "Curve3D.h"
#include "Axes.h"
#include "MouseInteractor.h"
#include "VTKGraphicsItem.h"
#include "XmlAttributeReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/core/AbstractAspect.h"

#include <QDebug>
#include <QPainter>
#include <QMenu>

#include <KAction>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

#include <cmath>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkLight.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkQImageToImageSource.h>
#include <vtkWindowToImageFilter.h>
#include <vtkBoundingBox.h>


Plot3D::Plot3D(const QString& name)
	: AbstractPlot(name, new Plot3DPrivate(this)){
}

void Plot3D::childSelected(const AbstractAspect* aspect) {
	Base3D* object = qobject_cast<Base3D*>(const_cast<AbstractAspect*>(aspect));
	if (object != 0)
		object->select(true);

	AbstractPlot::childSelected(aspect);
	Q_D(Plot3D);
	d->vtkItem->refresh();
}

void Plot3D::childDeselected(const AbstractAspect* aspect) {
	Base3D* object = qobject_cast<Base3D*>(const_cast<AbstractAspect*>(aspect));
	if (object != 0)
		object->select(false);

	AbstractPlot::childDeselected(aspect);
	Q_D(Plot3D);
	d->vtkItem->refresh();
}

void Plot3D::handleAspectAdded(const AbstractAspect* aspect) {
	Q_D(Plot3D);
	if (d->isInitialized) {
		Base3D* object = dynamic_cast<Base3D*>(const_cast<AbstractAspect*>(aspect));
		if (object != 0)
			object->recover();
		if (d->axes)
			d->axes->updateBounds();
		d->resetCamera();
		d->vtkItem->refresh();
		Surface3D* surface = dynamic_cast<Surface3D*>(object);
		if (surface) {
			d->surfaces.insert(surface);
		} else {
			Curve3D* curve = dynamic_cast<Curve3D*>(object);
			if (curve) {
				d->curves.insert(curve);
			} else
				d->axes = dynamic_cast<Axes*>(object);
		}
	}
}

void Plot3D::init(bool transform){
	Q_D(Plot3D);
	if (d->isInitialized)
		return;

	if (!d->rectSet || d->context == 0)
		return;

	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);

	//light

	d->init();

	d->isInitialized = true;

	initActions();
	initMenus();

	if (transform)
		retransform();
}

void Plot3D::setContext(QGLContext *context) {
	Q_D(Plot3D);
	d->context = context;
}

Plot3D::~Plot3D(){
}

QIcon Plot3D::icon() const {
	// TODO: Replace by some 3D chart
	return KIcon("office-chart-line");
}

BoundingBox Plot3D::range() const {
	Q_D(const Plot3D);
	if (!d->rangeBounds.isInitialized()) {
		const_cast<BoundingBox&>(d->rangeBounds) = bounds();
	}

	return d->rangeBounds;
}

BoundingBox Plot3D::bounds() const {
	BoundingBox bb;
	Q_D(const Plot3D);
	foreach(Surface3D* surface, d->surfaces) {
		bb.AddBox(surface->bounds());
	}

	foreach(Curve3D* curve, d->curves) {
		bb.AddBox(curve->bounds());
	}

	return bb;
}

void Plot3D::configureAspect(AbstractAspect* aspect) {
	Q_D(Plot3D);
	addChild(aspect);
	connect(aspect, SIGNAL(parametersChanged()), SLOT(onParametersChanged()));
	connect(aspect, SIGNAL(removed()), d->vtkItem, SLOT(refresh()));
	connect(aspect, SIGNAL(removed()), SLOT(onItemRemoved()));
}

void Plot3D::onParametersChanged() {
	Q_D(Plot3D);
	d->axes->updateBounds();
	d->resetCamera();
	d->vtkItem->refresh();
}

void Plot3D::addSurface() {
	Q_D(Plot3D);
	Surface3D* newSurface = new Surface3D;
	newSurface->setXScaling(d->xScaling);
	newSurface->setYScaling(d->yScaling);
	newSurface->setZScaling(d->zScaling);
	newSurface->setRenderer(d->renderer);
	d->surfaces.insert(newSurface);
	configureAspect(newSurface);
	d->resetCamera();
	if (d->axes)
		d->axes->updateBounds();
	d->vtkItem->refresh();
}

void Plot3D::addCurve() {
	Q_D(Plot3D);
	Curve3D* newCurve = new Curve3D;
	newCurve->setXScaling(d->xScaling);
	newCurve->setYScaling(d->yScaling);
	newCurve->setZScaling(d->zScaling);
	newCurve->setRenderer(d->renderer);
	d->curves.insert(newCurve);
	configureAspect(newCurve);
	d->resetCamera();
	d->vtkItem->refresh();
	if (d->axes)
		d->axes->updateBounds();
}

void Plot3D::addAxes() {
	Q_D(Plot3D);
	d->axes = new Axes;
	d->axes->setRenderer(d->renderer);
	configureAspect(d->axes);
	d->axes->updateBounds();
	d->resetCamera();
	d->vtkItem->refresh();
	addAxesAction->setEnabled(false);
}

void Plot3D::onItemRemoved() {
	Q_D(Plot3D);
	// TODO: Update axes bounding box
	Surface3D* surface = qobject_cast<Surface3D*>(sender());
	if (surface != 0) {
		qDebug() << "Remove surface";
		d->surfaces.remove(surface);
		if (d->axes)
			d->axes->updateBounds();
		d->resetCamera();
		return;
	}

	Curve3D* curve = qobject_cast<Curve3D*>(sender());
	if (curve != 0) {
		qDebug() << "Remove curve";
		d->curves.remove(curve);
		if (d->axes)
			d->axes->updateBounds();
		d->resetCamera();
		return;
	}

	Axes* axes = qobject_cast<Axes*>(sender());
	if (axes != 0) {
		qDebug() << "Remove axes";
		d->axes = 0;
		addAxesAction->setEnabled(true);
		return;
	}
}

void Plot3D::onObjectClicked(vtkProp* object) {
	Q_D(Plot3D);
	if (object == 0) {
		// Deselect all Plot3D children
		qDebug() << Q_FUNC_INFO << "Deselect";
		emit currentAspectChanged(this);
		foreach(Surface3D* surface, d->surfaces) {
			surface->select(false);
		}

		foreach(Curve3D* curve, d->curves) {
			curve->select(false);
		}
		d->axes->select(false);
	} else {
		foreach(Surface3D* surface, d->surfaces) {
			if (*surface == object) {
				qDebug() << Q_FUNC_INFO << "Surface clicked" << surface->name();
				emit currentAspectChanged(surface);
				surface->select(true);
			} else {
				surface->select(false);
			}
		}

		foreach(Curve3D* curve, d->curves) {
			if (*curve == object) {
				qDebug() << Q_FUNC_INFO << "Curve clicked" << curve->name();
				emit currentAspectChanged(curve);
				curve->select(true);
			} else {
				curve->select(false);
			}
		}
		if (*d->axes == object) {
			qDebug() << Q_FUNC_INFO << "Axes clicked";
			emit currentAspectChanged(d->axes);
			d->axes->select(true);
		} else {
			d->axes->select(false);
		}
	}
	d->vtkItem->refresh();
}

void Plot3D::onObjectHovered(vtkProp* object) {
	Q_D(Plot3D);

	if (object == 0) {
		foreach(Surface3D* surface, d->surfaces)
			surface->highlight(false);

		foreach(Curve3D* curve, d->curves)
			curve->highlight(false);
		d->axes->highlight(false);
	} else {
		foreach(Surface3D* surface, d->surfaces)
			surface->highlight(*surface == object);

		foreach(Curve3D* curve, d->curves)
			curve->highlight(*curve == object);
		if (*d->axes == object)
			qDebug() << Q_FUNC_INFO << "Axes hovered";
		d->axes->highlight(*d->axes == object);
	}

	d->vtkItem->refresh();
}

void Plot3D::setPrinting(bool pred) {
	plotArea()->setVisible(!pred);
	AbstractPlot::setPrinting(pred);
}

void Plot3D::initActions() {
	Q_D(Plot3D);
	//"add new" actions
	addCurveAction = new KAction(KIcon("3d-curve"), i18n("3D-curve"), this);
	addEquationCurveAction = new KAction(KIcon("3d-equation-curve"), i18n("3D-curve from a mathematical equation"), this);
	addSurfaceAction = new KAction(KIcon("3d-surface"), i18n("3D-surface"), this);
	addAxesAction = new KAction(KIcon("axis-horizontal"), i18n("Axes"), this);
	if (d->axes)
		addAxesAction->setEnabled(false);

 	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
// 	connect(addEquationCurveAction, SIGNAL(triggered()), SLOT(addEquationCurve()));
 	connect(addSurfaceAction, SIGNAL(triggered()), SLOT(addSurface()));
	connect(addAxesAction, SIGNAL(triggered()), SLOT(addAxes()));

	//zoom/navigate actions
	scaleAutoAction = new KAction(KIcon("auto-scale-all"), i18n("auto scale"), this);
	scaleAutoXAction = new KAction(KIcon("auto-scale-x"), i18n("auto scale X"), this);
	scaleAutoYAction = new KAction(KIcon("auto-scale-y"), i18n("auto scale Y"), this);
	scaleAutoZAction = new KAction(KIcon("auto-scale-z"), i18n("auto scale Z"), this);
	zoomInAction = new KAction(KIcon("zoom-in"), i18n("zoom in"), this);
	zoomOutAction = new KAction(KIcon("zoom-out"), i18n("zoom out"), this);
	zoomInXAction = new KAction(KIcon("zoom-in-x"), i18n("zoom in X"), this);
	zoomOutXAction = new KAction(KIcon("zoom-out-x"), i18n("zoom out X"), this);
	zoomInYAction = new KAction(KIcon("zoom-in-y"), i18n("zoom in Y"), this);
	zoomOutYAction = new KAction(KIcon("zoom-out-y"), i18n("zoom out Y"), this);
	zoomInZAction = new KAction(KIcon("zoom-in-z"), i18n("zoom in Z"), this);
	zoomOutZAction = new KAction(KIcon("zoom-out-z"), i18n("zoom out Z"), this);
	shiftLeftXAction = new KAction(KIcon("shift-left-x"), i18n("shift left X"), this);
	shiftRightXAction = new KAction(KIcon("shift-right-x"), i18n("shift right X"), this);
	shiftUpYAction = new KAction(KIcon("shift-up-y"), i18n("shift up Y"), this);
	shiftDownYAction = new KAction(KIcon("shift-down-y"), i18n("shift down Y"), this);
	shiftUpZAction = new KAction(KIcon("shift-up-z"), i18n("shift up Z"), this);
	shiftDownZAction = new KAction(KIcon("shift-down-z"), i18n("shift down Z"), this);

	connect(scaleAutoAction, SIGNAL(triggered()), SLOT(autoScale()));
	connect(scaleAutoXAction, SIGNAL(triggered()), SLOT(autoScaleX()));
	connect(scaleAutoYAction, SIGNAL(triggered()), SLOT(autoScaleY()));
	connect(scaleAutoZAction, SIGNAL(triggered()), SLOT(autoScaleZ()));
	connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));
	connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));
	connect(zoomInXAction, SIGNAL(triggered()), SLOT(zoomInX()));
	connect(zoomOutXAction, SIGNAL(triggered()), SLOT(zoomOutX()));
	connect(zoomInYAction, SIGNAL(triggered()), SLOT(zoomInY()));
	connect(zoomOutYAction, SIGNAL(triggered()), SLOT(zoomOutY()));
	connect(zoomOutZAction, SIGNAL(triggered()), SLOT(zoomOutZ()));
 	connect(zoomInZAction, SIGNAL(triggered()), SLOT(zoomInZ()));

	connect(shiftLeftXAction, SIGNAL(triggered()), SLOT(shiftLeftX()));
	connect(shiftRightXAction, SIGNAL(triggered()), SLOT(shiftRightX()));
	connect(shiftUpYAction, SIGNAL(triggered()), SLOT(shiftUpY()));
	connect(shiftDownYAction, SIGNAL(triggered()), SLOT(shiftDownY()));
	connect(shiftUpZAction, SIGNAL(triggered()), SLOT(shiftUpZ()));
	connect(shiftDownZAction, SIGNAL(triggered()), SLOT(shiftDownZ()));

	//rotation action
	rotateClockwiseAction = new KAction(KIcon("rotate-clockwise"), i18n("rotate clockwise"), this);
	rotateCounterclockwiseAction = new KAction(KIcon("rotate-counterclockwise"), i18n("rotate counterclockwise"), this);
	tiltLeftAction = new KAction(KIcon("tilt-left"), i18n("tilt left"), this);
	tiltRightAction = new KAction(KIcon("tilt-right"), i18n("tilt right"), this);
	tiltUpAction = new KAction(KIcon("tilt-up"), i18n("tilt up"), this);
	tiltDownAction = new KAction(KIcon("tilt-down"), i18n("tilt down"), this);
	resetRotationAction = new KAction(KIcon("reset-rotation"), i18n("reset rotation"), this);

	//visibility action
	visibilityAction = new QAction(i18n("visible"), this);
	visibilityAction->setCheckable(true);
// 	connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

void Plot3D::autoScale() {
	setRange(bounds());
}

void Plot3D::autoScaleX() {
	const BoundingBox& bb = bounds();
	BoundingBox r = range();
	r.setXMin(bb.xMin());
	r.setXMax(bb.xMax());
	setRange(r);
}

void Plot3D::autoScaleY() {
	const BoundingBox& bb = bounds();
	BoundingBox r = range();
	r.setYMin(bb.yMin());
	r.setYMax(bb.yMax());
	setRange(r);
}

void Plot3D::autoScaleZ() {
	const BoundingBox& bb = bounds();
	BoundingBox r = range();
	r.setZMin(bb.zMin());
	r.setZMax(bb.zMax());
	setRange(r);
}

void Plot3D::zoomIn() {
	BoundingBox r = range();
	r.Scale(0.9, 0.9, 0.9);
	setRange(r);
}

void Plot3D::zoomOut() {
	BoundingBox r = range();
	r.Scale(1.1, 1.1, 1.1);
	setRange(r);
}

void Plot3D::zoomInX() {
	BoundingBox r = range();
	r.Scale(0.9, 1, 1);
	setRange(r);
}

void Plot3D::zoomOutX() {
	BoundingBox r = range();
	r.Scale(1.1, 1, 1);
	setRange(r);
}

void Plot3D::zoomInY() {
	BoundingBox r = range();
	r.Scale(1, 0.9, 1);
	setRange(r);
}

void Plot3D::zoomOutY() {
	BoundingBox r = range();
	r.Scale(1, 1.1, 1);
	setRange(r);
}

void Plot3D::zoomInZ() {
	BoundingBox r = range();
	r.Scale(1, 1, 0.9);
	setRange(r);
}

void Plot3D::zoomOutZ() {
	BoundingBox r = range();
	r.Scale(1, 1, 1.1);
	setRange(r);
}

void Plot3D::shiftLeftX() {
	BoundingBox r = range();
	const double dx = (r.xMax() - r.xMin()) * 0.1;
	r.setXMin(r.xMin() - dx);
	r.setXMax(r.xMax() - dx);
	setRange(r);
}

void Plot3D::shiftRightX() {
	BoundingBox r = range();
	const double dx = (r.xMax() - r.xMin()) * 0.1;
	r.setXMin(r.xMin() + dx);
	r.setXMax(r.xMax() + dx);
	setRange(r);
}

void Plot3D::shiftUpY() {
	BoundingBox r = range();
	const double dy = (r.yMax() - r.yMin()) * 0.1;
	r.setYMin(r.yMin() + dy);
	r.setYMax(r.yMax() + dy);
	setRange(r);
}

void Plot3D::shiftDownY() {
	BoundingBox r = range();
	const double dy = (r.yMax() - r.yMin()) * 0.1;
	r.setYMin(r.yMin() - dy);
	r.setYMax(r.yMax() - dy);
	setRange(r);
}

void Plot3D::shiftUpZ() {
	BoundingBox r = range();
	const double dz = (r.zMax() - r.zMin()) * 0.1;
	r.setZMin(r.zMin() + dz);
	r.setZMax(r.zMax() + dz);
	setRange(r);
}

void Plot3D::shiftDownZ() {
	BoundingBox r = range();
	const double dz = (r.zMax() - r.zMin()) * 0.1;
	r.setZMin(r.zMin() - dz);
	r.setZMax(r.zMax() - dz);
	setRange(r);
}

void Plot3D::initMenus(){
	addNewMenu = new QMenu(i18n("Add new"));
	addNewMenu->addAction(addAxesAction);
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addAction(addEquationCurveAction);
	addNewMenu->addAction(addSurfaceAction);

	zoomMenu = new QMenu(i18n("Zoom"));
	zoomMenu->addAction(scaleAutoAction);
	zoomMenu->addAction(scaleAutoXAction);
	zoomMenu->addAction(scaleAutoYAction);
	zoomMenu->addAction(scaleAutoZAction);
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
	zoomMenu->addAction(zoomInZAction);
	zoomMenu->addAction(zoomOutZAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftLeftXAction);
	zoomMenu->addAction(shiftRightXAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftUpYAction);
	zoomMenu->addAction(shiftDownYAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftUpZAction);
	zoomMenu->addAction(shiftDownZAction);

	rotateMenu = new QMenu(i18n("Rotate"));
	rotateMenu->addAction(rotateClockwiseAction);
	rotateMenu->addAction(rotateCounterclockwiseAction);
	rotateMenu->addSeparator();
	rotateMenu->addAction(tiltLeftAction);
	rotateMenu->addAction(tiltRightAction);
	rotateMenu->addAction(tiltUpAction);
	rotateMenu->addAction(tiltDownAction);
	rotateMenu->addSeparator();
	rotateMenu->addAction(resetRotationAction);
	rotateMenu->addSeparator();

	QMenu* angleMenu = new QMenu(i18n("rotation angle"));
	//TODO: use QActionGroup and exlusive actions for this later.
	angleMenu->addAction("1°");
	angleMenu->addAction("2°");
	angleMenu->addAction("5°");
	angleMenu->addAction("10°");
	angleMenu->addAction("20°");
	angleMenu->addAction("30°");
	angleMenu->addAction("40°");
	angleMenu->addAction("50°");
	angleMenu->addAction("60°");
	angleMenu->addAction("70°");
	angleMenu->addAction("80°");
	angleMenu->addAction("90°");

	rotateMenu->addMenu(angleMenu);
}

QMenu* Plot3D::createContextMenu(){
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1);

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	menu->insertMenu(firstAction, addNewMenu);
	menu->insertMenu(firstAction, zoomMenu);
	menu->insertMenu(firstAction, rotateMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

void Plot3D::setRect(const QRectF &rect){
	Q_D(Plot3D);
	d->rect = rect;
	d->rectSet = true;

	retransform();
}

void Plot3D::retransform() {
	Q_D(Plot3D);
	if (d->context == 0)
		return;

	if (!d->isInitialized)
		init(false);

	d->retransform();
	WorksheetElementContainer::retransform();
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
// General
BASIC_SHARED_D_READER_IMPL(Plot3D, Plot3D::Scaling, xScaling, xScaling)
BASIC_SHARED_D_READER_IMPL(Plot3D, Plot3D::Scaling, yScaling, yScaling)
BASIC_SHARED_D_READER_IMPL(Plot3D, Plot3D::Scaling, zScaling, zScaling)

// Background
BASIC_SHARED_D_READER_IMPL(Plot3D, PlotArea::BackgroundType, backgroundType, backgroundType)
BASIC_SHARED_D_READER_IMPL(Plot3D, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle)
BASIC_SHARED_D_READER_IMPL(Plot3D, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle)
BASIC_SHARED_D_READER_IMPL(Plot3D, Qt::BrushStyle, backgroundBrushStyle, backgroundBrushStyle)
CLASS_SHARED_D_READER_IMPL(Plot3D, QColor, backgroundFirstColor, backgroundFirstColor)
CLASS_SHARED_D_READER_IMPL(Plot3D, QColor, backgroundSecondColor, backgroundSecondColor)
CLASS_SHARED_D_READER_IMPL(Plot3D, QString, backgroundFileName, backgroundFileName)
BASIC_SHARED_D_READER_IMPL(Plot3D, float, backgroundOpacity, backgroundOpacity)

// Light
BASIC_SHARED_D_READER_IMPL(Plot3D, double, intensity, intensity)
BASIC_SHARED_D_READER_IMPL(Plot3D, QColor, ambient, ambient)
BASIC_SHARED_D_READER_IMPL(Plot3D, QColor, diffuse, diffuse)
BASIC_SHARED_D_READER_IMPL(Plot3D, QColor, specular, specular)
BASIC_SHARED_D_READER_IMPL(Plot3D, double, elevation, elevation)
BASIC_SHARED_D_READER_IMPL(Plot3D, double, azimuth, azimuth)
BASIC_SHARED_D_READER_IMPL(Plot3D, double, coneAngle, coneAngle)


//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
// General
STD_SETTER_CMD_IMPL_F_S(Plot3D, SetXScaling, Plot3D::Scaling, xScaling, updateXScaling)
STD_SETTER_IMPL(Plot3D, XScaling, Plot3D::Scaling, xScaling, "%1: X axis scaling changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetYScaling, Plot3D::Scaling, yScaling, updateYScaling)
STD_SETTER_IMPL(Plot3D, YScaling, Plot3D::Scaling, yScaling, "%1: Y axis scaling changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetZScaling, Plot3D::Scaling, zScaling, updateZScaling)
STD_SETTER_IMPL(Plot3D, ZScaling, Plot3D::Scaling, zScaling, "%1: Z axis scaling changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetRange, BoundingBox, rangeBounds, updateRange)
STD_SETTER_IMPL(Plot3D, Range, const BoundingBox&, rangeBounds, "%1: bound range changed")

// Background
STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundType, PlotArea::BackgroundType, backgroundType, updateBackground)
STD_SETTER_IMPL(Plot3D, BackgroundType, PlotArea::BackgroundType, backgroundType, "%1: background type changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundColorStyle, PlotArea::BackgroundColorStyle, backgroundColorStyle, updateBackground)
STD_SETTER_IMPL(Plot3D, BackgroundColorStyle, PlotArea::BackgroundColorStyle, backgroundColorStyle, "%1: background color style changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundImageStyle, PlotArea::BackgroundImageStyle, backgroundImageStyle, updateBackground)
STD_SETTER_IMPL(Plot3D, BackgroundImageStyle, PlotArea::BackgroundImageStyle, backgroundImageStyle, "%1: background image style changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundBrushStyle, Qt::BrushStyle, backgroundBrushStyle, updateBackground)
STD_SETTER_IMPL(Plot3D, BackgroundBrushStyle, Qt::BrushStyle, backgroundBrushStyle, "%1: background brush style changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundFirstColor, QColor, backgroundFirstColor, updateBackground)
STD_SETTER_IMPL(Plot3D, BackgroundFirstColor, const QColor&, backgroundFirstColor, "%1: set background first color")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundSecondColor, QColor, backgroundSecondColor, updateBackground)
STD_SETTER_IMPL(Plot3D, BackgroundSecondColor, const QColor&, backgroundSecondColor, "%1: set background second color")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundFileName, QString, backgroundFileName, updateBackground)
STD_SETTER_IMPL(Plot3D, BackgroundFileName, const QString&, backgroundFileName, "%1: set background image")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundOpacity, float, backgroundOpacity, updateBackground)
STD_SETTER_IMPL(Plot3D, BackgroundOpacity, float, backgroundOpacity, "%1: set opacity")

// Light
STD_SETTER_CMD_IMPL_F_S(Plot3D, SetIntensity, double, intensity, updateLight)
STD_SETTER_IMPL(Plot3D, Intensity, double, intensity, "%1: intensity changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetAmbient, QColor, ambient, updateLight)
STD_SETTER_IMPL(Plot3D, Ambient, const QColor&, ambient, "%1: ambient changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetDiffuse, QColor, diffuse, updateLight)
STD_SETTER_IMPL(Plot3D, Diffuse, const QColor&, diffuse, "%1: diffuse changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetSpecular, QColor, specular, updateLight)
STD_SETTER_IMPL(Plot3D, Specular, const QColor&, specular, "%1: specular changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetElevation, double, elevation, updateLight)
STD_SETTER_IMPL(Plot3D, Elevation, double, elevation, "%1: elevation changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetAzimuth, double, azimuth, updateLight)
STD_SETTER_IMPL(Plot3D, Azimuth, double, azimuth, "%1: azimuth changed")

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetConeAngle, double, coneAngle, updateLight)
STD_SETTER_IMPL(Plot3D, ConeAngle, double, coneAngle, "%1: coneAngle changed")

//##############################################################################
//######################### Private implementation #############################
//##############################################################################

Plot3DPrivate::Plot3DPrivate(Plot3D* owner)
	: AbstractPlotPrivate(owner)
	, q(owner)
	, context(0)
	, vtkItem(0)
	, isInitialized(false)
	, rectSet(false)
	, rangeBounds(-INFINITY, INFINITY, -INFINITY, INFINITY, -INFINITY, INFINITY)
	, axes(0)
	, xScaling(Plot3D::Scaling_Linear)
	, yScaling(Plot3D::Scaling_Linear)
	, zScaling(Plot3D::Scaling_Linear)
	, intensity(1.0)
	, ambient(Qt::white)
	, diffuse(Qt::white)
	, specular(Qt::white)
	, elevation(15)
	, azimuth(15)
	, coneAngle(15) {
	//read default settings
	KConfig config;
	KConfigGroup group = config.group("Plot3D");

	//general

	//background
	backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", (int) PlotArea::Color);
	backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", (int) PlotArea::SingleColor);
	backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", (int) PlotArea::Scaled);
	backgroundBrushStyle = (Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", (int) Qt::SolidPattern);
	backgroundFileName = group.readEntry("BackgroundFileName", QString());
	backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);
}

Plot3DPrivate::~Plot3DPrivate() {
}

void Plot3DPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	Q_UNUSED(event);
}

void Plot3DPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	Q_UNUSED(event);
}

namespace {
	// http://www.vtk.org/Wiki/VTK/Examples/Cxx/Qt/ImageDataToQImage
	QImage vtkImageDataToQImage(vtkImageData* imageData) {
		if (!imageData)
			return QImage();

		int width = imageData->GetDimensions()[0];
		int height = imageData->GetDimensions()[1];
		QImage image(width, height, QImage::Format_RGB32);
		QRgb* rgbPtr = reinterpret_cast<QRgb*>(image.bits()) + width * (height-1);
		unsigned char* colorsPtr = reinterpret_cast<unsigned char*>(imageData->GetScalarPointer());
		for(int row = 0; row < height; ++row) {
			for (int col = 0; col < width; ++col) {
				*(rgbPtr++) = QColor(colorsPtr[0], colorsPtr[1], colorsPtr[2]).rgb();
				colorsPtr +=  3;
			}
			rgbPtr -= width * 2;
		}
		return image;
	}
}

void Plot3DPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
	Q_UNUSED(option);
	Q_UNUSED(widget);
	if (!q->isVisible())
		return;

	if (m_printing){
		vtkSmartPointer<vtkWindowToImageFilter> wti = vtkSmartPointer<vtkWindowToImageFilter>::New();
		wti->SetInput(vtkItem->GetRenderWindow());
		wti->Update();
		vtkSmartPointer<vtkImageData> image = wti->GetOutput();
		const QPixmap pixmap(QPixmap::fromImage(vtkImageDataToQImage(image)));
		const double halfWidth = rect.width() / 2;
		const double halfHeight = rect.height() / 2;
		painter->drawPixmap(-halfWidth, -halfHeight, pixmap);
		painter->setPen(QPen(Qt::black));
		painter->drawRect(-halfWidth, -halfHeight, rect.width(), rect.height());
	}
}

void Plot3DPrivate::updateLight(bool notify) {
	vtkLight* lights[] = {lightAbove, lightBelow};
	for (int i = 0; i < 2; ++i) {
		lights[i]->SetIntensity(intensity);
		lights[i]->SetAmbientColor(ambient.redF(), ambient.greenF(), ambient.blueF());
		lights[i]->SetDiffuseColor(diffuse.redF(), diffuse.greenF(), diffuse.blueF());
		lights[i]->SetSpecularColor(specular.redF(), specular.greenF(), specular.blueF());

		lights[i]->SetDirectionAngle(elevation, azimuth);
		lights[i]->SetConeAngle(coneAngle);
	}

	if (notify)
		emit q->parametersChanged();
}

void Plot3DPrivate::initLights() {
	//light
	lightAbove = vtkSmartPointer<vtkLight>::New();
	lightBelow = vtkSmartPointer<vtkLight>::New();

	lightAbove->SetFocalPoint(1.876, 0.6125, 0);
	lightAbove->SetPosition(0.875, 1.6125, 1);

	lightBelow->SetFocalPoint(-1.876, -0.6125, 0);
	lightBelow->SetPosition(-0.875, -1.612, -1);
	renderer->AddLight(lightAbove);
	renderer->AddLight(lightBelow);
}

void Plot3DPrivate::init() {
	//initialize VTK
	vtkItem = new VTKGraphicsItem(context, this);
	vtkItem->connect(q, SIGNAL(parametersChanged()), SLOT(refresh()));

	//foreground renderer
	renderer = vtkSmartPointer<vtkRenderer>::New();

	//background renderer
	backgroundRenderer = vtkSmartPointer<vtkRenderer>::New();

	//render window and layers
	vtkGenericOpenGLRenderWindow* renderWindow = vtkItem->GetRenderWindow();
	renderWindow->SetNumberOfLayers(2);
	renderWindow->AddRenderer(backgroundRenderer);
	renderWindow->AddRenderer(renderer);

	backgroundRenderer->SetLayer(0);
	renderer->SetLayer(1);
	backgroundRenderer->InteractiveOff();

	vtkSmartPointer<MouseInteractor> style = vtkSmartPointer<MouseInteractor>::New();
	style->SetDefaultRenderer(renderer);
	q->connect(&style->broadcaster, SIGNAL(objectClicked(vtkProp*)), SLOT(onObjectClicked(vtkProp*)));
	q->connect(&style->broadcaster, SIGNAL(objectHovered(vtkProp*)), SLOT(onObjectHovered(vtkProp*)));
	renderWindow->GetInteractor()->SetInteractorStyle(style);

	initLights();

	backgroundImageActor = vtkSmartPointer<vtkImageActor>::New();
	backgroundRenderer->AddActor(backgroundImageActor);

	if (axes) {
		axes->setRenderer(renderer);
		q->configureAspect(axes);
	}

	foreach(Surface3D* surface, surfaces) {
		surface->setRenderer(renderer);
		q->configureAspect(surface);
	}

	foreach(Curve3D* curve, curves) {
		curve->setRenderer(renderer);
		q->configureAspect(curve);
	}
}

void Plot3DPrivate::setupCamera() {
	//set the background camera in front of the background image (fill the complete layer)
	vtkCamera* camera = backgroundRenderer->GetActiveCamera();
	camera->ParallelProjectionOn();
	const double x = rect.width() / 2;
	const double y = rect.height() / 2;
	camera->SetFocalPoint(x, y, 0.0);
	camera->SetParallelScale(y);
	camera->SetPosition(x, y, 900);
}

void Plot3DPrivate::retransform() {
	prepareGeometryChange();
	const double halfWidth = rect.width() / 2;
	const double halfHeight = rect.height() / 2;
	setPos(rect.x() + halfWidth, rect.y() + halfHeight);

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	q->plotArea()->setRect(rect);

	if (halfHeight < 0.1 || halfWidth < 0.1) {
		vtkItem->hide();
	} else {
		vtkItem->show();
		vtkItem->setGeometry(-halfWidth, -halfHeight, rect.width(), rect.height());

		setupCamera();
		updateBackground(false);
		updateLight();

		resetCamera();
	}

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}

void Plot3DPrivate::resetCamera() {
	renderer->ResetCamera();
	renderer->GetActiveCamera()->Azimuth(15);
}

void Plot3DPrivate::updateXScaling() {
	qDebug() << Q_FUNC_INFO;
	foreach (Surface3D* surface, surfaces) {
		surface->setXScaling(xScaling);
	}

	foreach (Curve3D* curve, curves) {
		curve->setXScaling(xScaling);
	}

	axes->setXScaling(xScaling);
}

void Plot3DPrivate::updateYScaling() {
	foreach (Surface3D* surface, surfaces) {
		surface->setYScaling(yScaling);
	}

	foreach (Curve3D* curve, curves) {
		curve->setYScaling(yScaling);
	}

	axes->setYScaling(yScaling);
}

void Plot3DPrivate::updateZScaling() {
	foreach (Surface3D* surface, surfaces) {
		surface->setZScaling(zScaling);
	}

	foreach (Curve3D* curve, curves) {
		curve->setZScaling(zScaling);
	}

	axes->setZScaling(zScaling);
}

void Plot3DPrivate::updateRange(bool notify) {
	qDebug() << Q_FUNC_INFO << __LINE__;
	foreach(Surface3D* surface, surfaces) {
		surface->setRange(rangeBounds);
	}

	foreach(Curve3D* curve, curves) {
		curve->setRange(rangeBounds);
	}

	axes->updateBounds();
	if (notify)
		emit q->parametersChanged();
}

void Plot3DPrivate::updateBackground(bool notify) {
	const QRectF rect(0, 0, this->rect.width(), this->rect.height());
	//prepare the image
	QImage image(rect.width(), rect.height(), QImage::Format_ARGB32_Premultiplied);
	double borderCornerRadius = 0.0; //TODO
	QPainter painter(&image);
	painter.setOpacity(backgroundOpacity);
	painter.setPen(Qt::NoPen);
	if (backgroundType == PlotArea::Color){
		switch (backgroundColorStyle){
			case PlotArea::SingleColor:{
				painter.setBrush(QBrush(backgroundFirstColor));
				break;
			}
			case PlotArea::HorizontalLinearGradient:{
				QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
				linearGrad.setColorAt(0, backgroundFirstColor);
				linearGrad.setColorAt(1, backgroundSecondColor);
				painter.setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::VerticalLinearGradient:{
				QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
				linearGrad.setColorAt(0, backgroundFirstColor);
				linearGrad.setColorAt(1, backgroundSecondColor);
				painter.setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::TopLeftDiagonalLinearGradient:{
				QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
				linearGrad.setColorAt(0, backgroundFirstColor);
				linearGrad.setColorAt(1, backgroundSecondColor);
				painter.setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::BottomLeftDiagonalLinearGradient:{
				QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
				linearGrad.setColorAt(0, backgroundFirstColor);
				linearGrad.setColorAt(1, backgroundSecondColor);
				painter.setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::RadialGradient:{
				QRadialGradient radialGrad(rect.center(), rect.width()/2);
				radialGrad.setColorAt(0, backgroundFirstColor);
				radialGrad.setColorAt(1, backgroundSecondColor);
				painter.setBrush(QBrush(radialGrad));
				break;
			}
		}
	}else if (backgroundType == PlotArea::Image){
		if ( !backgroundFileName.trimmed().isEmpty() ) {
			QPixmap pix(backgroundFileName);
			if (pix.width() > 0 && pix.height() > 0) {
				const QImage img(pix.toImage());
				painter.fillRect(rect, img.pixel(QPoint(0, 0)));
			} else {
				painter.fillRect(rect, Qt::white);
			}
			switch (backgroundImageStyle){
				case PlotArea::ScaledCropped:
					pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
					painter.setBrush(QBrush(pix));
					painter.setBrushOrigin(0, 0);
					painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
					break;
				case PlotArea::Scaled:
					pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
					painter.setBrush(QBrush(pix));
					painter.setBrushOrigin(0, 0);
					painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
					break;
				case PlotArea::ScaledAspectRatio:
					pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
					painter.setBrush(QBrush(pix));
					painter.setBrushOrigin(0, 0);
					painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
					break;
				case PlotArea::Centered:
					painter.drawPixmap(QPointF(rect.center().x()-pix.size().width()/2,rect.center().y()-pix.size().height()/2),pix);
					break;
				case PlotArea::Tiled:
					painter.setBrush(QBrush(pix));
					painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
					break;
				case PlotArea::CenterTiled:
					painter.setBrush(QBrush(pix));
					painter.setBrushOrigin(rect.center().x()-pix.size().width()/2, 0);
					painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
			}
		} else {
			painter.fillRect(rect, Qt::white);
		}
	} else if (backgroundType == PlotArea::Pattern){
		painter.fillRect(rect, Qt::white);
		painter.setBrush(QBrush(backgroundFirstColor, backgroundBrushStyle));
	}

	if ( qFuzzyIsNull(borderCornerRadius) )
		painter.drawRect(rect);
	else
		painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);

	//set the prepared image in the background actor
	vtkSmartPointer<vtkQImageToImageSource> qimageToImageSource = vtkSmartPointer<vtkQImageToImageSource>::New();
	qimageToImageSource->SetQImage(&image);

	backgroundImageActor->SetInputData(qimageToImageSource->GetOutput());
	qimageToImageSource->Update();
	if (notify)
		emit q->parametersChanged();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Plot3D::save(QXmlStreamWriter* writer) const {
	Q_D(const Plot3D);

	writer->writeStartElement("Plot3D");
		writeBasicAttributes(writer);
		writeCommentElement(writer);

		writer->writeStartElement("general");
			writer->writeAttribute("xScaling", QString::number(d->xScaling));
			writer->writeAttribute("yScaling", QString::number(d->yScaling));
			writer->writeAttribute("zScaling", QString::number(d->zScaling));
		writer->writeEndElement();

		writer->writeStartElement("background");
			writer->writeAttribute("type", QString::number(d->backgroundType));
			writer->writeAttribute("colorStyle", QString::number(d->backgroundColorStyle));
			writer->writeAttribute("imageStyle", QString::number(d->backgroundImageStyle));
			writer->writeAttribute("brushStyle", QString::number(d->backgroundBrushStyle));
			writer->writeAttribute("firstColor", d->backgroundFirstColor.name());
			writer->writeAttribute("secondColor", d->backgroundSecondColor.name());
			writer->writeAttribute("fileName", d->backgroundFileName);
			writer->writeAttribute("opacity", QString::number(d->backgroundOpacity));
		writer->writeEndElement();

		writer->writeStartElement("light");
			writer->writeAttribute("intensity", QString::number(d->intensity));
			writer->writeAttribute("ambient", d->ambient.name());
			writer->writeAttribute("diffuse", d->diffuse.name());
			writer->writeAttribute("specular", d->specular.name());
			writer->writeAttribute("elevation", QString::number(d->elevation));
			writer->writeAttribute("azimuth", QString::number(d->azimuth));
			writer->writeAttribute("coneAngle", QString::number(d->coneAngle));
		writer->writeEndElement();

		d->axes->save(writer);
		foreach(const Surface3D* surface, d->surfaces)
			surface->save(writer);
		
		foreach(const Curve3D* curve, d->curves)
			curve->save(writer);

	writer->writeEndElement();
}

//! Load from XML
bool Plot3D::load(XmlStreamReader* reader) {
	Q_D(Plot3D);
	if(!reader->isStartElement() || reader->name() != "Plot3D"){
		reader->raiseError(i18n("no Plot3D element found"));
		return false;
	}

	if(!readBasicAttributes(reader)){
		return false;
	}

	const QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");

	while(!reader->atEnd()){
		reader->readNext();
		const QStringRef& sectionName = reader->name();
		if (reader->isEndElement() && sectionName == "Plot3D")
			break;

		if (reader->isEndElement())
			continue;

		if(sectionName == "comment"){
			if(!readCommentElement(reader)){
				return false;
			}
		} else if(sectionName == "general"){
			const QXmlStreamAttributes& attribs = reader->attributes();
			XmlAttributeReader attributeReader(reader, attribs);
			attributeReader.checkAndLoadAttribute("xScaling", d->xScaling);
			attributeReader.checkAndLoadAttribute("yScaling", d->yScaling);
			attributeReader.checkAndLoadAttribute("zScaling", d->zScaling);
		}else if(sectionName == "axes"){
			qDebug() << Q_FUNC_INFO << "Load axes";
			if (!d->axes) {
				d->axes = new Axes;
				if(!d->axes->load(reader))
					return false;
			}
		}else if(sectionName == "surface3d"){
			qDebug() << Q_FUNC_INFO << "Load surface";
			Surface3D* newSurface = new Surface3D();
			newSurface->load(reader);
			// Scaling has been already initialized in the general section
			newSurface->setXScaling(d->xScaling);
			newSurface->setYScaling(d->yScaling);
			newSurface->setZScaling(d->zScaling);
			d->surfaces.insert(newSurface);
		}else if(sectionName == "curve3d"){
			qDebug() << Q_FUNC_INFO << "Load curve";
			Curve3D* newCurve = new Curve3D();
			newCurve->load(reader);
			newCurve->setXScaling(d->xScaling);
			newCurve->setYScaling(d->yScaling);
			newCurve->setZScaling(d->zScaling);
			d->curves.insert(newCurve);
		}else if(sectionName == "background"){
			const QXmlStreamAttributes& attribs = reader->attributes();
			XmlAttributeReader attributeReader(reader, attribs);
			attributeReader.checkAndLoadAttribute<PlotArea::BackgroundType>("type", d->backgroundType);
			attributeReader.checkAndLoadAttribute<PlotArea::BackgroundColorStyle>("colorStyle", d->backgroundColorStyle);
			attributeReader.checkAndLoadAttribute<PlotArea::BackgroundImageStyle>("imageStyle", d->backgroundImageStyle);
			attributeReader.checkAndLoadAttribute<Qt::BrushStyle>("brushStyle", d->backgroundBrushStyle);
			attributeReader.checkAndLoadAttribute("firstColor", d->backgroundFirstColor);
			attributeReader.checkAndLoadAttribute("secondColor", d->backgroundSecondColor);
			attributeReader.checkAndLoadAttribute("fileName", d->backgroundFileName);
			attributeReader.checkAndLoadAttribute("opacity", d->backgroundOpacity);
		}else if(sectionName == "light"){
			const QXmlStreamAttributes& attribs = reader->attributes();
			XmlAttributeReader attributeReader(reader, attribs);
			attributeReader.checkAndLoadAttribute("intensity", d->intensity);
			attributeReader.checkAndLoadAttribute("ambient", d->ambient);
			attributeReader.checkAndLoadAttribute("diffuse", d->diffuse);
			attributeReader.checkAndLoadAttribute("specular", d->specular);
			attributeReader.checkAndLoadAttribute("elevation", d->elevation);
			attributeReader.checkAndLoadAttribute("azimuth", d->azimuth);
			attributeReader.checkAndLoadAttribute("coneAngle", d->coneAngle);
		}
	}
	return true;
}
