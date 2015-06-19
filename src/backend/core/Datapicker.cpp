#include "Datapicker.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Image.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/datapicker/DatapickerView.h"

#include "KIcon"
#include <KLocale>
#include <QDebug>

Datapicker::Datapicker(AbstractScriptingEngine* engine, const QString& name)
        : AbstractPart(name), scripted(engine), m_datasheet(0), m_image(0){
}

QIcon Datapicker::icon() const {
    return KIcon("");
}

/*!
 * Returns a new context menu. The caller takes ownership of the menu.
 */
QMenu* Datapicker::createContextMenu() {
    QMenu *menu = AbstractPart::createContextMenu();
    Q_ASSERT(menu);
    return menu;
}

QWidget* Datapicker::view() const {
    if (!m_view) {
        m_view = new DatapickerView(const_cast<Datapicker*>(this));
    }
    return m_view;
}

void Datapicker::initDefault() {
    m_image = new Image(0, i18n("Image"));
    addChild(m_image);
    initImageConnections();
}

void Datapicker::initImageConnections() {
    connect(m_image, SIGNAL(addDataToSheet(QPointF,int)),
            this, SLOT(addDataToDatasheet(QPointF,int)));
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

/*!
    this slot is called when a Datapicker child is selected in the project explorer.
    emits \c DatapickerItemSelected() to forward this event to the \c DatapickerView
    in order to select the corresponding tab.
 */
void Datapicker::childSelected(const AbstractAspect* aspect){
    int index = indexOfChild<AbstractAspect>(aspect);
    emit datapickerItemSelected(index);
}

/*!
    this slot is called when a worksheet element is deselected in the project explorer.
 */
void Datapicker::childDeselected(const AbstractAspect* aspect){
    Q_UNUSED(aspect);
    //TODO: do we need this slot?
}

/*!
 *  Emits the signal to select or to deselect the Datapicker item (spreadsheet or image) with the index \c index
 *  in the project explorer, if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 *  This function is called in \c DatapickerView when the current tab was changed
 */
void Datapicker::setChildSelectedInView(int index, bool selected){
    AbstractAspect* aspect = child<AbstractAspect>(index);
    if (selected) {
        emit childAspectSelectedInView(aspect);

        //deselect the Datapicker in the project explorer, if a child (spreadsheet or image) was selected.
        //prevents unwanted multiple selection with Datapicker if it was selected before.
        emit childAspectDeselectedInView(this);
    } else {
        emit childAspectDeselectedInView(aspect);

        //deselect also all children that were potentially selected before (columns of a spreadsheet)
        foreach(AbstractAspect* child, aspect->children<AbstractAspect>())
            emit childAspectDeselectedInView(child);
    }
}

void Datapicker::addDataToDatasheet(const QPointF& data, int index){
    //add spreadsheet if its not present
    //change code
    int count = childCount<Spreadsheet>();
    if (!count) {
        m_datasheet = new Spreadsheet(0, i18n("Data"));
        addChild(m_datasheet);
    }

    //add column if its not present
    if(m_datasheet->columnCount() < 2) {
        m_datasheet->appendColumns(2 - m_datasheet->columnCount());
    }

    //add row if its not present
    if(m_datasheet->rowCount() < index) {
        m_datasheet->appendRows(index - m_datasheet->rowCount());
    }

    m_datasheet->setUndoAware(false);
    m_datasheet->column(0)->setValueAt(index, data.x());
    m_datasheet->column(1)->setValueAt(index, data.y());
    m_datasheet->setUndoAware(true);
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
    foreach(AbstractAspect* aspect, children<AbstractAspect>())
        aspect->save(writer);

    writer->writeEndElement(); // close "worksheet" section
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

        if(reader->name() == "spreadsheet"){
            Spreadsheet* spreadsheet = new Spreadsheet(0, "spreadsheet", true);
            if (!spreadsheet->load(reader)){
                delete spreadsheet;
                return false;
            }else{
                addChild(spreadsheet);
                m_datasheet = spreadsheet;
            }
        } else if (reader->name() == "image"){
            Image* image = new Image(0, i18n("image"));
            if (!image->load(reader)){
                delete image;
                return false;
            }else{
                addChild(image);
                m_image = image;
            }
        }else{ // unknown element
            reader->raiseWarning(i18n("unknown datapicker element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    initImageConnections();
    return true;
}
