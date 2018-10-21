/***************************************************************************
    File                 : Workbook.h
    Project              : LabPlot
    Description          : Aspect providing a container for storing data
						   in form of spreadsheets and matrices
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Alexander Semke(alexander.semke@web.de)

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

#include "Workbook.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/workbook/WorkbookView.h"
#include "kdefrontend/spreadsheet/ExportSpreadsheetDialog.h"

#include <KLocalizedString>

/**
 * \class Workbook
 * \brief Top-level container for Spreadsheet and Matrix.
 * \ingroup backend
 */
Workbook::Workbook(const QString& name) : AbstractPart(name), m_view(nullptr) {
}

QIcon Workbook::icon() const {
	return QIcon::fromTheme("labplot-workbook");
}

/*!
 * Returns a new context menu. The caller takes ownership of the menu.
 */
QMenu* Workbook::createContextMenu() {
	QMenu *menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	emit requestProjectContextMenu(menu);
	return menu;
}

QWidget* Workbook::view() const {
	if (!m_partView) {
		m_view = new WorkbookView(const_cast<Workbook*>(this));
		m_partView = m_view;
	}
	return m_partView;
}

bool Workbook::exportView() const {
	Spreadsheet* s = currentSpreadsheet();
	bool ret = false;
	if (s)
		ret = s->exportView();
	else {
		Matrix* m = currentMatrix();
		if (m)
			ret = m->exportView();
	}
	return ret;
}

bool Workbook::printView() {
	Spreadsheet* s = currentSpreadsheet();
	bool ret = false;
	if (s)
		ret = s->printView();
	else {
		Matrix* m = currentMatrix();
		if (m)
			ret = m->printView();
	}
	return ret;
}

bool Workbook::printPreview() const {
	Spreadsheet* s = currentSpreadsheet();
	bool ret = false;
	if (s)
		ret = s->printPreview();
	else {
		Matrix* m = currentMatrix();
		if (m)
			ret = m->printPreview();
	}
	return ret;
}

Spreadsheet* Workbook::currentSpreadsheet() const {
	if (!m_view)
		return nullptr;

	int index = m_view->currentIndex();
	if(index != -1) {
		auto* aspect = child<AbstractAspect>(index);
		return dynamic_cast<Spreadsheet*>(aspect);
	}
	return nullptr;
}

Matrix* Workbook::currentMatrix() const {
	if (!m_view)
		return nullptr;

	int index = reinterpret_cast<const WorkbookView*>(m_view)->currentIndex();
	if(index != -1) {
		auto* aspect = child<AbstractAspect>(index);
		return dynamic_cast<Matrix*>(aspect);
	}
	return nullptr;
}

/*!
	this slot is called when a workbook child is selected in the project explorer.
	emits \c workbookItemSelected() to forward this event to the \c WorkbookView
	in order to select the corresponding tab.
 */
void Workbook::childSelected(const AbstractAspect* aspect) {
	int index = indexOfChild<AbstractAspect>(aspect);
	emit workbookItemSelected(index);
}

/*!
	this slot is called when a worksheet element is deselected in the project explorer.
 */
void Workbook::childDeselected(const AbstractAspect* aspect) {
	Q_UNUSED(aspect);
}

/*!
 *  Emits the signal to select or to deselect the workbook item (spreadsheet or matrix) with the index \c index
 *  in the project explorer, if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 *  This function is called in \c WorkbookView when the current tab was changed
 */
void Workbook::setChildSelectedInView(int index, bool selected) {
	auto* aspect = child<AbstractAspect>(index);
	if (selected) {
		emit childAspectSelectedInView(aspect);

		//deselect the workbook in the project explorer, if a child (spreadsheet or matrix) was selected.
		//prevents unwanted multiple selection with workbook if it was selected before.
		emit childAspectDeselectedInView(this);
	} else {
		emit childAspectDeselectedInView(aspect);

		//deselect also all children that were potentially selected before (columns of a spreadsheet)
		for (auto* child : aspect->children<AbstractAspect>())
			emit childAspectDeselectedInView(child);
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void Workbook::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement( "workbook" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//serialize all children
	for (auto* aspect : children<AbstractAspect>())
		aspect->save(writer);

	writer->writeEndElement(); // close "workbook" section
}

//! Load from XML
bool Workbook::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "workbook")
			break;

		if (!reader->isStartElement())
			continue;

		if(reader->name() == "spreadsheet") {
			Spreadsheet* spreadsheet = new Spreadsheet("spreadsheet", true);
			if (!spreadsheet->load(reader, preview)) {
				delete spreadsheet;
				return false;
			} else
				addChild(spreadsheet);
		} else if (reader->name() == "matrix") {
			Matrix* matrix = new Matrix(i18n("matrix"), true);
			if (!matrix->load(reader, preview)) {
				delete matrix;
				return false;
			} else
				addChild(matrix);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown workbook element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	return true;
}
