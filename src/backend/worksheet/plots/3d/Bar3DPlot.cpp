#include "Bar3DPlot.h"
#include "Axis3D.h"
#include "Bar3DPlotPrivate.h"
#include "MouseInteractor.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"

Bar3DPlot::Bar3DPlot(const QString& name)
	: Base3DArea(name, new Bar3DPlotPrivate(this), Base3DArea::Bar, AspectType::Bar3DPlot) {
	m_bar = new Q3DBars();
	m_bar->setActiveInputHandler(new MouseInteractor());
}

Bar3DPlot::~Bar3DPlot() {
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

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Bar3DPlot::save(QXmlStreamWriter* writer) const {
	Q_D(const Bar3DPlot);

	writer->writeStartElement("bar3dplot");

	// Saving data columns
	for (const auto& column : d->dataColumns) {
		if (column) {
			writer->writeStartElement("datacolumn");
			writer->writeAttribute("path", d->columnPaths.value(d->dataColumns.indexOf(column)));
			writer->writeEndElement(); // datacolumn
		}
	}

	// Saving color
	writer->writeStartElement("color");
	writer->writeAttribute("value", d->color.name());
	writer->writeEndElement(); // color

	// Saving attributes from the Base3DAreaPrivate class
	writer->writeStartElement("base3darea");

	writer->writeAttribute("xRotation", QString::number(d->xRotation));
	writer->writeAttribute("yRotation", QString::number(d->yRotation));
	writer->writeAttribute("theme", QString::number(static_cast<int>(d->theme)));
	writer->writeAttribute("zoomLevel", QString::number(d->zoomLevel));
	writer->writeAttribute("shadowQuality", QString::number(static_cast<int>(d->shadowQuality)));

	writer->writeEndElement(); // base3darea

	// Saving basic attributes and comments, similar to the Curve3D example
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeEndElement(); // bar3dplot
}

bool Bar3DPlot::load(XmlStreamReader* reader, bool preview) {
	Q_D(Bar3DPlot);

	// Reading basic attributes
	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == QLatin1String("bar3dplot"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			READ_INT_VALUE("xRotation", xRotation, int);
			READ_INT_VALUE("yRotation", yRotation, int);
			READ_INT_VALUE("theme", theme, Base3DArea::Theme);
			READ_INT_VALUE("zoomLevel", zoomLevel, int);
			READ_INT_VALUE("shadowQuality", shadowQuality, Base3DArea::ShadowQuality);

			str = attribs.value(QStringLiteral("color")).toString();
			if (!str.isEmpty())
				d->color.setNamedColor(str);
		} else if (reader->name() == QLatin1String("column")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("path")).toString();
			if (!str.isEmpty())
				d->columnPaths << str;
			// READ_COLUMN logic can be placed here if needed.
		} else { // Unknown element handling
			reader->raiseWarning(i18n("Unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	d->dataColumns.resize(d->columnPaths.size());

	return true;
}
