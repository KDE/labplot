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
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QPainter>
#include <QWidget>
#include <QFileInfo>

#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

#include <QVTKGraphicsItem.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkImageActor.h>
#include <vtkLight.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkRendererCollection.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkTriangle.h>
#include <vtkCellArray.h>
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

void Plot3D::setFile(const KUrl& path){
	Q_D(Plot3D);
	d->path = path;
	d->updatePlot();
}

void Plot3D::setXColumn(AbstractColumn *column){
	qDebug() << Q_FUNC_INFO;
	Q_D(Plot3D);
	d->xColumn= column;
	d->updatePlot();
}

void Plot3D::setYColumn(AbstractColumn *column){
	qDebug() << Q_FUNC_INFO;
	Q_D(Plot3D);
	d->yColumn= column;
	d->updatePlot();
}

void Plot3D::setZColumn(AbstractColumn *column){
	qDebug() << Q_FUNC_INFO;
	Q_D(Plot3D);
	d->zColumn= column;
	d->updatePlot();
}

void Plot3D::setNodeColumn(int node, AbstractColumn* column){
	qDebug() << Q_FUNC_INFO;
	if (node >= 0 && node < 3){
		Q_D(Plot3D);
		d->nodeColumn[node] = column;
		d->updatePlot();
	}
}

void Plot3D::setShowAxes(bool show){
	Q_D(Plot3D);
	d->setShowAxes(show);
	d->updatePlot();
}

bool Plot3D::needAxes() const{
	Q_D(const Plot3D);
	return d->showAxes;
}

void Plot3D::setAxesType(Axes::AxesType type) {
	Q_D(Plot3D);
	d->axesProps.type = type;
	d->addAxes();
}

void Plot3D::setAxesFontSize(int size) {
	if (size < 0 || size > 72)
		// TODO: Error message
		return;

	Q_D(Plot3D);
	d->axesProps.fontSize = size;
	d->addAxes();
}

void Plot3D::setAxesColor(int axes, const QColor& color) {
	if (axes < 0 || axes > 2)
		// TODO: Error message
		return;

	Q_D(Plot3D);
	if (axes == 0)
		d->axesProps.xLabelColor = color;
	else if (axes == 1)
		d->axesProps.yLabelColor = color;
	else
		d->axesProps.zLabelColor = color;
	d->addAxes();
}

void Plot3D::setMatrix(Matrix* matrix){
	Q_D(Plot3D);
	qDebug() << Q_FUNC_INFO << matrix;
	d->matrix = matrix;
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
	, sourceType(Plot3D::DataSource_Empty)
	, showAxes(true)
	, xColumn(0)
	, yColumn(0)
	, zColumn(0)
	, matrix(0) {

	for (int i = 0; i < 3; ++i) {
		nodeColumn[i] = 0;
	}
}

Plot3DPrivate::~Plot3DPrivate() {
}

void Plot3DPrivate::init() {
	//initialize VTK
	vtkItem = new QVTKGraphicsItem(context, q->plotArea()->graphicsItem());

	//foreground renderer
	renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
	renderer->AddLight(light);

	//background renderer
	backgroundRenderer = vtkSmartPointer<vtkRenderer>::New();
	backgroundImageActor = vtkSmartPointer<vtkImageActor>::New();
	backgroundRenderer->AddActor(backgroundImageActor);

	//render window and layers
	vtkGenericOpenGLRenderWindow* renderWindow = vtkItem->GetRenderWindow();
	backgroundRenderer->SetLayer(0);
	backgroundRenderer->InteractiveOff();
	renderer->SetLayer(1);
	renderWindow->SetNumberOfLayers(2);
	renderWindow->AddRenderer(backgroundRenderer);
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	renderWindow->GetInteractor()->SetInteractorStyle(style);

	if (showAxes)
		addAxes();

	//background
// 	renderer->SetBackground(.3, .6, .3);

	//light
	light->SetFocalPoint(1.875, 0.6125, 0);
	light->SetPosition(0.875, 1.6125, 1);

	updateBackground();
}

void Plot3DPrivate::setShowAxes(bool show) {
	const bool prevState = showAxes;
	showAxes = show;
	if (!showAxes && axes){
		axes.reset();
	} else if (showAxes && !prevState){
		addAxes();
	}
}

void Plot3DPrivate::addSphere() {
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
	vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
	sphereActor->GetProperty()->SetFrontfaceCulling(true);
	sphereActor->SetMapper(sphereMapper);
	renderer->AddActor(sphereActor);
}

template<class TReader>
void Plot3DPrivate::createReader() {
	const QByteArray ascii = path.path().toAscii();
	const char *path = ascii.constData();
	vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
	reader->SetFileName(path);
	reader->Update();

	//reader fails to read obj-files if the locale is not set to 'C'
	setlocale (LC_NUMERIC,"C");

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(reader->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	renderer->AddActor(actor);
}

void Plot3DPrivate::readFromFile() {
	const QString& fileName = path.fileName();
	qDebug() << Q_FUNC_INFO << "Read from the file:" << path.path().toAscii();
	const QString& fileType = fileName.split('.').last().toLower();

	if (fileType == "obj"){
		qDebug() << Q_FUNC_INFO << "Create obj reader";
		createReader<vtkOBJReader>();
	} else if (fileType == "stl"){
		qDebug() << Q_FUNC_INFO << "Create STL reader";
		createReader<vtkSTLReader>();
	}
}

void Plot3DPrivate::addAxes() {
	if (showAxes)
		axes.reset(new Axes(*renderer, axesProps));
}

void Plot3DPrivate::readFromColumns() {
	if (xColumn == 0 || yColumn == 0 || zColumn == 0) {
		return;
	}

	for (int i = 0; i < 3; ++i)
		if (nodeColumn[i] == 0) {
			qDebug() << Q_FUNC_INFO << "Node" << i << "== 0";
			return;
		}

	if (visType == Plot3D::VisualizationType_Triangles) {
		qDebug() << Q_FUNC_INFO << "Triangles rendering";
		vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

		qDebug() << Q_FUNC_INFO << "Row count:" << xColumn->rowCount() << yColumn->rowCount() << zColumn->rowCount();

		const int numPoints = std::min(xColumn->rowCount(), std::min(yColumn->rowCount(), zColumn->rowCount()));
		for (int i = 0; i < numPoints; ++i) {
			const int x = static_cast<int>(xColumn->valueAt(i));
			const int y = static_cast<int>(yColumn->valueAt(i));
			const int z = static_cast<int>(zColumn->valueAt(i));

			points->InsertNextPoint(x, y, z);
		}

		const int numTrianges = std::min(nodeColumn[0]->rowCount(), std::min(nodeColumn[1]->rowCount(), nodeColumn[2]->rowCount()));
		for (int i = 0; i < numTrianges; ++i) {
			vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
			const int id1 = static_cast<int>(nodeColumn[0]->valueAt(i));
			const int id2 = static_cast<int>(nodeColumn[1]->valueAt(i));
			const int id3 = static_cast<int>(nodeColumn[2]->valueAt(i));

			if (id1 < 1 || id2 < 1 || id3 < 1 || id1 > numPoints || id2 > numPoints || id3 > numPoints)
				// TODO: Return error
				continue;

			triangle->GetPointIds()->SetId(0, id1);
			triangle->GetPointIds()->SetId(1, id2);
			triangle->GetPointIds()->SetId(2, id3);

			triangles->InsertNextCell(triangle);
		}

		renderTriangles(points, triangles);
	}
}

void Plot3DPrivate::renderTriangles(vtkSmartPointer<vtkPoints>& points,
		vtkSmartPointer<vtkCellArray>& triangles) {
	qDebug() << Q_FUNC_INFO << "Amount of triangles:" << triangles->GetSize();

	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

	polydata->SetPoints(points);
	polydata->SetPolys(triangles);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polydata);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	renderer->AddActor(actor);
}

void Plot3DPrivate::readFromMatrix() {
	qDebug() << Q_FUNC_INFO;
	if (!matrix)
		return;

	if (visType == Plot3D::VisualizationType_Triangles) {
		qDebug() << Q_FUNC_INFO << "Triangles rendering";
		vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

		const double deltaX = (matrix->xEnd() - matrix->xStart()) / matrix->columnCount();
		const double deltaY = (matrix->yEnd() - matrix->yStart()) / matrix->rowCount();
		QVector<QVector<vtkIdType> > cellPoints(matrix->columnCount(), QVector<vtkIdType>(matrix->rowCount()));
		for (int x = 0; x < matrix->columnCount(); ++x){
			for (int y = 0; y < matrix->rowCount(); ++y){
				const double x_val = matrix->xStart() + deltaX * x;
				const double y_val = matrix->yStart() + deltaY * y;
				const double z_val = matrix->cell(x, y);
				cellPoints[x][y] = points->InsertNextPoint(x_val, y_val, z_val);
			}
		}

		for (int x = 0, max_x = cellPoints.size() - 1; x < max_x; ++x) {
			for (int y = 0, max_y = cellPoints[0].size() - 1; y < max_y; ++y) {
				const vtkIdType rectPoints[4] = {cellPoints[x][y], cellPoints[x +1][y],
					cellPoints[x + 1][y + 1], cellPoints[x][y + 1]};

				vtkSmartPointer<vtkTriangle> triangle1 = vtkSmartPointer<vtkTriangle>::New();
				triangle1->GetPointIds()->SetId(0, rectPoints[0]);
				triangle1->GetPointIds()->SetId(1, rectPoints[1]);
				triangle1->GetPointIds()->SetId(2, rectPoints[2]);

				vtkSmartPointer<vtkTriangle> triangle2 = vtkSmartPointer<vtkTriangle>::New();
				triangle2->GetPointIds()->SetId(0, rectPoints[2]);
				triangle2->GetPointIds()->SetId(1, rectPoints[3]);
				triangle2->GetPointIds()->SetId(2, rectPoints[0]);

				triangles->InsertNextCell(triangle1);
				triangles->InsertNextCell(triangle2);
			}
		}

		renderTriangles(points, triangles);
	}
}

void Plot3DPrivate::retransform() {
	prepareGeometryChange();
	setPos(rect.x()+rect.width()/2, rect.y()+rect.height()/2);

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	vtkItem->setGeometry(q->plotArea()->rect());
	q->plotArea()->setRect(rect);

	updateBackground();
	updatePlot();

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}

void Plot3DPrivate::updatePlot() {
	//clear all the available actors
	vtkActorCollection* actors = renderer->GetActors();
	actors->InitTraversal();

	vtkActor* actor = 0;
	while ((actor = actors->GetNextActor()) != 0) {
		if (axes == 0 || *axes != actor)
			renderer->RemoveActor(actor);
	}

	//render the plot
	if (sourceType == Plot3D::DataSource_Empty) {
		qDebug() << Q_FUNC_INFO << "Add Sphere";
		addSphere();
	} else if (sourceType == Plot3D::DataSource_File) {
		qDebug() << Q_FUNC_INFO << "Read file";
		readFromFile();
	} else if (sourceType == Plot3D::DataSource_Spreadsheet) {
		readFromColumns();
	} else if (sourceType == Plot3D::DataSource_Matrix) {
		readFromMatrix();
	}

	if (axes)
		axes->updateBounds();
}

void Plot3DPrivate::updateBackground() {
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
		painter.setBrush(QBrush(backgroundFirstColor,backgroundBrushStyle));
	}

	if ( qFuzzyIsNull(borderCornerRadius) )
		painter.drawRect(rect);
	else
		painter.drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);

	//set the prepared image in the background actor
	vtkSmartPointer<vtkImageData> imageData;
	vtkSmartPointer<vtkQImageToImageSource> qimageToImageSource = vtkSmartPointer<vtkQImageToImageSource>::New();
	qimageToImageSource->SetQImage(&image);
	qimageToImageSource->Update();
	imageData = qimageToImageSource->GetOutput();
	backgroundImageActor->SetInputData(imageData);

	//TODO how to update?
	vtkItem->GetRenderWindow()->Render();
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
			writer->writeAttribute("show_axes", QString::number(d->showAxes));
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
			else{
				d->showAxes = static_cast<bool>(showAxesAttr.toInt());
				qDebug() << Q_FUNC_INFO << "show_axes == " << d->showAxes;
			}

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
