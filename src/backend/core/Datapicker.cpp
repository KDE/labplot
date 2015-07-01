#include "Datapicker.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Image.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/datapicker/DatapickerView.h"

#include "KIcon"
#include <KLocale>
#include <QDebug>

Datapicker::Datapicker(AbstractScriptingEngine* engine, const QString& name)
    : AbstractPart(name), scripted(engine),
      m_datasheet(0),
      m_image(0),
      m_positionX(-1),
      m_positionY(-1),
      m_plusDeltaX(-1),
      m_minusDeltaX(-1),
      m_plusDeltaY(-1),
      m_minusDeltaY(-1) {
}

QIcon Datapicker::icon() const {
    return KIcon("color-picker-black");
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
    addDatasheet();
    m_image = new Image(0, i18n("Image"));
	setUndoAware(false);
    addChild(m_image);
	setUndoAware(true);
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

void Datapicker::addDataToSheet(double data, int row, const DataColumnType& type) {
    if (indexOfChild<Spreadsheet>(m_datasheet) == -1)
        addDatasheet();

    m_datasheet->setUndoAware(false);
    int *index = columnIndexFromType(type);
    Column* column;
    if (*index == -1 || m_datasheet->columnCount() <= *index) {
        column = new Column(columnNameFromType(type), AbstractColumn::Numeric);
        column->setPlotDesignation(AbstractColumn::Y);
        column->insertRows(0, m_datasheet->rowCount());
        m_datasheet->addChild(column);
        *index = m_datasheet->indexOfChild<Column>(column);
    } else {
        column = m_datasheet->child<Column>(*index);
    }

    column->setUndoAware(false);
    column->setValueAt(row, data);
    column->setUndoAware(true);
    m_datasheet->setUndoAware(true);
}

QString Datapicker::columnNameFromType(Datapicker::DataColumnType type) {
    QString name;
    if (type == Datapicker::PositionX)
        name = i18n("x");
    else if (type == Datapicker::PositionY)
        name = i18n("y");
    else if (type == Datapicker::PlusDeltaX)
        name = i18n("+delta_x");
    else if (type == Datapicker::MinusDeltaX)
        name = i18n("-delta_x");
    else if (type == Datapicker::PlusDeltaY)
        name = i18n("+delta_y");
    else if (type == Datapicker::MinusDeltaY)
        name = i18n("-delta_y");

    return name;
}

int *Datapicker::columnIndexFromType(Datapicker::DataColumnType type) {
    int *index;
    if (type == Datapicker::PositionX)
        index = &m_positionX;
    else if (type == Datapicker::PositionY)
        index = &m_positionY;
    else if (type == Datapicker::PlusDeltaX)
        index = &m_plusDeltaX;
    else if (type == Datapicker::MinusDeltaX)
        index = &m_minusDeltaX;
    else if (type == Datapicker::PlusDeltaY)
        index = &m_plusDeltaY;
    else
        index = &m_minusDeltaY;

    return index;
}

void Datapicker::addDatasheet() {
    m_datasheet = new Spreadsheet(0, i18n("Data"));
    m_datasheet->setUndoAware(false);
    m_datasheet->column(0)->setName("x");
    m_positionX = 0;
    m_datasheet->column(1)->setName("y");
    m_positionY = 1;
    addChild(m_datasheet);
    m_datasheet->setUndoAware(true);
    connect(m_datasheet, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
            this, SLOT(handleColumnRemoved(const AbstractAspect*)));
}

void Datapicker::handleColumnRemoved(const AbstractAspect* aspect) {
    const Column* column = qobject_cast<const Column*>(aspect);
    if (column){
        int removedIndex = m_datasheet->indexOfChild<Column>(column);
        for(int i = PositionX; i <= MinusDeltaY; i++) {
            int *index = columnIndexFromType(DataColumnType(i));
            if (*index == removedIndex)
                *index = -1;
            else if (*index > removedIndex)
                --*index;
        }
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
            } else {
                addChild(spreadsheet);
                m_datasheet = spreadsheet;
            }
        } else if (reader->name() == "image"){
            Image* image = new Image(0, i18n("image"));
            if (!image->load(reader)){
                delete image;
                return false;
            } else {
                addChild(image);
                m_image = image;
            }
        }else{ // unknown element
            reader->raiseWarning(i18n("unknown datapicker element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    return true;
}
