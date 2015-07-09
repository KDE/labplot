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
#include "backend/lib/commandtemplates.h"
#include "backend/core/AbstractColumn.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/matrix/Matrix.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"

#include <QDebug>
#include <QPainter>
#include <QWidget>

#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

#include <QVTKGraphicsItem.h>
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


Plot3D::Plot3D(const QString& name, QGLContext *context)
	: AbstractPlot(name, new Plot3DPrivate(this, context)){
	qDebug() << Q_FUNC_INFO;
	init();
}

Plot3D::Plot3D(const QString &name, Plot3DPrivate *dd)
	: AbstractPlot(name, dd){
	qDebug() << Q_FUNC_INFO;
	init();
}

void Plot3D::init(){
	Q_D(Plot3D);

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

	foreach (IDataHandler* handler, d->dataHandlers) {
		connect(handler, SIGNAL(parametersChanged()), this, SLOT(updatePlot()));
	}
}

void Plot3D::updatePlot() {
	Q_D(Plot3D);
	d->updatePlot();
}

Plot3D::~Plot3D(){

}

QIcon Plot3D::icon() const{
	// TODO: Replace by some 3D chart
	return KIcon("office-chart-line");
}

QMenu* Plot3D::createContextMenu(){
	QMenu* menu = WorksheetElement::createContextMenu();
	//TODO

	return menu;
}

void Plot3D::setRect(const QRectF &rect){
	Q_D(Plot3D);
	d->rect = rect;
	d->retransform();
}

void Plot3D::setVisualizationType(VisualizationType type){
	Q_D(Plot3D);
	d->visType = type;
	d->updatePlot();
}

Plot3D::VisualizationType Plot3D::visualizationType() const{
	Q_D(const Plot3D);
	return d->visType;
}

void Plot3D::setDataSource(DataSource source){
	Q_D(Plot3D);
	d->sourceType = source;
	d->updatePlot();
}

Plot3D::DataSource Plot3D::dataSource() const{
	Q_D(const Plot3D);
	return d->sourceType;
}

DemoDataHandler& Plot3D::demoDataHandler() {
	Q_D(Plot3D);
	return d->demoHandler;
}

SpreadsheetDataHandler& Plot3D::spreadsheetDataHandler() {
	Q_D(Plot3D);
	return d->spreadsheetHandler;
}

MatrixDataHandler& Plot3D::matrixDataHandler() {
	Q_D(Plot3D);
	return d->matrixHandler;
}

FileDataHandler& Plot3D::fileDataHandler() {
	Q_D(Plot3D);
	return d->fileHandler;
}

Axes& Plot3D::axes() {
	Q_D(Plot3D);
	return *d->axes;
}

void Plot3D::retransform(){
	Q_D(Plot3D);
	d->retransform();
	WorksheetElementContainer::retransform();

	//TODO:do we really need to trigger an update in the view?
// 	Worksheet* w = dynamic_cast<Worksheet*>(this->parentAspect());
// 	if (w)
// 		w->view()->update();
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

Plot3DPrivate::Plot3DPrivate(Plot3D* owner, QGLContext *context)
	: AbstractPlotPrivate(owner)
	, q(owner)
	, context(context)
	, visType(Plot3D::VisualizationType_Triangles)
	, sourceType(Plot3D::DataSource_File) {
}

Plot3DPrivate::~Plot3DPrivate() {
}

void Plot3DPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	QGraphicsItem::mousePressEvent(event);
}

void Plot3DPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	QGraphicsItem::mouseReleaseEvent(event);
}

namespace {
		class VTKGraphicsItem : public QVTKGraphicsItem {
		public:
			VTKGraphicsItem(QGLContext* ctx, QGraphicsItem* p)
				: QVTKGraphicsItem(ctx, p) {
			}

		protected:
			void mousePressEvent(QGraphicsSceneMouseEvent* event) {
				dynamic_cast<Plot3DPrivate*>(parentItem())->mousePressEvent(event);
				QVTKGraphicsItem::mousePressEvent(event);
			}

			void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
				dynamic_cast<Plot3DPrivate*>(parentItem())->mouseReleaseEvent(event);
				QVTKGraphicsItem::mouseReleaseEvent(event);
			}
	};
}

void Plot3DPrivate::init() {
	//initialize VTK
	vtkItem = new VTKGraphicsItem(context, this);

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

	axes.reset(new Axes(*renderer));

	//light
	light->SetFocalPoint(1.875, 0.6125, 0);
	light->SetPosition(0.875, 1.6125, 1);

	backgroundImageActor = vtkSmartPointer<vtkImageActor>::New();
	backgroundRenderer->AddActor(backgroundImageActor);

	updateBackground();

	dataHandlers.reserve(Plot3D::DataSource_MAX);
	dataHandlers[Plot3D::DataSource_Empty] = &demoHandler;
	dataHandlers[Plot3D::DataSource_File] = &fileHandler;
	dataHandlers[Plot3D::DataSource_Spreadsheet] = &spreadsheetHandler;
	dataHandlers[Plot3D::DataSource_Matrix] = &matrixHandler;
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
		camera->SetPosition(x,y,camera->GetDistance());
		updateBackground();
		updatePlot();
	}

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}

void Plot3DPrivate::updatePlot() {
	//clear all the available actors
	vtkActorCollection* actors = renderer->GetActors();
	actors->InitTraversal();

	vtkActor* actor = 0;
	while ((actor = actors->GetNextActor()) != 0) {
		if (*axes != actor)
			renderer->RemoveActor(actor);
	}

	//render the plot
	renderer->AddActor(dataHandlers[sourceType]->actor(visType));

	axes->updateBounds();
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
			switch (backgroundImageStyle){
				case PlotArea::ScaledCropped:
					pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
					painter.setBrush(QBrush(pix));
					painter.setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
					painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
					break;
				case PlotArea::Scaled:
					pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
					painter.setBrush(QBrush(pix));
					painter.setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
					painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
					break;
				case PlotArea::ScaledAspectRatio:
					pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
					painter.setBrush(QBrush(pix));
					painter.setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
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
					painter.setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
					painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
			}
		}
	} else if (backgroundType == PlotArea::Pattern){
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
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Plot3D::save(QXmlStreamWriter* writer) const {
	qDebug() << Q_FUNC_INFO;
	Q_D(const Plot3D);

	writer->writeStartElement("Plot3D");
		writeBasicAttributes(writer);
		writeCommentElement(writer);

		writer->writeStartElement("general");
			writer->writeAttribute("show_axes", QString::number(d->axes->isShown()));
			writer->writeAttribute("vis_type", QString::number(d->visType));
			if (d->visType == VisualizationType_Triangles){
				writer->writeAttribute("data_source", QString::number(d->sourceType));
			}
		writer->writeEndElement();
	writer->writeEndElement();
}

//! Load from XML
bool Plot3D::load(XmlStreamReader* reader) {
	qDebug() << Q_FUNC_INFO;
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
			else
				d->axes->show(static_cast<bool>(showAxesAttr.toInt()));

			const QString& visTypeAttr = attribs.value("vis_type").toString();
			if (!visTypeAttr.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'vis_type'"));
			else{
				d->visType = static_cast<VisualizationType>(visTypeAttr.toInt());
				qDebug() << Q_FUNC_INFO << "vis_type == " << d->visType;
			}

			const QString& dataSourceAttr = attribs.value("data_source").toString();
			if (!dataSourceAttr.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'data_source'"));
			else{
				d->sourceType = static_cast<DataSource>(dataSourceAttr.toInt());
			}
		}
	}
	return true;
}
