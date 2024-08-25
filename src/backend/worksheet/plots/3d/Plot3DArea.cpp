#include "Plot3DArea.h"
#include "backend/worksheet/plots/3d/Plot3DAreaPrivate.h"
#include "backend/worksheet/plots/PlotArea.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

Plot3DArea::Plot3DArea(QString& name)
	: WorksheetElementContainer(name, new Plot3DAreaPrivate(this), AspectType::Plot3DArea) {
}

void Plot3DArea::init(bool transform) {
	Q_D(Plot3DArea);
	if (d->isInitialized)
		return;

	qDebug() << Q_FUNC_INFO << "Init";

	auto m_plotArea = new PlotArea(name() + " plot area", this);
	addChild(m_plotArea);

	initActions();

	d->init();
	d->isInitialized = true;

	initMenus();

	if (transform)
		retransform();
}

void Plot3DArea::retransform() {
	Q_D(Plot3DArea);

	if (!d->isInitialized)
		init(false);

	d->retransform();
	WorksheetElementContainer::retransform();
}

void Plot3DArea::initActions() {
	Q_D(Plot3DArea);
	//"add new" actions
	addSurfaceAction = new QAction(QIcon(QStringLiteral("3d-surface")), i18n("3D-surface"), this);
	addScatterAction= new QAction(QIcon(QStringLiteral("3d-scatter")), i18n("3D-scatter"), this);
	addBarAction = new QAction(QIcon(QStringLiteral("3d-bar")), i18n("3D-bar"), this);

	connect(addSurfaceAction, SIGNAL(triggered()), SLOT(addSurface()));
	connect(addScatterAction, SIGNAL(triggered()), SLOT(addScatter()));
	connect(addBarAction, SIGNAL(triggered()), SLOT(addBar()));
}
void Plot3DArea::initMenus() {
	addNewMenu = new QMenu(i18n("Add new"));
	addNewMenu->addAction(addSurfaceAction);
	addNewMenu->addAction(addScatterAction);
	addNewMenu->addAction(addBarAction);
}

void Plot3DArea::configureAspect(AbstractAspect* aspect) {
}

void Plot3DArea::addScatter() {
	Q_D(Plot3DArea);
	Scatter3DPlot* newScatter = new Scatter3DPlot(i18n("Scatter 3D Plot"));
	d->scatters.insert(newScatter);
}

void Plot3DArea::addBar() {
	Q_D(Plot3DArea);
	Bar3DPlot* newBar = new Bar3DPlot(i18n("Surface 3D Plot"));
	d->bars.insert(newBar);
}
void Plot3DArea::addSurface() {
	Q_D(Plot3DArea);
	Surface3DPlot* newSurface = new Surface3DPlot(i18n("Surface 3D Plot"));

	d->surfaces.insert(newSurface);
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

Plot3DAreaPrivate::Plot3DAreaPrivate(Plot3DArea* owner)
	: WorksheetElementContainerPrivate(owner)
	, q(owner) {
}

void Plot3DAreaPrivate::init() {
	// TODO setup connect objects
}

void Plot3DAreaPrivate::retransform() {
	if (!isInitialized)
		return;

	prepareGeometryChange();
	const double halfWidth = rect.width() / 2;
	const double halfHeight = rect.height() / 2;
	setPos(rect.x() + halfWidth, rect.y() + halfHeight);
	q->setRect(rect);

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();

	q->WorksheetElementContainer::retransform();

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}
