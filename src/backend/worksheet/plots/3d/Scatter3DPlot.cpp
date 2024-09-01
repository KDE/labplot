#include "Scatter3DPlot.h"
#include "Axis3D.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include <QXmlStreamAttributes>

#include "backend/worksheet/plots/3d/Scatter3DPlotPrivate.h"

#include "MouseInteractor.h"
class Scatter3DPlot;
class Scatter3DPlotPrivate;
Scatter3DPlot::Scatter3DPlot(const QString& name)
	: Base3DArea(name, new Scatter3DPlotPrivate(this), Base3DArea::Scatter, AspectType::Scatter3DPlot) {
	m_scatter = new Q3DScatter();
	Axis3D* xAxis = new Axis3D(QStringLiteral("x-axis"), Axis3D::X);
	Axis3D* yAxis = new Axis3D(QStringLiteral("y-axis"), Axis3D::Y);
	Axis3D* zAxis = new Axis3D(QStringLiteral("z-axis"), Axis3D::Z);
	addChild(xAxis);
	addChild(yAxis);
	addChild(zAxis);
	m_scatter->setAxisX(xAxis->m_axis);
	m_scatter->setAxisY(yAxis->m_axis);
	m_scatter->setAxisZ(zAxis->m_axis);
	m_scatter->setActiveInputHandler(new MouseInteractor());
}

Scatter3DPlot::~Scatter3DPlot() {
}

void Scatter3DPlot::save(QXmlStreamWriter*) const {
}

bool Scatter3DPlot::load(XmlStreamReader*, bool preview) {
	return true;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

// Spreadsheet parameters
BASIC_SHARED_D_READER_IMPL(Scatter3DPlot, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlot, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlot, const AbstractColumn*, zColumn, zColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlot, QString, xColumnPath, xColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlot, QString, yColumnPath, yColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlot, QString, zColumnPath, zColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlot, Scatter3DPlot::PointStyle, pointStyle, pointStyle)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlot, QColor, color, color)
// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlot, SetXColumn, const AbstractColumn*, xColumn, recalc)
void Scatter3DPlot::setXColumn(const AbstractColumn* xCol) {
	Q_D(Scatter3DPlot);
	if (xCol != d->xColumn)
		exec(new Scatter3DPlotSetXColumnCmd(d, xCol, ki18n("%1: X Column changed")));
	if (xCol) {
		connect(xCol, &AbstractColumn::dataChanged, this, &Scatter3DPlot::recalc);
		if (xCol->parentAspect())
			connect(xCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Scatter3DPlot::xColumnAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlot, SetYColumn, const AbstractColumn*, yColumn, recalc)
void Scatter3DPlot::setYColumn(const AbstractColumn* yCol) {
	Q_D(Scatter3DPlot);
	if (yCol != d->yColumn)
		exec(new Scatter3DPlotSetYColumnCmd(d, yCol, ki18n("%1: Y Column changed")));
	if (yCol) {
		connect(yCol, &AbstractColumn::dataChanged, this, &Scatter3DPlot::recalc);
		if (yCol->parentAspect())
			connect(yCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Scatter3DPlot::yColumnAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlot, SetZColumn, const AbstractColumn*, zColumn, recalc)
void Scatter3DPlot::setZColumn(const AbstractColumn* zCol) {
	Q_D(Scatter3DPlot);
	if (zCol != d->zColumn)
		exec(new Scatter3DPlotSetZColumnCmd(d, zCol, ki18n("%1: Z Column changed")));
	if (zCol) {
		connect(zCol, &AbstractColumn::dataChanged, this, &Scatter3DPlot::recalc);
		if (zCol->parentAspect())
			connect(zCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Scatter3DPlot::zColumnAboutToBeRemoved);
	}
}

STD_SETTER_CMD_IMPL_F_S(Scatter3DPlot, SetPointStyle, Scatter3DPlot::PointStyle, pointStyle, updatePointStyle)
void Scatter3DPlot::setPointStyle(Scatter3DPlot::PointStyle pointStyle) {
	Q_D(Scatter3DPlot);
	if (pointStyle != d->pointStyle)
		exec(new Scatter3DPlotSetPointStyleCmd(d, pointStyle, ki18n("%1: point style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Scatter3DPlot, SetColor, QColor, color, updateColor)
void Scatter3DPlot::setColor(QColor color) {
	Q_D(Scatter3DPlot);
	if (color != d->color)
		exec(new Scatter3DPlotSetColorCmd(d, color, ki18n("%1: color changed")));
}

void Scatter3DPlot::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DPlot);
	d->xColumn = nullptr;
}

void Scatter3DPlot::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DPlot);
	d->yColumn = nullptr;
}
void Scatter3DPlot::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DPlot);
	d->zColumn = nullptr;
}

void Scatter3DPlot::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}

void Scatter3DPlot::retransform() {
	Q_D(Scatter3DPlot);
	d->retransform();
}

void Scatter3DPlot::recalc(){
	Q_D(Scatter3DPlot);
	d->recalc();
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Scatter3DPlotPrivate::Scatter3DPlotPrivate(Scatter3DPlot* owner)
	: Base3DAreaPrivate(owner)
	, pointStyle(Scatter3DPlot::Sphere)
	, color(Qt::green)
	, q(owner) {
}

void Scatter3DPlotPrivate::retransform() {
	const bool suppress = suppressRetransform || q->isLoading();
	trackRetransformCalled(suppress);

	if (suppress)
		return;

	prepareGeometryChange();
	setPos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);

	q->setRect(rect);

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();

	q->WorksheetElementContainer::retransform();
}
void Scatter3DPlotPrivate::recalcShapeAndBoundingRect() {
}
void Scatter3DPlotPrivate::recalc() {
	if (xColumn == nullptr || yColumn == nullptr || zColumn == nullptr)
		return;

	qDebug() << Q_FUNC_INFO << "Columns have been set";
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	const int numPoints = std::min(xColumn->availableRowCount(), std::min(yColumn->availableRowCount(), zColumn->availableRowCount()));
	if (numPoints == 0)
		return;

	QScatterDataArray* data = new QScatterDataArray;
	data->resize(numPoints);

	QScatterDataItem* ptrToDataArray = &data->first();
	for (int i = 0; i < numPoints; ++i) {
		const float x = static_cast<float>(xColumn->valueAt(i));
		const float y = static_cast<float>(yColumn->valueAt(i));
		const float z = static_cast<float>(zColumn->valueAt(i));

		ptrToDataArray->setPosition(QVector3D(x, y, z));
		++ptrToDataArray;
	}

	QScatter3DSeries* series = new QScatter3DSeries;
	series->dataProxy()->resetArray(*data);
	q->m_scatter->addSeries(series);
	Q_EMIT q->changed();
}
void Scatter3DPlotPrivate::updateColor() {
	auto* series = q->m_scatter->seriesList().first();
	if (!series)
		return;
	series->setBaseColor(color);
	Q_EMIT q->changed();
}

void Scatter3DPlotPrivate::updatePointStyle() {
	QScatter3DSeries* series = q->m_scatter->seriesList().at(0);
	if (!series)
		return;

	switch (pointStyle) {
	case Scatter3DPlot::Sphere:
		series->setMesh(QAbstract3DSeries::Mesh::Sphere);
		break;
	case Scatter3DPlot::Cube:
		series->setMesh(QAbstract3DSeries::Mesh::Cube);
		break;
	case Scatter3DPlot::Cone:
		series->setMesh(QAbstract3DSeries::Mesh::Cone);
		break;
	case Scatter3DPlot::Pyramid:
		series->setMesh(QAbstract3DSeries::Mesh::Pyramid);
		break;
	default:
		series->setMesh(QAbstract3DSeries::Mesh::Sphere);
		break;
	}
	q->m_scatter->update();
	Q_EMIT q->changed();
}
