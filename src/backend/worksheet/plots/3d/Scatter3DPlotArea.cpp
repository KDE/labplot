#include "Scatter3DPlotArea.h"
#include "Axis3D.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include <QXmlStreamAttributes>

#include "backend/worksheet/plots/3d/Scatter3DPlotAreaPrivate.h"
class Scatter3DPlotArea;
class Scatter3DPlotAreaPrivate;
Scatter3DPlotArea::Scatter3DPlotArea(const QString& name)
	: WorksheetElementContainer(name, new Scatter3DPlotAreaPrivate(this), AspectType::SurfacePlot)
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

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetXColumn, const AbstractColumn*, xColumn, generateData)
void Scatter3DPlotArea::setXColumn(const AbstractColumn* xCol) {
	Q_D(Scatter3DPlotArea);
	if (xCol != d->xColumn)
		exec(new Scatter3DPlotAreaSetXColumnCmd(d, xCol, ki18n("%1: X Column changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetYColumn, const AbstractColumn*, yColumn, generateData)
void Scatter3DPlotArea::setYColumn(const AbstractColumn* yCol) {
	Q_D(Scatter3DPlotArea);
	if (yCol != d->yColumn)
		exec(new Scatter3DPlotAreaSetYColumnCmd(d, yCol, ki18n("%1: Y Column changed")));
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DPlotArea, SetZColumn, const AbstractColumn*, zColumn, generateData)
void Scatter3DPlotArea::setZColumn(const AbstractColumn* zCol) {
	Q_D(Scatter3DPlotArea);
	if (zCol != d->zColumn)
		exec(new Scatter3DPlotAreaSetZColumnCmd(d, zCol, ki18n("%1: Z Column changed")));
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
class Surface3DPlotAreaSetPrevRectCmd : public QUndoCommand {
public:
	Surface3DPlotAreaSetPrevRectCmd(Scatter3DPlotAreaPrivate* private_obj, const QRectF& rect)
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
	exec(new Surface3DPlotAreaSetPrevRectCmd(d, prevRect));
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
void Scatter3DPlotAreaPrivate::generateData() {
	if (xColumn == nullptr || yColumn == nullptr || zColumn == nullptr)
		return;

	qDebug() << Q_FUNC_INFO << "Columns have been set";

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
}
