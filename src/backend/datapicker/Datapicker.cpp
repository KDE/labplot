/*
	File                 : Datapicker.cpp
	Project              : LabPlot
	Description          : Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Datapicker.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/Transform.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/DefaultColorTheme.h"
#include "frontend/datapicker/DatapickerView.h"

#include "QIcon"
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QGraphicsScene>

/**
 * \class Datapicker
 * \brief Top-level container for DatapickerCurve and DatapickerImage.
 * \ingroup backend
 */
Datapicker::Datapicker(const QString& name, const bool loading)
	: AbstractPart(name, AspectType::Datapicker)
	, m_transform(new Transform()) {
	connect(this, &Datapicker::childAspectAdded, this, &Datapicker::handleAspectAdded);
	connect(this, &Datapicker::childAspectAboutToBeRemoved, this, &Datapicker::handleAspectAboutToBeRemoved);

	if (!loading)
		init();
}

Datapicker::~Datapicker() {
	delete m_transform;
}

void Datapicker::init() {
	m_image = new DatapickerImage(i18n("Plot"));
	m_image->setHidden(true);
	setUndoAware(false);
	addChild(m_image);
	setUndoAware(true);

	connect(m_image, &DatapickerImage::statusInfo, this, &Datapicker::statusInfo);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon Datapicker::icon() const {
	return QIcon::fromTheme(QLatin1String("color-picker-black"));
}

/*!
 * Returns a new context menu. The caller takes ownership of the menu.
 */
QMenu* Datapicker::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	m_image->createContextMenu(menu);
	return menu;
}

QWidget* Datapicker::view() const {
	if (!m_partView) {
		m_view = new DatapickerView(const_cast<Datapicker*>(this));
		m_partView = m_view;
		connect(this, &Datapicker::viewAboutToBeDeleted, [this]() {
			m_view = nullptr;
		});
	}
	return m_partView;
}

bool Datapicker::exportView() const {
	auto* s = currentSpreadsheet();
	if (s)
		return s->exportView();
	else
		return m_image->exportView();
}

bool Datapicker::printView() {
	auto* s = currentSpreadsheet();
	if (s)
		return s->printView();
	else
		return m_image->printView();
}

bool Datapicker::printPreview() const {
	auto* s = currentSpreadsheet();
	if (s)
		return s->printPreview();
	else
		return m_image->printPreview();
}

DatapickerCurve* Datapicker::activeCurve() {
	return m_activeCurve;
}

Spreadsheet* Datapicker::currentSpreadsheet() const {
	if (!m_view)
		return nullptr;

	const int index = m_view->currentIndex();
	if (index > 0) {
		auto* curve = child<DatapickerCurve>(index - 1);
		return curve->child<Spreadsheet>(0);
	}
	return nullptr;
}

DatapickerImage* Datapicker::image() const {
	return m_image;
}

DatapickerImageView* Datapicker::imageView() const {
	return reinterpret_cast<DatapickerImageView*>(m_image->view());
}

/*!
	this slot is called when a datapicker child is selected in the project explorer.
	emits \c datapickerItemSelected() to forward this event to the \c DatapickerView
	in order to select the corresponding tab.
 */
void Datapicker::childSelected(const AbstractAspect* aspect) {
	Q_ASSERT(aspect);
	m_activeCurve = dynamic_cast<DatapickerCurve*>(const_cast<AbstractAspect*>(aspect));

	int index = -1;
	if (m_activeCurve) {
		// if one of the curves is currently selected, select the image with the plot (the very first child)
		index = 0;
		Q_EMIT statusInfo(i18n("%1, active curve \"%2\"", this->name(), m_activeCurve->name()));
		Q_EMIT requestUpdateActions();
	} else if (aspect) {
		const auto* curve = aspect->ancestor<const DatapickerCurve>();
		index = indexOfChild<AbstractAspect>(curve);
		++index; //+1 because of the hidden plot image being the first child and shown in the first tab in the view
	}

	Q_EMIT datapickerItemSelected(index);
}

/*!
	this slot is called when a worksheet element is deselected in the project explorer.
 */
void Datapicker::childDeselected(const AbstractAspect* aspect) {
	Q_UNUSED(aspect);
}

/*!
 *  Emits the signal to select or to deselect the datapicker item (spreadsheet or image) with the index \c index
 *  in the project explorer, if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 *  This function is called in \c DatapickerView when the current tab was changed
 */
void Datapicker::setChildSelectedInView(int index, bool selected) {
	// select/deselect the datapicker itself if the first tab "representing" the plot image and the curves was selected in the view
	if (index == 0) {
		if (selected)
			Q_EMIT childAspectSelectedInView(this);
		else {
			Q_EMIT childAspectDeselectedInView(this);

			// deselect also all curves (they don't have any tab index in the view) that were potentially selected before
			for (const auto* curve : children<const DatapickerCurve>())
				Q_EMIT childAspectDeselectedInView(curve);
		}

		return;
	}

	--index; //-1 because of the first tab in the view being reserved for the plot image and curves

	// select/deselect the data spreadhseets
	auto spreadsheets = children<const Spreadsheet>(ChildIndexFlag::Recursive);
	const AbstractAspect* aspect = spreadsheets.at(index);
	if (selected) {
		Q_EMIT childAspectSelectedInView(aspect);

		// deselect the datapicker in the project explorer, if a child (spreadsheet or image) was selected.
		// prevents unwanted multiple selection with datapicker if it was selected before.
		Q_EMIT childAspectDeselectedInView(this);
	} else {
		Q_EMIT childAspectDeselectedInView(aspect);

		// deselect also all children that were potentially selected before (columns of a spreadsheet)
		for (const auto* child : aspect->children<const AbstractAspect>())
			Q_EMIT childAspectDeselectedInView(child);
	}
}

/*!
	Selects or deselects the datapicker or its current active curve in the project explorer.
	This function is called in \c DatapickerImageView.
*/
void Datapicker::setSelectedInView(const bool b) {
	if (b)
		Q_EMIT childAspectSelectedInView(this);
	else
		Q_EMIT childAspectDeselectedInView(this);
}

/*!
 * \brief Datapicker::addNewPoint
 * \param pos position in scene coordinates
 * \param parentAspect
 */
void Datapicker::addNewPoint(QPointF pos, AbstractAspect* parentAspect) {
	auto points = parentAspect->children<DatapickerPoint>(ChildIndexFlag::IncludeHidden);

	auto* newPoint = new DatapickerPoint(i18n("Point %1", points.count() + 1));
	newPoint->setHidden(true);

	beginMacro(i18n("%1: add %2", parentAspect->name(), newPoint->name()));
	if (parentAspect->addChild(newPoint)) {
		const QPointF oldPos = newPoint->position();
		newPoint->setPosition(pos);
		if (oldPos == pos) {
			// Just if pos == oldPos, setPosition will not trigger retransform() then
			newPoint->retransform();
		}

		auto* datapickerCurve = static_cast<DatapickerCurve*>(parentAspect);
		if (m_image == parentAspect)
			newPoint->setIsReferencePoint(true);
		else if (datapickerCurve)
			newPoint->initErrorBar(datapickerCurve->curveErrorTypes());
	} else
		delete newPoint;
	endMacro();
	Q_EMIT requestUpdateActions();
}

bool Datapicker::xDateTime() const {
	return m_image->axisPoints().datetime;
}

Vector3D Datapicker::mapSceneToLogical(QPointF point) const {
	return m_transform->mapSceneToLogical(point, m_image->axisPoints());
}

Vector3D Datapicker::mapSceneLengthToLogical(QPointF point) const {
	return m_transform->mapSceneLengthToLogical(point, m_image->axisPoints());
}

void Datapicker::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* curve = qobject_cast<const DatapickerCurve*>(aspect);
	if (curve) {
		// clear scene
		const auto& points = curve->children<DatapickerPoint>(ChildIndexFlag::IncludeHidden);
		for (auto* point : points)
			handleChildAspectAboutToBeRemoved(point);

		if (curve == m_activeCurve) {
			m_activeCurve = nullptr;
			Q_EMIT statusInfo(QString());
		}
	} else
		handleChildAspectAboutToBeRemoved(aspect);

	Q_EMIT requestUpdateActions();
}

void Datapicker::handleAspectAdded(const AbstractAspect* aspect) {
	const auto* addedPoint = qobject_cast<const DatapickerPoint*>(aspect);
	const auto* curve = qobject_cast<const DatapickerCurve*>(aspect);
	if (addedPoint)
		handleChildAspectAdded(addedPoint);
	else if (curve) {
		const auto count = childCount<DatapickerCurve>(AbstractAspect::ChildIndexFlag::IncludeHidden);
		curve->symbol()->setColor(themeColorPalette(count - 1));
		connect(m_image, &DatapickerImage::axisPointsChanged, curve, &DatapickerCurve::updatePoints);
		connect(m_image, &DatapickerImage::axisPointsRemoved, curve, &DatapickerCurve::updatePoints);
		auto points = curve->children<DatapickerPoint>(ChildIndexFlag::IncludeHidden);
		for (auto* point : points)
			handleChildAspectAdded(point);
	} else
		return;

	qreal zVal = 0;
	const auto& points = m_image->children<DatapickerPoint>(ChildIndexFlag::IncludeHidden);
	for (auto* point : points)
		point->graphicsItem()->setZValue(zVal++);

	for (const auto* curve : children<DatapickerCurve>()) {
		for (auto* point : curve->children<DatapickerPoint>(ChildIndexFlag::IncludeHidden))
			point->graphicsItem()->setZValue(zVal++);
	}

	Q_EMIT requestUpdateActions();
}

void Datapicker::handleChildAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* removedPoint = qobject_cast<const DatapickerPoint*>(aspect);
	if (removedPoint) {
		QGraphicsItem* item = removedPoint->graphicsItem();
		Q_ASSERT(item != nullptr);
		Q_ASSERT(m_image != nullptr);
		m_image->scene()->removeItem(item);
	}
}

void Datapicker::handleChildAspectAdded(const AbstractAspect* aspect) {
	const auto* addedPoint = qobject_cast<const DatapickerPoint*>(aspect);
	if (addedPoint) {
		QGraphicsItem* item = addedPoint->graphicsItem();
		Q_ASSERT(item != nullptr);
		Q_ASSERT(m_image != nullptr);
		m_image->scene()->addItem(item);
	}
}

void Datapicker::setColorPalette(const KConfig& config) {
	if (config.hasGroup(QStringLiteral("Theme"))) {
		KConfigGroup group = config.group(QStringLiteral("Theme"));

		// read the five colors defining the palette
		m_themeColorPalette.clear();
		m_themeColorPalette.append(group.readEntry(QStringLiteral("ThemePaletteColor1"), QColor()));
		m_themeColorPalette.append(group.readEntry(QStringLiteral("ThemePaletteColor2"), QColor()));
		m_themeColorPalette.append(group.readEntry(QStringLiteral("ThemePaletteColor3"), QColor()));
		m_themeColorPalette.append(group.readEntry(QStringLiteral("ThemePaletteColor4"), QColor()));
		m_themeColorPalette.append(group.readEntry(QStringLiteral("ThemePaletteColor5"), QColor()));
	}
}

QColor Datapicker::themeColorPalette(int index) const {
	const int i = index % m_themeColorPalette.count();
	return m_themeColorPalette.at(i);
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void Datapicker::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("datapicker"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// serialize all children
	for (auto* child : children<AbstractAspect>(ChildIndexFlag::IncludeHidden))
		child->save(writer);

	writer->writeEndElement(); // close "datapicker" section
}

//! Load from XML
bool Datapicker::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("datapicker"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == QLatin1String("datapickerImage")) {
			auto* plot = new DatapickerImage(i18n("Plot"), true);
			if (!plot->load(reader, preview)) {
				delete plot;
				return false;
			} else {
				plot->setHidden(true);
				addChild(plot);
				m_image = plot;
			}
		} else if (reader->name() == QLatin1String("datapickerCurve")) {
			auto* curve = new DatapickerCurve(QString());
			if (!curve->load(reader, preview)) {
				delete curve;
				return false;
			} else
				addChild(curve);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown datapicker element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	for (auto* aspect : children<AbstractAspect>(ChildIndexFlag::IncludeHidden)) {
		for (auto* point : aspect->children<DatapickerPoint>(ChildIndexFlag::IncludeHidden))
			handleAspectAdded(point);
	}

	for (auto* curve : children<DatapickerCurve>(ChildIndexFlag::IncludeHidden))
		curve->updatePoints();

	return true;
}
