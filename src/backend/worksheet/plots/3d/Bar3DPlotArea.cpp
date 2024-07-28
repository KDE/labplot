#include "Bar3DPlotArea.h"
#include "Axis3D.h"
#include "Bar3DPrivatePlotArea.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"

Bar3DPlotArea::Bar3DPlotArea(QString name)
	: WorksheetElementContainer(name, new Bar3DPlotAreaPrivate(this), AspectType::Bar3DPlot)
	, m_bar{new Q3DBars()} {
}

Bar3DPlotArea::~Bar3DPlotArea() {
}

void Bar3DPlotArea::save(QXmlStreamWriter*) const {
	// TODO
}

bool Bar3DPlotArea::load(XmlStreamReader*, bool preview) {
	// TODO
	return 1;
}
void Bar3DPlotArea::recalc() {
	Q_D(Bar3DPlotArea);
	d->recalc();
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

BASIC_SHARED_D_READER_IMPL(Bar3DPlotArea, QVector<AbstractColumn*>, columns, columns)
BASIC_SHARED_D_READER_IMPL(Bar3DPlotArea, QVector<QString>, columnPaths, columnPaths)
BASIC_SHARED_D_READER_IMPL(Bar3DPlotArea, int, xRotation, xRotation)
BASIC_SHARED_D_READER_IMPL(Bar3DPlotArea, int, yRotation, yRotation)
BASIC_SHARED_D_READER_IMPL(Bar3DPlotArea, int, zoomLevel, zoomLevel)
BASIC_SHARED_D_READER_IMPL(Bar3DPlotArea, Bar3DPlotArea::Theme, theme, theme)
BASIC_SHARED_D_READER_IMPL(Bar3DPlotArea, Bar3DPlotArea::ShadowQuality, shadowQuality, shadowQuality)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

STD_SETTER_CMD_IMPL_F_S(Bar3DPlotArea, SetColumns, QVector<AbstractColumn*>, columns, recalc)
void Bar3DPlotArea::setColumns(QVector<AbstractColumn*> columns) {
	Q_D(Bar3DPlotArea);
	if (columns != d->columns) {
		exec(new Bar3DPlotAreaSetColumnsCmd(d, columns, ki18n("%1: columns changed")));
		for (auto* column : columns) {
			if (!column)
				continue;
			connect(column, &AbstractColumn::dataChanged, &Bar3DPlotArea::recalc);
			if (column->parentAspect())
				connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, &Bar3DPlotArea::columnAboutToBeRemoved);
			connect(column, &AbstractColumn::dataChanged, &Bar3DPlotArea::dataChanged);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Bar3DPlotArea, SetShadowQuality, Bar3DPlotArea::ShadowQuality, shadowQuality, updateShadowQuality)
void Bar3DPlotArea::setShadowQuality(Bar3DPlotArea::ShadowQuality shadowQuality) {
	Q_D(Bar3DPlotArea);
	if (shadowQuality != d->shadowQuality)
		exec(new Bar3DPlotAreaSetShadowQualityCmd(d, shadowQuality, ki18n("%1: shadow quality changed")));
}

STD_SETTER_CMD_IMPL_F_S(Bar3DPlotArea, SetTheme, Bar3DPlotArea::Theme, theme, updateTheme)
void Bar3DPlotArea::setTheme(Bar3DPlotArea::Theme theme) {
	Q_D(Bar3DPlotArea);
	if (theme != d->theme)
		exec(new Bar3DPlotAreaSetThemeCmd(d, theme, ki18n("%1: theme changed")));
}
STD_SETTER_CMD_IMPL_F_S(Bar3DPlotArea, SetColor, QColor, color, updateColor)
void Bar3DPlotArea::setColor(QColor color) {
	Q_D(Bar3DPlotArea);
	if (color != d->color)
		exec(new Bar3DPlotAreaSetColorCmd(d, color, ki18n("%1: color changed")));
}
STD_SETTER_CMD_IMPL_F_S(Bar3DPlotArea, SetXRotation, int, xRotation, updateXRotation)
void Bar3DPlotArea::setXRotation(int xRot) {
	Q_D(Bar3DPlotArea);
	if (xRot != d->xRotation)
		exec(new Bar3DPlotAreaSetXRotationCmd(d, xRot, ki18n("%1: X Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Bar3DPlotArea, SetYRotation, int, yRotation, updateYRotation)
void Bar3DPlotArea::setYRotation(int yRot) {
	Q_D(Bar3DPlotArea);
	if (yRot != d->yRotation)
		exec(new Bar3DPlotAreaSetXRotationCmd(d, yRot, ki18n("%1: Y Rotation changed")));
}
STD_SETTER_CMD_IMPL_F_S(Bar3DPlotArea, SetZoomLevel, int, zoomLevel, updateZoomLevel)
void Bar3DPlotArea::setZoomLevel(int zoom) {
	Q_D(Bar3DPlotArea);
	if (zoom != d->zoomLevel)
		exec(new Bar3DPlotAreaSetZoomLevelCmd(d, zoom, ki18n("%1: zoom changed")));
}
void Bar3DPlotArea::setColumnPaths(const QVector<QString>& paths) {
	Q_D(Bar3DPlotArea);
	d->columnPaths = paths;
}
class Bar3DPlotAreaSetRectCmd : public QUndoCommand {
public:
	Bar3DPlotAreaSetRectCmd(Bar3DPlotAreaPrivate* private_obj, const QRectF& rect)
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
	Bar3DPlotAreaPrivate* m_private;
	QRectF m_rect;
};
void Bar3DPlotArea::setRect(const QRectF& rect) {
	Q_D(Bar3DPlotArea);
	if (rect != d->rect)
		exec(new Bar3DPlotAreaSetRectCmd(d, rect));
}
class Bar3DPlotAreaSetPrevRectCmd : public QUndoCommand {
public:
	Bar3DPlotAreaSetPrevRectCmd(Bar3DPlotAreaPrivate* private_obj, const QRectF& rect)
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
	Bar3DPlotAreaPrivate* m_private;
	QRectF m_rect;
	bool m_initilized{false};
};

void Bar3DPlotArea::setPrevRect(const QRectF& prevRect) {
	Q_D(Bar3DPlotArea);
	exec(new Bar3DPlotAreaSetPrevRectCmd(d, prevRect));
}

void Bar3DPlotArea::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}

void Bar3DPlotArea::retransform() {
	Q_D(Bar3DPlotArea);
	d->retransform();
}

void Bar3DPlotArea::columnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Bar3DPlotArea);
	for (int i = 0; i < d->columns.size(); ++i) {
		if (aspect == d->columns.at(i)) {
			d->columns[i] = nullptr;
			d->recalc();
			Q_EMIT changed();
			break;
		}
	}
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Bar3DPlotAreaPrivate::Bar3DPlotAreaPrivate(Bar3DPlotArea* owner)
	: WorksheetElementContainerPrivate(owner)
	, q(owner)
	, shadowQuality(Bar3DPlotArea::None)
	, theme(Bar3DPlotArea::Qt) {
}
void Bar3DPlotAreaPrivate::retransform() {
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

void Bar3DPlotAreaPrivate::recalcShapeAndBoundingRect() {
}
void Bar3DPlotAreaPrivate::recalc() {
	if (columns.isEmpty())
		return;
	qDebug() << Q_FUNC_INFO << "Columns have been set";
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	// Determine the number of columns and rows
	const int numColumns = columns.size();
	int numRows = INT_MAX;
	for (const auto& column : columns) {
		if (column != nullptr)
			numRows = std::min(numRows, column->availableRowCount());
	}
	if (numRows == 0 || numColumns == 0)
		return;
	QBarDataArray* dataArray = new QBarDataArray;
	dataArray->reserve(numColumns);
	for (int col = 0; col < numColumns; ++col) {
		QBarDataRow* dataRow = new QBarDataRow(numRows);
		for (int row = 0; row < numRows; ++row) {
			if (columns[col] != nullptr) {
				const float value = static_cast<float>(columns[col]->valueAt(row));
				(*dataRow)[row].setValue(value);
			}
		}
		dataArray->append(*dataRow);
	}
	QBar3DSeries* series = new QBar3DSeries;
	series->dataProxy()->resetArray(*dataArray);
	q->m_bar->addSeries(series);
	Q_EMIT q->changed();
}
void Bar3DPlotAreaPrivate::updateColor() {
	auto* series = q->m_bar->seriesList().first();
	if (!series)
		return;
	series->setBaseColor(color);
	q->m_bar->update();
	Q_EMIT q->changed();
}
void Bar3DPlotAreaPrivate::updateShadowQuality() {
	q->m_bar->setShadowQuality(static_cast<QAbstract3DGraph::ShadowQuality>(shadowQuality));
	q->m_bar->update();
	Q_EMIT q->changed();
}
void Bar3DPlotAreaPrivate::updateTheme() {
	q->m_bar->activeTheme()->setType(static_cast<Q3DTheme::Theme>(theme));
	q->m_bar->update();
	Q_EMIT q->changed();
}
void Bar3DPlotAreaPrivate::updateXRotation() {
	q->m_bar->setCameraXRotation(xRotation);
	Q_EMIT q->changed();
}
void Bar3DPlotAreaPrivate::updateYRotation() {
	q->m_bar->setCameraXRotation(xRotation);
	Q_EMIT q->changed();
}
void Bar3DPlotAreaPrivate::updateZoomLevel() {
	q->m_bar->setCameraZoomLevel(zoomLevel);
	Q_EMIT q->changed();
}
