/***************************************************************************
    File                 : Datapicker.cpp
    Project              : LabPlot
    Description          : Aspect providing a container for storing image and data
                           in form of worksheet and spreadsheets
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)

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
#include "backend/datapicker/Image.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/datapicker/DatapickerView.h"
#include "backend/datapicker/DataPickerCurve.h"
#include "backend/datapicker/Transform.h"

#include "KIcon"
#include <KLocale>

/**
 * \class Datapicker
 * \brief Top-level container for DataPickerCurve and Image.
 * \ingroup backend
 */
Datapicker::Datapicker(AbstractScriptingEngine* engine, const QString& name, const bool loading)
    : AbstractPart(name), scripted(engine), activeCurve(0), m_transform(new Transform()), m_image(0) {

    connect( this, SIGNAL(aspectAdded(const AbstractAspect*)),
             this, SLOT(handleChildAspectAdded(const AbstractAspect*)) );
    connect( this, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
             this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)) );

	if (!loading)
		init();
}

void Datapicker::init() {
    m_image = new Image(0, i18n("Plot"));
    m_image->setHidden(true);
    setUndoAware(false);
    addChild(m_image);
    setUndoAware(true);
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon Datapicker::icon() const {
    return KIcon("color-picker-black");
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
    if (!m_view) {
        m_view = new DatapickerView(const_cast<Datapicker*>(this));
    }
    return m_view;
}

Spreadsheet* Datapicker::currentSpreadsheet() const {
    if (!m_view)
        return 0;

    int index = reinterpret_cast<const DatapickerView*>(m_view)->currentIndex();
    if(index != -1) {
        AbstractAspect* aspect = child<AbstractAspect>(index);
        return dynamic_cast<Spreadsheet*>(aspect);
    }
    return 0;
}

Image* Datapicker::currentImage() const {
    if (!m_view)
        return 0;

    int index = reinterpret_cast<const DatapickerView*>(m_view)->currentIndex();
    if(index != -1) {
        AbstractAspect* aspect = child<AbstractAspect>(index);
        return dynamic_cast<Image*>(aspect);
    }
    return 0;
}

Image* Datapicker::image() const {
	return m_image;
}

/*!
    this slot is called when a datapicker child is selected in the project explorer.
    emits \c datapickerItemSelected() to forward this event to the \c DatapickerView
    in order to select the corresponding tab.
 */
void Datapicker::childSelected(const AbstractAspect* aspect) {
    activeCurve = dynamic_cast<DataPickerCurve*>(const_cast<AbstractAspect*>(aspect));

    int index = -1;
    if (activeCurve) {
        //if one of the curves is currently selected, select the image with the plot (the very first child)
        index = 0;
    } else {
	const DataPickerCurve* curve = aspect->ancestor<const DataPickerCurve>();
        index= indexOfChild<AbstractAspect>(curve);
		++index; //+1 because of the hidden plot image being the first child and shown in the first tab in the view
    }

    emit datapickerItemSelected(index);
}

/*!
    this slot is called when a worksheet element is deselected in the project explorer.
 */
void Datapicker::childDeselected(const AbstractAspect* aspect){
    Q_UNUSED(aspect);
}

/*!
 *  Emits the signal to select or to deselect the datapicker item (spreadsheet or image) with the index \c index
 *  in the project explorer, if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 *  This function is called in \c DatapickerView when the current tab was changed
 */
void Datapicker::setChildSelectedInView(int index, bool selected){
	//select the datapicker itself if the first item (plot image) was selected in the view
	if (index==0) {
		if (selected)
			emit childAspectSelectedInView(this);
		else
			emit childAspectDeselectedInView(this);

		return;
	}

    QList<const AbstractAspect*> allChildren = children<const AbstractAspect>(AbstractAspect::Recursive|AbstractAspect::IncludeHidden);
    const AbstractAspect* aspect = allChildren.at(index);
    if (selected) {
        emit childAspectSelectedInView(aspect);

        //deselect the datapicker in the project explorer, if a child (spreadsheet or image) was selected.
        //prevents unwanted multiple selection with datapicker if it was selected before.
        emit childAspectDeselectedInView(this);
    } else {
        emit childAspectDeselectedInView(aspect);

        //deselect also all children that were potentially selected before (columns of a spreadsheet)
        foreach(const AbstractAspect* child, aspect->children<const AbstractAspect>())
            emit childAspectDeselectedInView(child);
    }
}

QVector3D Datapicker::mapSceneToLogical(const QPointF& point) const {
    return m_transform->mapSceneToLogical(point, m_image->axisPoints());
}


QVector3D Datapicker::mapSceneLengthToLogical(const QPointF& point) const {
    return m_transform->mapSceneLengthToLogical(point, m_image->axisPoints());
}

void Datapicker::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
    const DataPickerCurve* curve = qobject_cast<const DataPickerCurve*>(aspect);
    if (curve) {
        //clear scene
        QList<WorksheetElement *> childElements = curve->children<WorksheetElement>(IncludeHidden);
        foreach(WorksheetElement *elem, childElements) {
            handleChildAspectAboutToBeRemoved(elem);
        }
    } else {
        handleChildAspectAboutToBeRemoved(aspect);
    }
}

void Datapicker::handleChildAspectAdded(const AbstractAspect* aspect) {
    const WorksheetElement* addedElement = qobject_cast<const WorksheetElement*>(aspect);
    if (addedElement) {
        QGraphicsItem *item = addedElement->graphicsItem();
        Q_ASSERT(item != NULL);
        Q_ASSERT(m_image != NULL);
        m_image->scene()->addItem(item);

        qreal zVal = 0;
        QList<WorksheetElement *> childElements = m_image->children<WorksheetElement>(IncludeHidden);
        foreach(WorksheetElement *elem, childElements) {
            elem->graphicsItem()->setZValue(zVal++);
        }

        foreach (DataPickerCurve* curve, children<DataPickerCurve>()) {
            foreach (WorksheetElement* elem, curve->children<WorksheetElement>(IncludeHidden)) {
                elem->graphicsItem()->setZValue(zVal++);
            }
        }
    }
}

void Datapicker::handleChildAspectAboutToBeRemoved(const AbstractAspect* aspect) {
    const WorksheetElement *removedElement = qobject_cast<const WorksheetElement*>(aspect);
    if (removedElement) {
        QGraphicsItem *item = removedElement->graphicsItem();
        Q_ASSERT(item != NULL);
        Q_ASSERT(m_image != NULL);
        m_image->scene()->removeItem(item);
    }
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void Datapicker::save(QXmlStreamWriter* writer) const{
    writer->writeStartElement( "datapicker" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);

    //serialize all children
    foreach(AbstractAspect* child, children<AbstractAspect>())
        child->save(writer);

    writer->writeEndElement(); // close "datapicker" section
}

//! Load from XML
bool Datapicker::load(XmlStreamReader* reader){
    if(!reader->isStartElement() || reader->name() != "datapicker"){
        reader->raiseError(i18n("no datapicker element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "datapicker")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "image") {
            Image* plot = new Image(0, i18n("Plot"), true);
            if (!plot->load(reader)){
                delete plot;
                return false;
            } else {
				plot->setHidden(true);
                addChild(plot);
                m_image = plot;
            }
        } else if (reader->name() == "dataPickerCurve") {
            DataPickerCurve* curve = new DataPickerCurve("");
            if (!curve->load(reader)){
                delete curve;
                return false;
            }else{
                addChild(curve);
            }
        } else { // unknown element
            reader->raiseWarning(i18n("unknown datapicker element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    foreach (AbstractAspect* aspect, children<AbstractAspect>()) {
        foreach (WorksheetElement* elem, aspect->children<WorksheetElement>(IncludeHidden)) {
            handleChildAspectAdded(elem);
        }
    }

    return true;
}
