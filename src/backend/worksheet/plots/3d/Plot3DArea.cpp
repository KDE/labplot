#include "Plot3DArea.h"
#include "backend/worksheet/plots/3d/Plot3DAreaPrivate.h"
#include "backend/worksheet/plots/PlotArea.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QUndoCommand>

Plot3DArea::Plot3DArea(const QString& name)
	: AbstractPlot(name, new Plot3DAreaPrivate(this), AspectType::Plot3DArea) {
}

void Plot3DArea::init(bool transform) {
	Q_D(Plot3DArea);
	if (d->isInitialized)
		return;

	qDebug() << Q_FUNC_INFO << "Init";

	// auto m_plotArea = new PlotArea(name() + QLatin1String(" plot area"), this);
	// addChild(m_plotArea);

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

void Plot3DArea::save(QXmlStreamWriter*) const {
}
bool Plot3DArea::load(XmlStreamReader*, bool preview) {
	return true;
}

class Plot3DAreaSetRectCmd : public QUndoCommand {
public:
	Plot3DAreaSetRectCmd(Plot3DAreaPrivate* private_obj, const QRectF& rect)
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
	Plot3DAreaPrivate* m_private;
	QRectF m_rect;
};
void Plot3DArea::setRect(const QRectF& rect) {
	Q_D(Plot3DArea);
	if (rect != d->rect)
		exec(new Plot3DAreaSetRectCmd(d, rect));
}

QMenu* Plot3DArea::createContextMenu() {
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1);
	menu->insertMenu(firstAction, addNewMenu);
	return menu;
}

class Plot3DAreaSetPrevRectCmd : public QUndoCommand {
public:
	Plot3DAreaSetPrevRectCmd(Plot3DAreaPrivate* private_obj, const QRectF& rect)
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
	Plot3DAreaPrivate* m_private;
	QRectF m_rect;
	bool m_initilized{false};
};

void Plot3DArea::setPrevRect(const QRectF& prevRect) {
	Q_D(Plot3DArea);
	exec(new Plot3DAreaSetPrevRectCmd(d, prevRect));
}

void Plot3DArea::initActions() {
	//"add new" actions
	addSurfaceAction = new QAction(QIcon(QStringLiteral("3d-surface")), i18n("3D-surface"), this);
	addScatterAction = new QAction(QIcon(QStringLiteral("3d-scatter")), i18n("3D-scatter"), this);
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
	addChild(newScatter);
}

void Plot3DArea::addBar() {
	Q_D(Plot3DArea);
	Bar3DPlot* newBar = new Bar3DPlot(i18n("Surface 3D Plot"));
	d->bars.insert(newBar);
	addChild(newBar);
}
void Plot3DArea::addSurface() {
	Q_D(Plot3DArea);
	Surface3DPlot* newSurface = new Surface3DPlot(i18n("Surface 3D Plot"));
	d->surfaces.insert(newSurface);
	addChild(newSurface);
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

Plot3DAreaPrivate::Plot3DAreaPrivate(Plot3DArea* owner)
	: AbstractPlotPrivate(owner)
	, q(owner)
	, isInitialized(false){
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

void Plot3DAreaPrivate::recalcShapeAndBoundingRect() {
}
