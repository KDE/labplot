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
#include "Axes.h"
#include "DataHandlers.h"
#include "VTKGraphicsItem.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/Worksheet.h"

#include <QDebug>
#include <QPainter>
#include <QMenu>

#include <KAction>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkLight.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkQImageToImageSource.h>


Plot3D::Plot3D(const QString& name)
	: AbstractPlot(name, new Plot3DPrivate(this)){
}

void Plot3D::init(bool transform){
	Q_D(Plot3D);
	if (d->isInitialized)
		return;

	if (!d->rectSet || d->context == 0)
		return;

	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);

	//read default settings
	KConfig config;
	KConfigGroup group = config.group("Plot3D");

	//general

	//background
	d->backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", (int) PlotArea::Color);
	d->backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", (int) PlotArea::SingleColor);
	d->backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", (int) PlotArea::Scaled);
	d->backgroundBrushStyle = (Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", (int) Qt::SolidPattern);
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);

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

void Plot3D::updatePlot() {
	Q_D(Plot3D);
	d->updatePlot();
}

Plot3D::~Plot3D(){
}

QIcon Plot3D::icon() const {
	// TODO: Replace by some 3D chart
	return KIcon("office-chart-line");
}

void Plot3D::addSurface() {
	Q_D(Plot3D);
	Surface3D* newSurface = new Surface3D(*d->renderer);
	d->surfaces.append(newSurface);
	addChild(newSurface);
	d->vtkItem->connect(newSurface, SIGNAL(parametersChanged()), SLOT(refresh()));
	d->vtkItem->refresh();
}

void Plot3D::initActions() {
	Q_D(Plot3D);
	//"add new" actions
	addCurveAction = new KAction(KIcon("3d-curve"), i18n("3D-curve"), this);
	addEquationCurveAction = new KAction(KIcon("3d-equation-curve"), i18n("3D-curve from a mathematical equation"), this);
	addSurfaceAction = new KAction(KIcon("3d-surface"), i18n("3D-surface"), this);

// 	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
// 	connect(addEquationCurveAction, SIGNAL(triggered()), SLOT(addEquationCurve()));
 	connect(addSurfaceAction, SIGNAL(triggered()), SLOT(addSurface()));

	showAxesAction = new KAction(i18n("Axes"), this);
	showAxesAction->setCheckable(true);
	showAxesAction->setChecked(d->axes->isVisible());
	connect(showAxesAction, SIGNAL(toggled(bool)), d->axes, SLOT(show(bool)));

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

// 	connect(scaleAutoAction, SIGNAL(triggered()), SLOT(scaleAuto()));
// 	connect(scaleAutoXAction, SIGNAL(triggered()), SLOT(scaleAutoX()));
// 	connect(scaleAutoYAction, SIGNAL(triggered()), SLOT(scaleAutoY()));
// 	connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));
// 	connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));
// 	connect(zoomInXAction, SIGNAL(triggered()), SLOT(zoomInX()));
// 	connect(zoomOutXAction, SIGNAL(triggered()), SLOT(zoomOutX()));
// 	connect(zoomInYAction, SIGNAL(triggered()), SLOT(zoomInY()));
// 	connect(zoomOutYAction, SIGNAL(triggered()), SLOT(zoomOutY()));
// 	connect(shiftLeftXAction, SIGNAL(triggered()), SLOT(shiftLeftX()));
// 	connect(shiftRightXAction, SIGNAL(triggered()), SLOT(shiftRightX()));
// 	connect(shiftUpYAction, SIGNAL(triggered()), SLOT(shiftUpY()));
// 	connect(shiftDownYAction, SIGNAL(triggered()), SLOT(shiftDownY()));

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

void Plot3D::initMenus(){
	addNewMenu = new QMenu(i18n("Add new"));
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
	zoomMenu->addAction(zoomOutZAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInYAction);
	zoomMenu->addAction(zoomOutYAction);
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

	menu->insertAction(firstAction, showAxesAction);

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

BASIC_SHARED_D_READER_IMPL(Plot3D, PlotArea::BackgroundType, backgroundType, backgroundType)
BASIC_SHARED_D_READER_IMPL(Plot3D, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle)
BASIC_SHARED_D_READER_IMPL(Plot3D, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle)
BASIC_SHARED_D_READER_IMPL(Plot3D, Qt::BrushStyle, backgroundBrushStyle, backgroundBrushStyle)
CLASS_SHARED_D_READER_IMPL(Plot3D, QColor, backgroundFirstColor, backgroundFirstColor)
CLASS_SHARED_D_READER_IMPL(Plot3D, QColor, backgroundSecondColor, backgroundSecondColor)
CLASS_SHARED_D_READER_IMPL(Plot3D, QString, backgroundFileName, backgroundFileName)
BASIC_SHARED_D_READER_IMPL(Plot3D, float, backgroundOpacity, backgroundOpacity)


//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundType, PlotArea::BackgroundType, backgroundType, updateBackground)
void Plot3D::setBackgroundType(PlotArea::BackgroundType type) {
	Q_D(Plot3D);
	if (type != d->backgroundType)
		exec(new Plot3DSetBackgroundTypeCmd(d, type, i18n("%1: background type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundColorStyle, PlotArea::BackgroundColorStyle, backgroundColorStyle, updateBackground)
void Plot3D::setBackgroundColorStyle(PlotArea::BackgroundColorStyle style) {
	Q_D(Plot3D);
	if (style != d->backgroundColorStyle)
		exec(new Plot3DSetBackgroundColorStyleCmd(d, style, i18n("%1: background color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundImageStyle, PlotArea::BackgroundImageStyle, backgroundImageStyle, updateBackground)
void Plot3D::setBackgroundImageStyle(PlotArea::BackgroundImageStyle style) {
	Q_D(Plot3D);
	if (style != d->backgroundImageStyle)
		exec(new Plot3DSetBackgroundImageStyleCmd(d, style, i18n("%1: background image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundBrushStyle, Qt::BrushStyle, backgroundBrushStyle, updateBackground)
void Plot3D::setBackgroundBrushStyle(Qt::BrushStyle style) {
	Q_D(Plot3D);
	if (style != d->backgroundBrushStyle)
		exec(new Plot3DSetBackgroundBrushStyleCmd(d, style, i18n("%1: background brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundFirstColor, QColor, backgroundFirstColor, updateBackground)
void Plot3D::setBackgroundFirstColor(const QColor &color) {
	Q_D(Plot3D);
	if (color!= d->backgroundFirstColor)
		exec(new Plot3DSetBackgroundFirstColorCmd(d, color, i18n("%1: set background first color")));
}

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundSecondColor, QColor, backgroundSecondColor, updateBackground)
void Plot3D::setBackgroundSecondColor(const QColor &color) {
	Q_D(Plot3D);
	if (color!= d->backgroundSecondColor)
		exec(new Plot3DSetBackgroundSecondColorCmd(d, color, i18n("%1: set background second color")));
}

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundFileName, QString, backgroundFileName, updateBackground)
void Plot3D::setBackgroundFileName(const QString& fileName) {
	Q_D(Plot3D);
	if (fileName!= d->backgroundFileName)
		exec(new Plot3DSetBackgroundFileNameCmd(d, fileName, i18n("%1: set background image")));
}

STD_SETTER_CMD_IMPL_F_S(Plot3D, SetBackgroundOpacity, float, backgroundOpacity, updateBackground)
void Plot3D::setBackgroundOpacity(float opacity) {
	Q_D(Plot3D);
	if (opacity != d->backgroundOpacity)
		exec(new Plot3DSetBackgroundOpacityCmd(d, opacity, i18n("%1: set opacity")));
}

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
	, axes(0) {
}

Plot3DPrivate::~Plot3DPrivate() {
	q->removeChild(axes);
	axes = 0;

	foreach (Surface3D* surface, surfaces) {
		q->removeChild(surface);
	}
}

void Plot3DPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	QGraphicsItem::mousePressEvent(event);
}

void Plot3DPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	QGraphicsItem::mouseReleaseEvent(event);
}

void Plot3DPrivate::init() {
	//initialize VTK
	vtkItem = new VTKGraphicsItem(context, this);
	vtkItem->connect(q, SIGNAL(parametersChanged()), SLOT(refresh()));

	//foreground renderer
	renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
	renderer->AddLight(light);

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

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	renderWindow->GetInteractor()->SetInteractorStyle(style);

	//light
	light->SetFocalPoint(1.875, 0.6125, 0);
	light->SetPosition(0.875, 1.6125, 1);

	backgroundImageActor = vtkSmartPointer<vtkImageActor>::New();
	backgroundRenderer->AddActor(backgroundImageActor);

	// TODO: Configure as a separate widget
	axes = new Axes(*renderer);
	q->addChild(axes);
	axes->setHidden(true);
	vtkItem->connect(axes, SIGNAL(parametersChanged()), SLOT(refresh()));
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

		//set the background camera in front of the background image (fill the complete layer)
		vtkCamera* camera = backgroundRenderer->GetActiveCamera();
		camera->ParallelProjectionOn();
		const double x = rect.width() / 2;
		const double y = rect.height() / 2;
		camera->SetFocalPoint(x, y, 0.0);
		camera->SetParallelScale(y);
		camera->SetPosition(x,y, 900);
		updateBackground();
		updatePlot();
	}

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}

void Plot3DPrivate::updatePlot() {
	if (axes) {
		axes->updateBounds();
		emit q->parametersChanged();
	}
}

void Plot3DPrivate::updateBackground() {
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
			writer->writeAttribute("show_axes", QString::number(d->axes->isVisible()));
		writer->writeEndElement();
	writer->writeEndElement();
}

//! Load from XML
bool Plot3D::load(XmlStreamReader* reader) {
	Q_D(Plot3D);

	if(!reader->isStartElement() || reader->name() != "Plot3D"){
		reader->raiseError(i18n("no Plot3D element found"));
		qDebug() << Q_FUNC_INFO << __LINE__;
		return false;
	}

	if(!readBasicAttributes(reader)){
		qDebug() << Q_FUNC_INFO << __LINE__;
		return false;
	}

	const QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");

	while(!reader->atEnd()){
		reader->readNext();
		const QStringRef& sectionName = reader->name();
		if(reader->isEndElement() && sectionName == "Plot3D")
			break;

		if(sectionName == "comment"){
			if(!readCommentElement(reader)){
				qDebug() << Q_FUNC_INFO << __LINE__;
				return false;
			}
		}else if(sectionName == "general"){
			const QXmlStreamAttributes& attribs = reader->attributes();
			const QString& showAxesAttr = attribs.value("show_axes").toString();
			if(!showAxesAttr.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'show_axes'"));
			else {
				// TODO: Implement
			}
		}
	}
	return true;
}
