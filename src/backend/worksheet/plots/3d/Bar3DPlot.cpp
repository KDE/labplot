#include "Bar3DPlot.h"
#include "Axis3D.h"
#include "Bar3DPlotPrivate.h"
#include "MouseInteractor.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"

Bar3DPlot::Bar3DPlot(const QString& name)
	: Base3DArea(name, Base3DArea::Bar, AspectType::Bar3DPlot) {
	m_bar = new Q3DBars();
	m_bar->setActiveInputHandler(new MouseInteractor());
}

Bar3DPlot::~Bar3DPlot() {
}

void Bar3DPlot::save(QXmlStreamWriter*) const {
	// TODO
}

bool Bar3DPlot::load(XmlStreamReader*, bool preview) {
	// TODO
	return 1;
}
void Bar3DPlot::recalc() {
	Q_D(Bar3DPlot);
	d->recalc();
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

BASIC_SHARED_D_READER_IMPL(Bar3DPlot, QVector<AbstractColumn*>, dataColumns, dataColumns)
BASIC_SHARED_D_READER_IMPL(Bar3DPlot, QVector<QString>, columnPaths, columnPaths)
BASIC_SHARED_D_READER_IMPL(Bar3DPlot, QColor, color, color)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

STD_SETTER_CMD_IMPL_F_S(Bar3DPlot, SetColumns, QVector<AbstractColumn*>, dataColumns, recalc)
void Bar3DPlot::setDataColumns(QVector<AbstractColumn*> dataColumns) {
	Q_D(Bar3DPlot);
	if (dataColumns != d->dataColumns) {
		exec(new Bar3DPlotSetColumnsCmd(d, dataColumns, ki18n("%1: columns changed")));
		for (auto* column : dataColumns) {
			if (!column)
				continue;
			connect(column, &AbstractColumn::dataChanged, this, &Bar3DPlot::recalc);
			if (column->parentAspect())
				connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Bar3DPlot::columnAboutToBeRemoved);
			connect(column, &AbstractColumn::dataChanged, this, &Bar3DPlot::dataChanged);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Bar3DPlot, SetColor, QColor, color, updateColor)
void Bar3DPlot::setColor(QColor color) {
	Q_D(Bar3DPlot);
	if (color != d->color)
		exec(new Bar3DPlotSetColorCmd(d, color, ki18n("%1: color changed")));
}

void Bar3DPlot::setColumnPaths(const QVector<QString>& paths) {
	Q_D(Bar3DPlot);
	d->columnPaths = paths;
}
class Bar3DPlotSetRectCmd : public QUndoCommand {
public:
	Bar3DPlotSetRectCmd(Bar3DPlotPrivate* private_obj, const QRectF& rect)
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
	Bar3DPlotPrivate* m_private;
	QRectF m_rect;
};
void Bar3DPlot::setRect(const QRectF& rect) {
	Q_D(Bar3DPlot);
	if (rect != d->rect)
		exec(new Bar3DPlotSetRectCmd(d, rect));
}
class Bar3DPlotAreaSetPrevRectCmd : public QUndoCommand {
public:
	Bar3DPlotAreaSetPrevRectCmd(Bar3DPlotPrivate* private_obj, const QRectF& rect)
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
	Bar3DPlotPrivate* m_private;
	QRectF m_rect;
	bool m_initilized{false};
};

void Bar3DPlot::setPrevRect(const QRectF& prevRect) {
	Q_D(Bar3DPlot);
	exec(new Bar3DPlotAreaSetPrevRectCmd(d, prevRect));
}

void Bar3DPlot::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}

void Bar3DPlot::retransform() {
	Q_D(Bar3DPlot);
	d->retransform();
}

void Bar3DPlot::columnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Bar3DPlot);
	for (int i = 0; i < d->dataColumns.size(); ++i) {
		if (aspect == d->dataColumns.at(i)) {
			d->dataColumns[i] = nullptr;
			d->recalc();
			Q_EMIT changed();
			break;
		}
	}
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Bar3DPlotPrivate::Bar3DPlotPrivate(Bar3DPlot* owner)
	: Base3DAreaPrivate(owner)
	, q(owner)
	, color(Qt::green) {
}
void Bar3DPlotPrivate::retransform() {
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

void Bar3DPlotPrivate::recalcShapeAndBoundingRect() {
}
void Bar3DPlotPrivate::recalc() {
	if (dataColumns.isEmpty())
		return;
	qDebug() << Q_FUNC_INFO << "Columns have been set";
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	// Determine the number of columns and rows
	const int numColumns = dataColumns.size();
	int numRows = INT_MAX;
	for (const auto& column : dataColumns) {
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
			if (dataColumns[col] != nullptr) {
				const float value = static_cast<float>(dataColumns[col]->valueAt(row));
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
void Bar3DPlotPrivate::updateColor() {
	auto* series = q->m_bar->seriesList().first();
	if (!series)
		return;
	series->setBaseColor(color);
	q->m_bar->update();
	Q_EMIT q->changed();
}
