#include "Scatter3DScene.h"
#include "Axis3D.h"
#include "Scatter3DPlot.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/WorksheetElementPrivate.h"
#include "backend/worksheet/plots/3d/Scatter3DScenePrivate.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QIcon>
#include <QMenu>
#include <QXmlStreamAttributes>
#include <QtGraphsWidgets/Q3DScatterWidgetItem>

#include <KLocalizedString>

Scatter3DScene::Scatter3DScene(const QString& name)
	: Base3DPlot(name, new Scatter3DScenePrivate(this), Base3DPlot::Scatter, AspectType::Scatter3DScene) {
	m_scatter = new Q3DScatterWidgetItem();

	// Create QQuickWidget and set it on the WidgetItem
	auto* quickWidget = new QQuickWidget();
	m_scatter->setWidget(quickWidget);

	// Apply initial rotation and zoom values to the widget
	Q_D(Scatter3DScene);
	m_scatter->setCameraXRotation(d->xRotation);
	m_scatter->setCameraYRotation(d->yRotation);
	m_scatter->setCameraZoomLevel(d->zoomLevel);

	// Listen for child Plot additions
	connect(this, &AbstractAspect::childAspectAdded, this, &Scatter3DScene::handleChildAdded);

	// Only create default axes when NOT loading from file
	// Check parent too - if Scene is loading, axes come as separate children
	auto* parent = parentAspect();
	bool parentLoading = parent && parent->isLoading();

	if (!isLoading() && !parentLoading) {
		Axis3D* xAxis = new Axis3D(QStringLiteral("x-axis"), Axis3D::X);
		Axis3D* yAxis = new Axis3D(QStringLiteral("y-axis"), Axis3D::Y);
		Axis3D* zAxis = new Axis3D(QStringLiteral("z-axis"), Axis3D::Z);
		addChild(xAxis);
		addChild(yAxis);
		addChild(zAxis);
		finalizeLoad(); // Connect axes immediately for new plots
	}
}

void Scatter3DScene::finalizeAdd() {
	WorksheetElement::finalizeAdd();
}

Scatter3DScene::~Scatter3DScene() {
}

void Scatter3DScene::init(bool transform) {
	Q_D(Scatter3DScene);

	// Create proxy widget and set as child of this graphics item
	if (m_scatter && m_scatter->widget()) {
		auto* widget = m_scatter->widget();
		auto* proxy = new QGraphicsProxyWidget();
		proxy->setParentItem(static_cast<QGraphicsItem*>(d));
		proxy->setWidget(widget);

		// Size from rect
		QRectF rect = d->rect;
		proxy->setGeometry(QRectF(-rect.width() / 2, -rect.height() / 2, rect.width(), rect.height()));
		widget->resize(rect.width(), rect.height());
		widget->show();
	}

	if (transform)
		retransform();
}

void Scatter3DScene::retransform() {
	Q_D(Scatter3DScene);
	d->retransform();
}

void Scatter3DScene::finalizeLoad() {
	// Connect axes to the 3D widget after load
	for (auto* child : children<Axis3D>()) {
		if (child->type() == Axis3D::X)
			m_scatter->setAxisX(child->m_axis);
		else if (child->type() == Axis3D::Y)
			m_scatter->setAxisY(child->m_axis);
		else if (child->type() == Axis3D::Z)
			m_scatter->setAxisZ(child->m_axis);
	}
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

// Spreadsheet parameters
BASIC_SHARED_D_READER_IMPL(Scatter3DScene, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DScene, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DScene, const AbstractColumn*, zColumn, zColumn)
BASIC_SHARED_D_READER_IMPL(Scatter3DScene, QString, xColumnPath, xColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DScene, QString, yColumnPath, yColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DScene, QString, zColumnPath, zColumnPath)
BASIC_SHARED_D_READER_IMPL(Scatter3DScene, Scatter3DScene::PointStyle, pointStyle, pointStyle)
BASIC_SHARED_D_READER_IMPL(Scatter3DScene, QColor, color, color)
// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(Scatter3DScene, SetXColumn, const AbstractColumn*, xColumn, recalc)
void Scatter3DScene::setXColumn(const AbstractColumn* xCol) {
	Q_D(Scatter3DScene);
	if (xCol != d->xColumn)
		exec(new Scatter3DSceneSetXColumnCmd(d, xCol, ki18n("%1: X Column changed")));
	if (xCol) {
		connect(xCol, &AbstractColumn::dataChanged, this, &Scatter3DScene::recalc);
		if (xCol->parentAspect())
			connect(xCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Scatter3DScene::xColumnAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DScene, SetYColumn, const AbstractColumn*, yColumn, recalc)
void Scatter3DScene::setYColumn(const AbstractColumn* yCol) {
	Q_D(Scatter3DScene);
	if (yCol != d->yColumn)
		exec(new Scatter3DSceneSetYColumnCmd(d, yCol, ki18n("%1: Y Column changed")));
	if (yCol) {
		connect(yCol, &AbstractColumn::dataChanged, this, &Scatter3DScene::recalc);
		if (yCol->parentAspect())
			connect(yCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Scatter3DScene::yColumnAboutToBeRemoved);
	}
}
STD_SETTER_CMD_IMPL_F_S(Scatter3DScene, SetZColumn, const AbstractColumn*, zColumn, recalc)
void Scatter3DScene::setZColumn(const AbstractColumn* zCol) {
	Q_D(Scatter3DScene);
	if (zCol != d->zColumn)
		exec(new Scatter3DSceneSetZColumnCmd(d, zCol, ki18n("%1: Z Column changed")));
	if (zCol) {
		connect(zCol, &AbstractColumn::dataChanged, this, &Scatter3DScene::recalc);
		if (zCol->parentAspect())
			connect(zCol->parentAspect(), &AbstractAspect::childAspectAboutToBeRemoved, this, &Scatter3DScene::zColumnAboutToBeRemoved);
	}
}

STD_SETTER_CMD_IMPL_F_S(Scatter3DScene, SetPointStyle, Scatter3DScene::PointStyle, pointStyle, updatePointStyle)
void Scatter3DScene::setPointStyle(Scatter3DScene::PointStyle pointStyle) {
	Q_D(Scatter3DScene);
	if (pointStyle != d->pointStyle)
		exec(new Scatter3DSceneSetPointStyleCmd(d, pointStyle, ki18n("%1: point style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Scatter3DScene, SetColor, QColor, color, updateColor)
void Scatter3DScene::setColor(QColor color) {
	Q_D(Scatter3DScene);
	if (color != d->color)
		exec(new Scatter3DSceneSetColorCmd(d, color, ki18n("%1: color changed")));
}

void Scatter3DScene::xColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DScene);
	d->xColumn = nullptr;
}

void Scatter3DScene::yColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DScene);
	d->yColumn = nullptr;
}
void Scatter3DScene::zColumnAboutToBeRemoved(const AbstractAspect*) {
	Q_D(Scatter3DScene);
	d->zColumn = nullptr;
}

void Scatter3DScene::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}

void Scatter3DScene::recalc() {
	Q_D(Scatter3DScene);
	d->recalc();
}

void Scatter3DScene::handleChildAdded(const AbstractAspect* child) {
	auto* plot = dynamic_cast<const Scatter3DPlot*>(child);
	if (!plot)
		return;

	// Connect to Plot column changes to trigger recalc
	connect(plot, &Scatter3DPlot::xColumnChanged, this, &Scatter3DScene::recalc);
	connect(plot, &Scatter3DPlot::yColumnChanged, this, &Scatter3DScene::recalc);
	connect(plot, &Scatter3DPlot::zColumnChanged, this, &Scatter3DScene::recalc);

	// Trigger immediate recalc if columns are already set
	if (plot->xColumn() && plot->yColumn() && plot->zColumn())
		recalc();
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Scatter3DScenePrivate::Scatter3DScenePrivate(Scatter3DScene* owner)
	: Base3DPlotPrivate(owner)
	, pointStyle(Scatter3DScene::Sphere)
	, color(Qt::green)
	, q(owner) {
}

void Scatter3DScenePrivate::retransform() {
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
void Scatter3DScenePrivate::recalcShapeAndBoundingRect() {
}
void Scatter3DScenePrivate::recalc() {
	// Get columns from first child Scatter3DPlot
	auto plots = q->children<Scatter3DPlot>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	if (plots.isEmpty())
		return;

	auto* plot = plots.first();
	const AbstractColumn* xCol = plot->xColumn();
	const AbstractColumn* yCol = plot->yColumn();
	const AbstractColumn* zCol = plot->zColumn();

	if (!xCol || !yCol || !zCol)
		return;

	qDebug() << Q_FUNC_INFO << "Columns have been set";
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	const int numPoints = std::min(xCol->availableRowCount(), std::min(yCol->availableRowCount(), zCol->availableRowCount()));
	if (numPoints == 0)
		return;

	QScatterDataArray* data = new QScatterDataArray;
	data->resize(numPoints);

	QScatterDataItem* ptrToDataArray = &data->first();
	for (int i = 0; i < numPoints; ++i) {
		const float x = static_cast<float>(xCol->valueAt(i));
		const float y = static_cast<float>(yCol->valueAt(i));
		const float z = static_cast<float>(zCol->valueAt(i));

		ptrToDataArray->setPosition(QVector3D(x, y, z));
		++ptrToDataArray;
	}

	QScatter3DSeries* series = new QScatter3DSeries;
	series->dataProxy()->resetArray(*data);
	q->m_scatter->addSeries(series);
	Q_EMIT q->changed();
}
void Scatter3DScenePrivate::updateColor() {
	auto* series = q->m_scatter->seriesList().first();
	if (!series)
		return;
	series->setBaseColor(color);
	Q_EMIT q->changed();
}

void Scatter3DScenePrivate::updatePointStyle() {
	QScatter3DSeries* series = q->m_scatter->seriesList().at(0);
	if (!series)
		return;

	switch (pointStyle) {
	case Scatter3DScene::Sphere:
		series->setMesh(QAbstract3DSeries::Mesh::Sphere);
		break;
	case Scatter3DScene::Cube:
		series->setMesh(QAbstract3DSeries::Mesh::Cube);
		break;
	case Scatter3DScene::Cone:
		series->setMesh(QAbstract3DSeries::Mesh::Cone);
		break;
	case Scatter3DScene::Pyramid:
		series->setMesh(QAbstract3DSeries::Mesh::Pyramid);
		break;
	default:
		series->setMesh(QAbstract3DSeries::Mesh::Sphere);
		break;
	}
	Q_EMIT q->changed();
}
// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Scatter3DScene::save(QXmlStreamWriter* writer) const {
	Q_D(const Scatter3DScene);

	writer->writeStartElement("scatter3dplot");

	// Save x, y, z columns
	WRITE_COLUMN(d->xColumn, xColumn);
	WRITE_COLUMN(d->yColumn, yColumn);
	WRITE_COLUMN(d->zColumn, zColumn);

	// Save Scatter3DScene specific attributes
	writer->writeAttribute("pointStyle", QString::number(static_cast<int>(d->pointStyle)));
	writer->writeAttribute("color", d->color.name());

	// Save Base3DPlotPrivate attributes
	writer->writeAttribute("xRotation", QString::number(d->xRotation));
	writer->writeAttribute("yRotation", QString::number(d->yRotation));
	writer->writeAttribute("theme", QString::number(static_cast<int>(d->theme)));
	writer->writeAttribute("zoomLevel", QString::number(d->zoomLevel));
	writer->writeAttribute("shadowQuality", QString::number(static_cast<int>(d->shadowQuality)));

	// Save base class attributes
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeEndElement();
}

//! Load as XML
bool Scatter3DScene::load(XmlStreamReader* reader, bool preview) {
	Q_D(Scatter3DScene);

	// Reading basic attributes
	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == QLatin1String("scatter3dplot"))
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

			str = attribs.value(QStringLiteral("pointStyle")).toString();
			if (!str.isEmpty())
				d->pointStyle = static_cast<Scatter3DScene::PointStyle>(str.toInt());
		} else if (reader->name() == QLatin1String("column")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("path")).toString();
			if (!str.isEmpty())
				d->xColumnPath = str; // Assuming xColumnPath should be set here. Adapt as needed.
		} else { // Unknown element handling
			reader->raiseWarning(i18n("Unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// Connect axes after all children are loaded
	finalizeLoad();

	return true;
}

// ##############################################################################
// ######################### Context Menu  ######################################
// ##############################################################################

void Scatter3DScene::initMenus() {
	m_addNewMenu = new QMenu(i18n("Add New"));
	m_addNewMenu->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	auto* action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-3d-plot")), i18n("3D Scatter"), this);
	connect(action, &QAction::triggered, this, &Scatter3DScene::addPlot);
	m_addNewMenu->addAction(action);

	m_menusInitialized = true;
}

QMenu* Scatter3DScene::createContextMenu() {
	if (!m_menusInitialized)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* visibilityAction = this->visibilityAction();

	menu->insertMenu(visibilityAction, m_addNewMenu);
	menu->insertSeparator(visibilityAction);

	return menu;
}

void Scatter3DScene::addPlot() {
	auto* plot = new Scatter3DPlot(i18n("Scatter Plot"));
	addChild(plot);
}
