#include "Datapicker.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/datapicker/DatapickerView.h"

#include "KIcon"
#include <KLocale>
#include <QDebug>

Datapicker::Datapicker(AbstractScriptingEngine* engine, const QString& name)
		: AbstractPart(name), scripted(engine){
}

void Datapicker::initDefault() {
    Spreadsheet* spreadsheet = new Spreadsheet(0,i18n("datasheet"));
    addChild(spreadsheet);
    Worksheet* worksheet = new Worksheet(0, i18n("Worksheet"));
    worksheet->setDatapicker(true);
    addChild(worksheet);
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
    //emit requestProjectContextMenu(menu);
	return menu;
}

QWidget* Datapicker::view() const {
	if (!m_view) {
        m_view = new DatapickerView(const_cast<Datapicker*>(this));
		m_view->resize(200,200);
	}
	return m_view;
}

Spreadsheet* Datapicker::currentSpreadsheet() const {
	if (!m_view)
		return 0;

    int index = reinterpret_cast<const DatapickerView*>(m_view)->currentIndex();
	if(index != -1) {
		AbstractAspect* aspect = child<AbstractAspect>(index);
		if(aspect->inherits("Spreadsheet"))
			return dynamic_cast<Spreadsheet*>(aspect);
	}
	return 0;
}

Worksheet* Datapicker::currentWorksheet() const {
	if (!m_view)
		return 0;

    int index = reinterpret_cast<const DatapickerView*>(m_view)->currentIndex();
	if(index != -1) {
		AbstractAspect* aspect = child<AbstractAspect>(index);
        if(aspect->inherits("Worksheet"))
            return dynamic_cast<Worksheet*>(aspect);
	}
	return 0;
}

void Datapicker::childSelected(const AbstractAspect* aspect){
	int index = indexOfChild<AbstractAspect>(aspect);
    emit datapickerItemSelected(index);
}

void Datapicker::childDeselected(const AbstractAspect* aspect){
	Q_UNUSED(aspect);
}

void Datapicker::setChildSelectedInView(int index, bool selected){
	if (selected) {
		emit childAspectSelectedInView(child<AbstractAspect>(index));
		emit childAspectDeselectedInView(this);
	} else {
		emit childAspectDeselectedInView(child<AbstractAspect>(index));
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

    writer->writeEndElement();
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
            Spreadsheet* spreadsheet = new Spreadsheet(0, "spreadsheet");
            if (!spreadsheet->load(reader)){
                delete spreadsheet;
                return false;
            }else{
                addChild(spreadsheet);
            }
        } else if (reader->name() == "worksheet"){
            Worksheet* worksheet = new Worksheet(0, i18n("worksheet"));
            if (!worksheet->load(reader)){
                delete worksheet;
                return false;
            }else{
                addChild(worksheet);
            }
        }else{ // unknown element
            reader->raiseWarning(i18n("unknown datapicker element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    return true;
}
