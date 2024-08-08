#include "Plot3DArea.h"

#include "Plot3DAreaPrivate.h"
#include "backend/lib/commandtemplates.h"

Plot3DArea::Plot3DArea(const QString& name, Plot3DAreaPrivate* dd, AspectType type)
	: WorksheetElementContainer(name, dd, type)
	, d_ptr(dd) {
}
// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

BASIC_SHARED_D_READER_IMPL(Plot3DArea, Plot3DArea::Theme, theme, theme)
BASIC_SHARED_D_READER_IMPL(Plot3DArea, Plot3DArea::ShadowQuality, shadowQuality, shadowQuality)
BASIC_SHARED_D_READER_IMPL(Plot3DArea, int, xRotation, xRotation)
BASIC_SHARED_D_READER_IMPL(Plot3DArea, int, yRotation, yRotation)
BASIC_SHARED_D_READER_IMPL(Plot3DArea, int, zoomLevel, zoomLevel)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

STD_SETTER_CMD_IMPL_F_S(Plot3DArea, SetShadowQuality, Plot3DArea::ShadowQuality, shadowQuality, updateShadowQuality)
void Plot3DArea::setShadowQuality(Plot3DArea::ShadowQuality shadowQuality) {
	Q_D(Plot3DArea);
	if (shadowQuality != d->shadowQuality)
		exec(new Plot3DAreaSetShadowQualityCmd(d, shadowQuality, ki18n("%1: shadow quality changed")));
}

STD_SETTER_CMD_IMPL_F_S(Plot3DArea, SetTheme, Plot3DArea::Theme, theme, updateTheme)
void Plot3DArea::setTheme(Plot3DArea::Theme value) {
	Q_D(Plot3DArea);
	if (value != d->theme)
		exec(new Plot3DAreaSetThemeCmd(d, value, ki18n("%1: theme changed")));
}
STD_SETTER_CMD_IMPL_F_S(Plot3DArea, SetXRotation, int, xRotation, updateXRotation)
void Plot3DArea::setXRotation(int value) {
	Q_D(Plot3DArea);
	if (value != d->xRotation)
		exec(new Plot3DAreaSetXRotationCmd(d, value, ki18n("%1: X Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Plot3DArea, SetYRotation, int, yRotation, updateYRotation)
void Plot3DArea::setYRotation(int value) {
	Q_D(Plot3DArea);
	if (value != d->yRotation)
		exec(new Plot3DAreaSetYRotationCmd(d, value, ki18n("%1: Y Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Plot3DArea, SetZoomLevel, int, zoomLevel, updateZoomLevel)
void Plot3DArea::setZoomLevel(int value) {
	Q_D(Plot3DArea);
	if (value != d->zoomLevel)
		exec(new Plot3DAreaSetZoomLevelCmd(d, value, ki18n("%1: zoom changed")));
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

void Plot3DArea::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}
void Plot3DArea::retransform() {
	Q_D(Plot3DArea);
	d->retransform();
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Plot3DAreaPrivate::Plot3DAreaPrivate(Plot3DArea* owner, Plot3DArea::Type type)
	: WorksheetElementContainerPrivate(owner)
	, xRotation(90)
	, yRotation(0)
	, theme(Plot3DArea::Qt)
	, zoomLevel(100)
	, shadowQuality(Plot3DArea::Medium)
	, q(owner) {
	this->type = type;
}

void Plot3DAreaPrivate::recalcShapeAndBoundingRect() {
}
void Plot3DAreaPrivate::retransform() {
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

void Plot3DAreaPrivate::updateTheme() {
	switch (type) {
	case Plot3DArea::Type::Surface:
		q->m_surface->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
		break;
	case Plot3DArea::Type::Scatter:
		q->m_scatter->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
		break;
	case Plot3DArea::Type::Bar:
		q->m_bar->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
		break;
	default:
		break;
	}

	Q_EMIT q->changed();
}

void Plot3DAreaPrivate::updateZoomLevel() {
	switch (type) {
	case Plot3DArea::Type::Surface:
		q->m_surface->setCameraZoomLevel(zoomLevel);
		break;
	case Plot3DArea::Type::Scatter:
		q->m_scatter->setCameraZoomLevel(zoomLevel);
		break;
	case Plot3DArea::Type::Bar:
		q->m_bar->setCameraZoomLevel(zoomLevel);
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}

void Plot3DAreaPrivate::updateShadowQuality() {
	switch (type) {
	case Plot3DArea::Type::Surface:
		q->m_surface->setShadowQuality(static_cast<QAbstract3DGraph::ShadowQuality>(shadowQuality));
		break;
	case Plot3DArea::Type::Scatter:
		q->m_scatter->setShadowQuality(static_cast<QAbstract3DGraph::ShadowQuality>(shadowQuality));
		break;
	case Plot3DArea::Type::Bar:
		q->m_bar->setShadowQuality(static_cast<QAbstract3DGraph::ShadowQuality>(shadowQuality));
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
void Plot3DAreaPrivate::updateXRotation() {
	switch (type) {
	case Plot3DArea::Type::Surface:
		q->m_surface->setCameraXRotation(xRotation);
		break;
	case Plot3DArea::Type::Scatter:
		q->m_scatter->setCameraXRotation(xRotation);
		break;
	case Plot3DArea::Type::Bar:
		q->m_bar->setCameraXRotation(xRotation);
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
void Plot3DAreaPrivate::updateYRotation() {
	switch (type) {
	case Plot3DArea::Type::Surface:
		q->m_surface->setCameraYRotation(yRotation);
		break;
	case Plot3DArea::Type::Scatter:
		q->m_scatter->setCameraYRotation(yRotation);
		break;
	case Plot3DArea::Type::Bar:
		q->m_bar->setCameraYRotation(yRotation);
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
