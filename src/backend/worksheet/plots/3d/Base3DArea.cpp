#include "Base3DArea.h"

#include "Base3DAreaPrivate.h"
#include "backend/lib/commandtemplates.h"

Base3DArea::Base3DArea(const QString& name, Base3DAreaPrivate* dd, Base3DArea::Type baseType, AspectType type)
	: WorksheetElementContainer(name, dd, type) {
	Q_D(Base3DArea);
	d->type = baseType;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

BASIC_SHARED_D_READER_IMPL(Base3DArea, Base3DArea::Theme, theme, theme)
BASIC_SHARED_D_READER_IMPL(Base3DArea, Base3DArea::ShadowQuality, shadowQuality, shadowQuality)
BASIC_SHARED_D_READER_IMPL(Base3DArea, int, xRotation, xRotation)
BASIC_SHARED_D_READER_IMPL(Base3DArea, int, yRotation, yRotation)
BASIC_SHARED_D_READER_IMPL(Base3DArea, int, zoomLevel, zoomLevel)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

STD_SETTER_CMD_IMPL_F_S(Base3DArea, SetShadowQuality, Base3DArea::ShadowQuality, shadowQuality, updateShadowQuality)
void Base3DArea::setShadowQuality(Base3DArea::ShadowQuality shadowQuality) {
	Q_D(Base3DArea);
	if (shadowQuality != d->shadowQuality)
		exec(new Base3DAreaSetShadowQualityCmd(d, shadowQuality, ki18n("%1: shadow quality changed")));
}

STD_SETTER_CMD_IMPL_F_S(Base3DArea, SetTheme, Base3DArea::Theme, theme, updateTheme)
void Base3DArea::setTheme(Base3DArea::Theme value) {
	Q_D(Base3DArea);
	if (value != d->theme)
		exec(new Base3DAreaSetThemeCmd(d, value, ki18n("%1: theme changed")));
}
STD_SETTER_CMD_IMPL_F_S(Base3DArea, SetXRotation, int, xRotation, updateXRotation)
void Base3DArea::setXRotation(int value) {
	Q_D(Base3DArea);
	if (value != d->xRotation)
		exec(new Base3DAreaSetXRotationCmd(d, value, ki18n("%1: X Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Base3DArea, SetYRotation, int, yRotation, updateYRotation)
void Base3DArea::setYRotation(int value) {
	Q_D(Base3DArea);
	if (value != d->yRotation)
		exec(new Base3DAreaSetYRotationCmd(d, value, ki18n("%1: Y Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Base3DArea, SetZoomLevel, int, zoomLevel, updateZoomLevel)
void Base3DArea::setZoomLevel(int value) {
	Q_D(Base3DArea);
	if (value != d->zoomLevel)
		exec(new Base3DAreaSetZoomLevelCmd(d, value, ki18n("%1: zoom changed")));
}

class Base3DAreaSetRectCmd : public QUndoCommand {
public:
	Base3DAreaSetRectCmd(Base3DAreaPrivate* private_obj, const QRectF& rect)
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
	Base3DAreaPrivate* m_private;
	QRectF m_rect;
};
void Base3DArea::setRect(const QRectF& rect) {
	Q_D(Base3DArea);
	if (rect != d->rect)
		exec(new Base3DAreaSetRectCmd(d, rect));
}
class Base3DAreaSetPrevRectCmd : public QUndoCommand {
public:
	Base3DAreaSetPrevRectCmd(Base3DAreaPrivate* private_obj, const QRectF& rect)
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
	Base3DAreaPrivate* m_private;
	QRectF m_rect;
	bool m_initilized{false};
};

void Base3DArea::setPrevRect(const QRectF& prevRect) {
	Q_D(Base3DArea);
	exec(new Base3DAreaSetPrevRectCmd(d, prevRect));
}

void Base3DArea::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}
void Base3DArea::retransform() {
	Q_D(Base3DArea);
	d->retransform();
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Base3DAreaPrivate::Base3DAreaPrivate(Base3DArea* owner)
	: WorksheetElementContainerPrivate(owner)
	, xRotation(90)
	, yRotation(0)
	, theme(Base3DArea::Qt)
	, zoomLevel(100)
	, shadowQuality(Base3DArea::Medium)
	, q(owner) {
}

void Base3DAreaPrivate::recalcShapeAndBoundingRect() {
}
void Base3DAreaPrivate::retransform() {
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

void Base3DAreaPrivate::updateTheme() {
	switch (type) {
	case Base3DArea::Type::Surface:
		q->m_surface->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
		break;
	case Base3DArea::Type::Scatter:
		q->m_scatter->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
		break;
	case Base3DArea::Type::Bar:
		q->m_bar->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
		break;
	default:
		break;
	}

	Q_EMIT q->changed();
}

void Base3DAreaPrivate::updateZoomLevel() {
	switch (type) {
	case Base3DArea::Type::Surface:
		q->m_surface->setCameraZoomLevel(zoomLevel);
		q->m_surface->scene()->needRender();
		break;
	case Base3DArea::Type::Scatter:
		q->m_scatter->setCameraZoomLevel(zoomLevel);
		q->m_scatter->scene()->needRender();
		break;
	case Base3DArea::Type::Bar:
		q->m_bar->setCameraZoomLevel(zoomLevel);
		q->m_bar->scene()->needRender();
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}

void Base3DAreaPrivate::updateShadowQuality() {
	switch (type) {
	case Base3DArea::Type::Surface:
		q->m_surface->setShadowQuality(static_cast<QAbstract3DGraph::ShadowQuality>(shadowQuality));
		break;
	case Base3DArea::Type::Scatter:
		q->m_scatter->setShadowQuality(static_cast<QAbstract3DGraph::ShadowQuality>(shadowQuality));
		break;
	case Base3DArea::Type::Bar:
		q->m_bar->setShadowQuality(static_cast<QAbstract3DGraph::ShadowQuality>(shadowQuality));
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
void Base3DAreaPrivate::updateXRotation() {
	switch (type) {
	case Base3DArea::Type::Surface:
		q->m_surface->setCameraXRotation(xRotation);
		q->m_surface->scene()->needRender();
		break;
	case Base3DArea::Type::Scatter:
		q->m_scatter->setCameraXRotation(xRotation);
		q->m_scatter->scene()->needRender();
		break;
	case Base3DArea::Type::Bar:
		q->m_bar->setCameraXRotation(xRotation);
		q->m_bar->scene()->needRender();
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
void Base3DAreaPrivate::updateYRotation() {
	switch (type) {
	case Base3DArea::Type::Surface:
		q->m_surface->setCameraYRotation(yRotation);
		q->m_surface->scene()->needRender();
		break;
	case Base3DArea::Type::Scatter:
		q->m_scatter->setCameraYRotation(yRotation);
		q->m_scatter->scene()->needRender();
		break;
	case Base3DArea::Type::Bar:
		q->m_bar->setCameraYRotation(yRotation);
		q->m_bar->scene()->needRender();
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
