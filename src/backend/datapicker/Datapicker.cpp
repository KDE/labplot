/***************************************************************************
    File                 : Datapicker.cpp
    Project              : LabPlot
    Description          : Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2016 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "Datapicker.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/datapicker/DatapickerView.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/datapicker/Transform.h"
#include "backend/datapicker/DatapickerPoint.h"

#include <QGraphicsScene>
#include "QIcon"
#include <KLocalizedString>

/**
 * \class Datapicker
 * \brief Top-level container for DatapickerCurve and DatapickerImage.
 * \ingroup backend
 */
Datapicker::Datapicker(const QString& name, const bool loading)
: AbstractPart(name, AspectType::Datapicker), m_transform(new Transform())
{
	connect(this, &Datapicker::aspectAdded, this, &Datapicker::handleAspectAdded);
	connect(this, &Datapicker::aspectAboutToBeRemoved, this, &Datapicker::handleAspectAboutToBeRemoved);

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
	return QIcon::fromTheme("color-picker-black");
}

/*!
 * Returns a new context menu. The caller takes ownership of the menu.
 */
QMenu* Datapicker::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	m_image->createContextMenu(menu);
	return menu;
}

QWidget* Datapicker::view() const {
	if (!m_partView) {
		m_view = new DatapickerView(const_cast<Datapicker*>(this));
		m_partView = m_view;
	}
	return m_partView;
}


bool Datapicker::exportView() const {
	Spreadsheet* s = currentSpreadsheet();
	bool ret;
	if (s)
		ret = s->exportView();
	else
		ret = m_image->exportView();
	return ret;
}

bool Datapicker::printView() {
	Spreadsheet* s = currentSpreadsheet();
	bool ret;
	if (s)
		ret = s->printView();
	else
		ret = m_image->printView();
	return ret;
}

bool Datapicker::printPreview() const {
	Spreadsheet* s = currentSpreadsheet();
	bool ret;
	if (s)
		ret = s->printPreview();
	else
		ret = m_image->printPreview();
	return ret;
}

DatapickerCurve* Datapicker::activeCurve() {
	return m_activeCurve;
}

Spreadsheet* Datapicker::currentSpreadsheet() const {
	if (!m_view)
		return nullptr;

	const int index = m_view->currentIndex();
	if (index > 0) {
		auto* curve = child<DatapickerCurve>(index-1);
		return curve->child<Spreadsheet>(0);
	}
	return nullptr;
}

DatapickerImage* Datapicker::image() const {
	return m_image;
}

/*!
    this slot is called when a datapicker child is selected in the project explorer.
    emits \c datapickerItemSelected() to forward this event to the \c DatapickerView
    in order to select the corresponding tab.
 */
void Datapicker::childSelected(const AbstractAspect* aspect) {
	m_activeCurve = dynamic_cast<DatapickerCurve*>(const_cast<AbstractAspect*>(aspect));

	int index = -1;
	if (m_activeCurve) {
		//if one of the curves is currently selected, select the image with the plot (the very first child)
		index = 0;
		emit statusInfo(i18n("%1, active curve \"%2\"", this->name(), m_activeCurve->name()));
		emit requestUpdateActions();
	} else {
		const auto* curve = aspect->ancestor<const DatapickerCurve>();
		index = indexOfChild<AbstractAspect>(curve);
		++index; //+1 because of the hidden plot image being the first child and shown in the first tab in the view
	}

	emit datapickerItemSelected(index);
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
	//select/deselect the datapicker itself if the first tab "representing" the plot image and the curves was selected in the view
	if (index == 0) {
		if (selected)
			emit childAspectSelectedInView(this);
		else {
			emit childAspectDeselectedInView(this);

			//deselect also all curves (they don't have any tab index in the view) that were potentially selected before
			for (const auto* curve : children<const DatapickerCurve>())
				emit childAspectDeselectedInView(curve);
		}

		return;
	}

	--index; //-1 because of the first tab in the view being reserved for the plot image and curves

	//select/deselect the data spreadhseets
	QVector<const Spreadsheet*> spreadsheets = children<const Spreadsheet>(AbstractAspect::Recursive);
	const AbstractAspect* aspect = spreadsheets.at(index);
	if (selected) {
		emit childAspectSelectedInView(aspect);

		//deselect the datapicker in the project explorer, if a child (spreadsheet or image) was selected.
		//prevents unwanted multiple selection with datapicker if it was selected before.
		emit childAspectDeselectedInView(this);
	} else {
		emit childAspectDeselectedInView(aspect);

		//deselect also all children that were potentially selected before (columns of a spreadsheet)
		for (const auto* child : aspect->children<const AbstractAspect>())
			emit childAspectDeselectedInView(child);
	}
}

/*!
	Selects or deselects the datapicker or its current active curve in the project explorer.
    This function is called in \c DatapickerImageView.
*/
void Datapicker::setSelectedInView(const bool b) {
	if (b)
		emit childAspectSelectedInView(this);
	else
		emit childAspectDeselectedInView(this);
}

void Datapicker::addNewPoint(const QPointF& pos, AbstractAspect* parentAspect) {
	QVector<DatapickerPoint*> childPoints = parentAspect->children<DatapickerPoint>(AbstractAspect::IncludeHidden);
	if (childPoints.isEmpty())
		beginMacro(i18n("%1: add new point", parentAspect->name()));
	else
		beginMacro(i18n("%1: add new point %2", parentAspect->name(), childPoints.count()));

	auto* newPoint = new DatapickerPoint(i18n("%1 Point", parentAspect->name()));
	newPoint->setPosition(pos);
	newPoint->setHidden(true);
	parentAspect->addChild(newPoint);
	newPoint->retransform();

	auto* datapickerCurve = dynamic_cast<DatapickerCurve*>(parentAspect);
	if (m_image == parentAspect) {
		DatapickerImage::ReferencePoints points = m_image->axisPoints();
		points.scenePos[childPoints.count()].setX(pos.x());
		points.scenePos[childPoints.count()].setY(pos.y());
		m_image->setAxisPoints(points);
	} else if (datapickerCurve) {
		newPoint->initErrorBar(datapickerCurve->curveErrorTypes());
		datapickerCurve->updateData(newPoint);
	}

	endMacro();
	emit requestUpdateActions();
}

QVector3D Datapicker::mapSceneToLogical(const QPointF& point) const {
	return m_transform->mapSceneToLogical(point, m_image->axisPoints());
}

QVector3D Datapicker::mapSceneLengthToLogical(const QPointF& point) const {
	return m_transform->mapSceneLengthToLogical(point, m_image->axisPoints());
}

void Datapicker::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* curve = qobject_cast<const DatapickerCurve*>(aspect);
	if (curve) {
		//clear scene
		QVector<DatapickerPoint*> childPoints = curve->children<DatapickerPoint>(IncludeHidden);
		for (auto* point : childPoints)
			handleChildAspectAboutToBeRemoved(point);

		if (curve == m_activeCurve) {
			m_activeCurve = nullptr;
			emit statusInfo(QString());
		}
	} else
		handleChildAspectAboutToBeRemoved(aspect);

	emit requestUpdateActions();
}

void Datapicker::handleAspectAdded(const AbstractAspect* aspect) {
	const auto* addedPoint = qobject_cast<const DatapickerPoint*>(aspect);
	const auto* curve = qobject_cast<const DatapickerCurve*>(aspect);
	if (addedPoint)
		handleChildAspectAdded(addedPoint);
	else if (curve) {
		QVector<DatapickerPoint*> childPoints = curve->children<DatapickerPoint>(IncludeHidden);
		for (auto* point : childPoints)
			handleChildAspectAdded(point);
	} else
		return;

	qreal zVal = 0;
	QVector<DatapickerPoint*> childPoints = m_image->children<DatapickerPoint>(IncludeHidden);
	for (auto* point : childPoints)
		point->graphicsItem()->setZValue(zVal++);

	for (const auto* curve : children<DatapickerCurve>()) {
		for (auto* point : curve->children<DatapickerPoint>(IncludeHidden))
			point->graphicsItem()->setZValue(zVal++);
	}

	emit requestUpdateActions();
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

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void Datapicker::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement( "datapicker" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//serialize all children
	for (auto* child : children<AbstractAspect>(IncludeHidden))
		child->save(writer);

	writer->writeEndElement(); // close "datapicker" section
}

//! Load from XML
bool Datapicker::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "datapicker")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "datapickerImage") {
			DatapickerImage* plot = new DatapickerImage(i18n("Plot"), true);
			if (!plot->load(reader, preview)) {
				delete plot;
				return false;
			} else {
				plot->setHidden(true);
				addChild(plot);
				m_image = plot;
			}
		} else if (reader->name() == "datapickerCurve") {
			DatapickerCurve* curve = new DatapickerCurve(QString());
			if (!curve->load(reader, preview)) {
				delete curve;
				return false;
			} else
				addChild(curve);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown datapicker element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	for (auto* aspect : children<AbstractAspect>(IncludeHidden)) {
		for (auto* point : aspect->children<DatapickerPoint>(IncludeHidden))
			handleAspectAdded(point);
	}

	return true;
}
