#include "Scatter3DPlotArea.h"
#include "Axis3D.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include <QXmlStreamAttributes>

#include "backend/worksheet/plots/3d/Scatter3DPlotAreaPrivate.h"
class Scatter3DPlotArea;
class Scatter3DPlotAreaPrivate;
Scatter3DPlotArea::Scatter3DPlotArea(const QString& name)
	: WorksheetElementContainer(name, new Scatter3DPlotAreaPrivate(this), AspectType::Scatter3DPlot)
	, m_scatter{new Q3DScatter()} {
	Axis3D* xAxis = new Axis3D(QStringLiteral("x-axis"), Axis3D::X);
	Axis3D* yAxis = new Axis3D(QStringLiteral("y-axis"), Axis3D::Y);
	Axis3D* zAxis = new Axis3D(QStringLiteral("z-axis"), Axis3D::Z);
	addChild(xAxis);
	addChild(yAxis);
	addChild(zAxis);
	m_scatter->setAxisX(xAxis->m_axis);
	m_scatter->setAxisY(yAxis->m_axis);
	m_scatter->setAxisZ(zAxis->m_axis);
}

Scatter3DPlotArea::~Scatter3DPlotArea() {
}

void Scatter3DPlotArea::save(QXmlStreamWriter*) const {
}

bool Scatter3DPlotArea::load(XmlStreamReader*, bool preview) {
	return true;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

// Spreadsheet parameters
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, const AbstractColumn*, zColumn, zColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, QString, xColumnPath, xColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, QString, yColumnPath, yColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, QString, zColumnPath, zColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, Scatter3DPlotArea::Theme, theme, theme)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, Scatter3DPlotArea::PointStyle, pointStyle, pointStyle)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, QColor, color, color)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, Scatter3DPlotArea::ShadowQuality, shadowQuality, shadowQuality)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, double, opacity, opacity)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, int, xRotation, xRotation)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, int, yRotation, yRotation)
BASIC_SHARED_D_READER_IMPL(Scatter3DPlotArea, int, zoomLevel, zoomLevel)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetXColumn, const AbstractColumn*, xColumn, recalc)
void Scatter3DPlotArea::setXColumn(const AbstractColumn* xCol) {
	Q_D(Scatter3DPlotArea);
	if (xCol != d->xColumn)
		exec(new Scatter3DPlotAreaSetXColumnCmd(d, xCol, ki18n("%1: X Column changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetYColumn, const AbstractColumn*, yColumn, recalc)
void Scatter3DPlotArea::setYColumn(const AbstractColumn* yCol) {
	Q_D(Scatter3DPlotArea);
	if (yCol != d->yColumn)
		exec(new Scatter3DPlotAreaSetYColumnCmd(d, yCol, ki18n("%1: Y Column changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetZColumn, const AbstractColumn*, zColumn, recalc)
void Scatter3DPlotArea::setZColumn(const AbstractColumn* zCol) {
	Q_D(Scatter3DPlotArea);
	if (zCol != d->zColumn)
		exec(new Scatter3DPlotAreaSetZColumnCmd(d, zCol, ki18n("%1: Z Column changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetTheme, Scatter3DPlotArea::Theme, theme, updateTheme)
void Scatter3DPlotArea::setTheme(Scatter3DPlotArea::Theme theme) {
	Q_D(Scatter3DPlotArea);
	if (theme != d->theme)
		exec(new Scatter3DPlotAreaSetThemeCmd(d, theme, ki18n("%1: theme changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetPointStyle, Scatter3DPlotArea::PointStyle, pointStyle, updatePointStyle)
void Scatter3DPlotArea::setPointStyle(Scatter3DPlotArea::PointStyle pointStyle) {
	Q_D(Scatter3DPlotArea);
	if (pointStyle != d->pointStyle)
		exec(new Scatter3DPlotAreaSetPointStyleCmd(d, pointStyle, ki18n("%1: point style changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetShadowQuality, Scatter3DPlotArea::ShadowQuality, shadowQuality, updateShadowQuality)
void Scatter3DPlotArea::setShadowQuality(Scatter3DPlotArea::ShadowQuality shadowQuality) {
	Q_D(Scatter3DPlotArea);
	if (shadowQuality != d->shadowQuality)
		exec(new Scatter3DPlotAreaSetShadowQualityCmd(d, shadowQuality, ki18n("%1: shadow quality changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetColor, QColor, color, updateColor)
void Scatter3DPlotArea::setColor(QColor color) {
	Q_D(Scatter3DPlotArea);
	if (color != d->color)
		exec(new Scatter3DPlotAreaSetColorCmd(d, color, ki18n("%1: color changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetOpacity, double, opacity, updateOpacity)
void Scatter3DPlotArea::setOpacity(double opacity) {
	Q_D(Scatter3DPlotArea);
	if (opacity != d->opacity)
		exec(new Scatter3DPlotAreaSetOpacityCmd(d, opacity, ki18n("%1: opacity changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetXRotation, int, xRotation, updateXRotation)
void Scatter3DPlotArea::setXRotation(int value) {
	Q_D(Scatter3DPlotArea);
	if (value != d->xRotation)
		exec(new Scatter3DPlotAreaSetXRotationCmd(d, value, ki18n("%1: X Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetYRotation, int, yRotation, updateYRotation)
void Scatter3DPlotArea::setYRotation(int value) {
	Q_D(Scatter3DPlotArea);
	if (value != d->xRotation)
		exec(new Scatter3DPlotAreaSetYRotationCmd(d, value, ki18n("%1: X Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetZoomLevel, int, zoomLevel, updateZoomLevel)
void Scatter3DPlotArea::setZoomLevel(int value) {
	Q_D(Scatter3DPlotArea);
	if (value != d->zoomLevel)
		exec(new Scatter3DPlotAreaSetZoomLevelCmd(d, value, ki18n("%1: Zoom Level changed")));
}
class Scatter3DPlotAreaSetRectCmd : public QUndoCommand {
public:
	Scatter3DPlotAreaSetRectCmd(Scatter3DPlotAreaPrivate* private_obj, const QRectF& rect)
		: m_private(private_obj)
		, m_rect(rect) {
		setText(i18n("%1: change geometry rect", m_private->name()));
	}

	void redo() override {
		// 		const double horizontalRatio = m_rect.width() / m_private->rect.width();
		// 		const double verticalRatio = m_rect.height() / m_private->rect.height();

		qSwap(m_private->rect, m_rect);

		// 		m_private->q->handleResize(horizontalRatio, verticalRatio, false);
		m_private->retransform();
		Q_EMIT m_private->q->rectChanged(m_private->rect);
	}

	void undo() override {
		redo();
	}

private:
	Scatter3DPlotAreaPrivate* m_private;
	QRectF m_rect;
};
void Scatter3DPlotArea::setRect(const QRectF& rect) {
	Q_D(Scatter3DPlotArea);
	if (rect != d->rect)
		exec(new Scatter3DPlotAreaSetRectCmd(d, rect));
}
class Scatter3DPlotAreaSetPrevRectCmd : public QUndoCommand {
public:
	Scatter3DPlotAreaSetPrevRectCmd(Scatter3DPlotAreaPrivate* private_obj, const QRectF& rect)
		: m_private(private_obj)
		, m_rect(rect) {
		setText(i18n("%1: change geometry rect", m_private->name()));
	}

	void redo() override {
		if (m_initilized) {
			qSwap(m_private->rect, m_rect);
			m_private->retransform();
			Q_EMIT m_private->q->rectChanged(m_private->rect);
		} else {
			// this function is called for the first time,
			// nothing to do, we just need to remember what the previous rect was
			// which has happened already in the constructor.
			m_initilized = true;
		}
	}

	void undo() override {
		redo();
	}

private:
	Scatter3DPlotAreaPrivate* m_private;
	QRectF m_rect;
	bool m_initilized{false};
};

void Scatter3DPlotArea::setPrevRect(const QRectF& prevRect) {
	Q_D(Scatter3DPlotArea);
	exec(new Scatter3DPlotAreaSetPrevRectCmd(d, prevRect));
}

void Scatter3DPlotArea::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DPlotArea);
	d->xColumn = nullptr;
}

void Scatter3DPlotArea::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DPlotArea);
	d->yColumn = nullptr;
}
void Scatter3DPlotArea::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DPlotArea);
	d->zColumn = nullptr;
}

void Scatter3DPlotArea::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}

void Scatter3DPlotArea::retransform() {
	Q_D(Scatter3DPlotArea);
	d->retransform();
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Scatter3DPlotAreaPrivate::Scatter3DPlotAreaPrivate(Scatter3DPlotArea* owner)
	: WorksheetElementContainerPrivate(owner)
	, theme(Scatter3DPlotArea::Qt)
	, pointStyle(Scatter3DPlotArea::Sphere)
	, shadowQuality(Scatter3DPlotArea::None)
	, q(owner) {
}

void Scatter3DPlotAreaPrivate::retransform() {
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
void Scatter3DPlotAreaPrivate::recalcShapeAndBoundingRect() {
}
void Scatter3DPlotAreaPrivate::recalc() {
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

void Scatter3DPlotAreaPrivate::updateTheme() {
	q->m_scatter->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
	q->m_scatter->update();
	Q_EMIT q->changed();
}

void Scatter3DPlotAreaPrivate::updatePointStyle() {
	QScatter3DSeries* series = q->m_scatter->seriesList().at(0);
	if (!series)
		return;

	switch (pointStyle) {
	case Scatter3DPlotArea::Sphere:
		series->setMesh(QAbstract3DSeries::Mesh::Sphere);
		break;
	case Scatter3DPlotArea::Cube:
		series->setMesh(QAbstract3DSeries::Mesh::Cube);
		break;
	case Scatter3DPlotArea::Cone:
		series->setMesh(QAbstract3DSeries::Mesh::Cone);
		break;
	case Scatter3DPlotArea::Pyramid:
		series->setMesh(QAbstract3DSeries::Mesh::Pyramid);
		break;
	default:
		series->setMesh(QAbstract3DSeries::Mesh::Sphere);
		break;
	}
	q->m_scatter->update();
	Q_EMIT q->changed();
}

void Scatter3DPlotAreaPrivate::updateShadowQuality() {
	q->m_scatter->setShadowQuality(static_cast<QAbstract3DGraph::ShadowQuality>(shadowQuality));
	q->m_scatter->update();
	Q_EMIT q->changed();
}

void Scatter3DPlotAreaPrivate::updateColor() {
	QScatter3DSeries* series = q->m_scatter->seriesList().at(0);
	if (!series)
		return;
	series->setBaseColor(color);
	q->m_scatter->update();
	Q_EMIT q->changed();
}
void Scatter3DPlotAreaPrivate::updateOpacity() {
	QScatter3DSeries* series = q->m_scatter->seriesList().at(0);
	if (!series)
		return;
	QColor baseColor = series->baseColor();
	baseColor.setAlphaF(opacity);
	series->setBaseColor(baseColor);
	q->m_scatter->update();
	Q_EMIT q->changed();
}

void Scatter3DPlotAreaPrivate::updateXRotation() {
	q->m_scatter->setCameraXRotation(xRotation);
	Q_EMIT q->changed();
}

void Scatter3DPlotAreaPrivate::updateYRotation() {
	q->m_scatter->setCameraYRotation(yRotation);
	Q_EMIT q->changed();
}

void Scatter3DPlotAreaPrivate::updateZoomLevel() {
	q->m_scatter->setCameraZoomLevel(zoomLevel);
	Q_EMIT q->changed();
}
