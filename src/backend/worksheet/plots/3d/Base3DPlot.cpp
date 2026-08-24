#include "Base3DPlot.h"
#include "Base3DPlotPrivate.h"
#include "backend/lib/commandtemplates.h"

#include <QtGraphsWidgets/Q3DBarsWidgetItem>
#include <QtGraphsWidgets/Q3DScatterWidgetItem>
#include <QtGraphsWidgets/Q3DSurfaceWidgetItem>

Base3DPlot::Base3DPlot(const QString& name, Base3DPlotPrivate* dd, Base3DPlot::Type baseType, AspectType type)
	: WorksheetElementContainer(name, dd, type) {
	Q_D(Base3DPlot);
	d->type = baseType;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

BASIC_SHARED_D_READER_IMPL(Base3DPlot, Base3DPlot::Theme, theme, theme)
BASIC_SHARED_D_READER_IMPL(Base3DPlot, Base3DPlot::ShadowQuality, shadowQuality, shadowQuality)
BASIC_SHARED_D_READER_IMPL(Base3DPlot, int, xRotation, xRotation)
BASIC_SHARED_D_READER_IMPL(Base3DPlot, int, yRotation, yRotation)
BASIC_SHARED_D_READER_IMPL(Base3DPlot, int, zoomLevel, zoomLevel)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

STD_SETTER_CMD_IMPL_F_S(Base3DPlot, SetShadowQuality, Base3DPlot::ShadowQuality, shadowQuality, updateShadowQuality)
void Base3DPlot::setShadowQuality(Base3DPlot::ShadowQuality shadowQuality) {
	Q_D(Base3DPlot);
	if (shadowQuality != d->shadowQuality)
		exec(new Base3DPlotSetShadowQualityCmd(d, shadowQuality, ki18n("%1: shadow quality changed")));
}

STD_SETTER_CMD_IMPL_F_S(Base3DPlot, SetTheme, Base3DPlot::Theme, theme, updateTheme)
void Base3DPlot::setTheme(Base3DPlot::Theme value) {
	Q_D(Base3DPlot);
	if (value != d->theme)
		exec(new Base3DPlotSetThemeCmd(d, value, ki18n("%1: theme changed")));
}
STD_SETTER_CMD_IMPL_F_S(Base3DPlot, SetXRotation, int, xRotation, updateXRotation)
void Base3DPlot::setXRotation(int value) {
	Q_D(Base3DPlot);
	if (value != d->xRotation)
		exec(new Base3DPlotSetXRotationCmd(d, value, ki18n("%1: X Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Base3DPlot, SetYRotation, int, yRotation, updateYRotation)
void Base3DPlot::setYRotation(int value) {
	Q_D(Base3DPlot);
	if (value != d->yRotation)
		exec(new Base3DPlotSetYRotationCmd(d, value, ki18n("%1: Y Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Base3DPlot, SetZoomLevel, int, zoomLevel, updateZoomLevel)
void Base3DPlot::setZoomLevel(int value) {
	Q_D(Base3DPlot);
	if (value != d->zoomLevel)
		exec(new Base3DPlotSetZoomLevelCmd(d, value, ki18n("%1: zoom changed")));
}

class Base3DPlotSetRectCmd : public QUndoCommand {
public:
	Base3DPlotSetRectCmd(Base3DPlotPrivate* private_obj, const QRectF& rect)
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
	Base3DPlotPrivate* m_private;
	QRectF m_rect;
};
void Base3DPlot::setRect(const QRectF& rect) {
	Q_D(Base3DPlot);
	if (rect != d->rect)
		exec(new Base3DPlotSetRectCmd(d, rect));
}
class Base3DPlotSetPrevRectCmd : public QUndoCommand {
public:
	Base3DPlotSetPrevRectCmd(Base3DPlotPrivate* private_obj, const QRectF& rect)
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
	Base3DPlotPrivate* m_private;
	QRectF m_rect;
	bool m_initilized{false};
};

void Base3DPlot::setPrevRect(const QRectF& prevRect) {
	Q_D(Base3DPlot);
	exec(new Base3DPlotSetPrevRectCmd(d, prevRect));
}

void Base3DPlot::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}
void Base3DPlot::retransform() {
	Q_D(Base3DPlot);
	d->retransform();
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Base3DPlotPrivate::Base3DPlotPrivate(Base3DPlot* owner)
	: WorksheetElementContainerPrivate(owner)
	, xRotation(90)
	, yRotation(0)
	, theme(Base3DPlot::Qt)
	, zoomLevel(100)
	, shadowQuality(Base3DPlot::Medium)
	, q(owner) {
}

void Base3DPlotPrivate::recalcShapeAndBoundingRect() {
}
void Base3DPlotPrivate::retransform() {
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

void Base3DPlotPrivate::updateTheme() {
	// QtGraphs API: setTheme() is called directly on the graph widget, not on activeTheme()
	// Cast theme enum from Base3DPlot::Theme to QGraphsTheme::Theme
	auto graphsTheme = static_cast<QGraphsTheme::Theme>(theme);

	switch (type) {
	case Base3DPlot::Type::Surface:
		q->m_surface->activeTheme()->setTheme(graphsTheme);
		break;
	case Base3DPlot::Type::Scatter:
		q->m_scatter->activeTheme()->setTheme(graphsTheme);
		break;
	case Base3DPlot::Type::Bar:
		q->m_bar->activeTheme()->setTheme(graphsTheme);
		break;
	default:
		break;
	}

	Q_EMIT q->changed();
}

void Base3DPlotPrivate::updateZoomLevel() {
	// QtGraphs widget API: camera properties are directly on the widget
	switch (type) {
	case Base3DPlot::Type::Surface:
		q->m_surface->setCameraZoomLevel(zoomLevel);
		break;
	case Base3DPlot::Type::Scatter:
		q->m_scatter->setCameraZoomLevel(zoomLevel);
		break;
	case Base3DPlot::Type::Bar:
		q->m_bar->setCameraZoomLevel(zoomLevel);
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}

void Base3DPlotPrivate::updateShadowQuality() {
	// QtGraphs API: enum is QtGraphs3D::ShadowQuality
	auto quality = static_cast<QtGraphs3D::ShadowQuality>(shadowQuality);
	switch (type) {
	case Base3DPlot::Type::Surface:
		q->m_surface->setShadowQuality(quality);
		break;
	case Base3DPlot::Type::Scatter:
		q->m_scatter->setShadowQuality(quality);
		break;
	case Base3DPlot::Type::Bar:
		q->m_bar->setShadowQuality(quality);
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
void Base3DPlotPrivate::updateXRotation() {
	// QtGraphs widget API: camera properties are directly on the widget
	switch (type) {
	case Base3DPlot::Type::Surface:
		q->m_surface->setCameraXRotation(xRotation);
		break;
	case Base3DPlot::Type::Scatter:
		q->m_scatter->setCameraXRotation(xRotation);
		break;
	case Base3DPlot::Type::Bar:
		q->m_bar->setCameraXRotation(xRotation);
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
void Base3DPlotPrivate::updateYRotation() {
	// QtGraphs widget API: camera properties are directly on the widget
	switch (type) {
	case Base3DPlot::Type::Surface:
		q->m_surface->setCameraYRotation(yRotation);
		break;
	case Base3DPlot::Type::Scatter:
		q->m_scatter->setCameraYRotation(yRotation);
		break;
	case Base3DPlot::Type::Bar:
		q->m_bar->setCameraYRotation(yRotation);
		break;
	default:
		break;
	}
	Q_EMIT q->changed();
}
