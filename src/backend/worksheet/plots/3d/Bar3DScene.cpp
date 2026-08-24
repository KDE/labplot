/***************************************************************************
	File                 : Bar3DScene.cpp
	Project              : LabPlot
	Description          : 3D Bar Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "Bar3DScene.h"
#include "Axis3D.h"
#include "Bar3DScenePrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/WorksheetElementPrivate.h"

#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QtGraphsWidgets/Q3DBarsWidgetItem>

#include <KLocalizedString>

Bar3DScene::Bar3DScene(const QString& name)
	: Base3DPlot(name, new Bar3DScenePrivate(this), Base3DPlot::Bar, AspectType::Bar3DScene) {
	m_bar = new Q3DBarsWidgetItem();

	// Create QQuickWidget and set it on the WidgetItem
	auto* quickWidget = new QQuickWidget();
	m_bar->setWidget(quickWidget);

	// Apply initial rotation and zoom values to the widget
	Q_D(Bar3DScene);
	m_bar->setCameraXRotation(d->xRotation);
	m_bar->setCameraYRotation(d->yRotation);
	m_bar->setCameraZoomLevel(d->zoomLevel);
}

void Bar3DScene::finalizeAdd() {
	WorksheetElement::finalizeAdd();
}

Bar3DScene::~Bar3DScene() {
}

void Bar3DScene::init(bool transform) {
	Q_D(Bar3DScene);

	// Create proxy widget and set as child of this graphics item
	if (m_bar && m_bar->widget()) {
		auto* widget = m_bar->widget();
		auto* proxy = new QGraphicsProxyWidget();
		proxy->setParentItem(static_cast<QGraphicsItem*>(d));
		proxy->setWidget(widget);

		// Size from rect
		QRectF rect = d->rect;
		proxy->setGeometry(QRectF(-rect.width()/2, -rect.height()/2, rect.width(), rect.height()));
		widget->resize(rect.width(), rect.height());
		widget->show();
	}

	if (transform)
		retransform();
}

void Bar3DScene::retransform() {
	Q_D(Bar3DScene);
	d->retransform();
}

void Bar3DScene::recalc() {
	Q_D(Bar3DScene);
	d->recalc();
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

BASIC_SHARED_D_READER_IMPL(Bar3DScene, QVector<AbstractColumn*>, dataColumns, dataColumns)
BASIC_SHARED_D_READER_IMPL(Bar3DScene, QVector<QString>, columnPaths, columnPaths)
BASIC_SHARED_D_READER_IMPL(Bar3DScene, QColor, color, color)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

STD_SETTER_CMD_IMPL_F_S(Bar3DScene, SetColumns, QVector<AbstractColumn*>, dataColumns, recalc)
void Bar3DScene::setDataColumns(QVector<AbstractColumn*> dataColumns) {
	Q_D(Bar3DScene);
	if (dataColumns != d->dataColumns) {
		exec(new Bar3DSceneSetColumnsCmd(d, dataColumns, ki18n("%1: columns changed")));
		for (auto* column : dataColumns) {
			if (!column)
				continue;
			connect(column, &AbstractColumn::dataChanged, this, &Bar3DScene::recalc);
			if (column->parentAspect())
				connect(column->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Bar3DScene::columnAboutToBeRemoved);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Bar3DScene, SetColor, QColor, color, updateColor)
void Bar3DScene::setColor(QColor color) {
	Q_D(Bar3DScene);
	if (color != d->color)
		exec(new Bar3DSceneSetColorCmd(d, color, ki18n("%1: color changed")));
}

void Bar3DScene::setColumnPaths(const QVector<QString>& paths) {
	Q_D(Bar3DScene);
	d->columnPaths = paths;
}
class Bar3DSceneSetRectCmd : public QUndoCommand {
public:
	Bar3DSceneSetRectCmd(Bar3DScenePrivate* private_obj, const QRectF& rect)
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
	Bar3DScenePrivate* m_private;
	QRectF m_rect;
};
void Bar3DScene::setRect(const QRectF& rect) {
	Q_D(Bar3DScene);
	if (rect != d->rect)
		exec(new Bar3DSceneSetRectCmd(d, rect));
}
class Bar3DSceneAreaSetPrevRectCmd : public QUndoCommand {
public:
	Bar3DSceneAreaSetPrevRectCmd(Bar3DScenePrivate* private_obj, const QRectF& rect)
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
	Bar3DScenePrivate* m_private;
	QRectF m_rect;
	bool m_initilized{false};
};

void Bar3DScene::setPrevRect(const QRectF& prevRect) {
	Q_D(Bar3DScene);
	exec(new Bar3DSceneAreaSetPrevRectCmd(d, prevRect));
}

void Bar3DScene::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}

void Bar3DScene::columnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Bar3DScene);
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
Bar3DScenePrivate::Bar3DScenePrivate(Bar3DScene* owner)
	: Base3DPlotPrivate(owner)
	, q(owner)
	, color(Qt::green) {
}
void Bar3DScenePrivate::retransform() {
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

void Bar3DScenePrivate::recalcShapeAndBoundingRect() {
}

void Bar3DScenePrivate::recalc() {
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
	auto* dataArray = new QBarDataArray;
	dataArray->reserve(numColumns);
	for (int col = 0; col < numColumns; ++col) {
		QBarDataRow dataRow;
		dataRow.resize(numRows);
		for (int row = 0; row < numRows; ++row) {
			if (dataColumns[col] != nullptr) {
				const float value = static_cast<float>(dataColumns[col]->valueAt(row));
				dataRow[row].setValue(value);
			}
		}
		dataArray->append(dataRow);
	}

	auto* series = new QBar3DSeries;
	series->dataProxy()->resetArray(*dataArray);
	q->m_bar->addSeries(series);
	Q_EMIT q->changed();
}

void Bar3DScenePrivate::updateColor() {
	auto* series = q->m_bar->seriesList().first();
	if (!series)
		return;
	series->setBaseColor(color);
	//q->m_bar->update();
	Q_EMIT q->changed();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Bar3DScene::save(QXmlStreamWriter* writer) const {
	Q_D(const Bar3DScene);

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

	// Saving attributes from the Base3DPlotPrivate class
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

bool Bar3DScene::load(XmlStreamReader* reader, bool preview) {
	Q_D(Bar3DScene);

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
			READ_INT_VALUE("theme", theme, Base3DPlot::Theme);
			READ_INT_VALUE("zoomLevel", zoomLevel, int);
			READ_INT_VALUE("shadowQuality", shadowQuality, Base3DPlot::ShadowQuality);

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
