/***************************************************************************
    File                 : CantorWorksheet.cpp
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)

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

#include "CantorWorksheet.h"
#include "commonfrontend/cantorWorksheet/CantorWorksheetView.h"
#include "VariableParser.h"

#include <QDebug>
#include <KLocalizedString>
#include <KMessageBox>
#include <QGraphicsView>

CantorWorksheet::CantorWorksheet(AbstractScriptingEngine* engine, const QString &name, bool loading)
		: AbstractPart(name), scripted(engine), m_part(0), m_backendName(name){
	if(!loading)
		init();
}

/*!
	initializes Cantor's part and plugins
*/
bool CantorWorksheet::init(QByteArray* content) {
	KPluginFactory* factory = KPluginLoader(QLatin1String("libcantorpart")).factory();
	if (factory) {
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		m_part = factory->create<KParts::ReadWritePart>(this, QVariantList()<<m_backendName<<"--noprogress");
		// CantorPart* m_cantorPart = dynamic_cast<CantorPart*>(m_part);
		if (!m_part) {
			KMessageBox::error(view(), i18n("Could not create the Cantor Part."));
			return false;
		}
		m_worksheetAccess = m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);
		connect(m_worksheetAccess, SIGNAL(sessionChanged()), this, SIGNAL(sessionChanged()));
		if(content)
			m_worksheetAccess->loadWorksheetFromByteArray(content);
		Cantor::PanelPluginHandler* handler = m_part->findChild<Cantor::PanelPluginHandler*>(QLatin1String("PanelPluginHandler"));
		if(!handler) {
			KMessageBox::error(view(), i18n("no PanelPluginHandle found for the Cantor Part."));
			return false;
		}
		m_plugins = handler->plugins();
		foreach(Cantor::PanelPlugin* plugin, m_plugins) {
			if(plugin->name() == "Variable Manager") {
				Cantor::PanelPlugin* m_variablemgr = plugin;
				m_session = m_variablemgr->session();
				m_backendName = m_session->backend()->name();
				m_variableModel = m_session->variableModel();
				connect(m_variableModel, SIGNAL(rowsInserted(const QModelIndex, int, int)), this, SLOT(rowsInserted(const QModelIndex, int, int)));
				connect(m_variableModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex, int, int)), this, SLOT(rowsAboutToBeRemoved(const QModelIndex, int, int)));
				connect(m_variableModel, SIGNAL(modelReset()), this, SLOT(modelReset()));
				break;
			}
		}
	}
	else {
		// if we couldn't find our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(view(), i18n("Could not find the Cantor Part."));
		// we return here, cause qApp->quit() only means "exit the
		// next time we enter the event loop...
		return false;
	}
	return true;
}

//SLots
void CantorWorksheet::rowsInserted(const QModelIndex & parent, int first, int last) {
	qDebug() << "Inserting Row";
	for(int i = first; i <= last; ++i) {
		QString name = m_variableModel->data(m_variableModel->index(first, 0)).toString();
		QString value = m_variableModel->data(m_variableModel->index(first, 1)).toString();
		qDebug() << "Variable value: " << value;
		VariableParser* parser = new VariableParser(m_backendName, value);
		if(parser->isParsed()) {
			Column * new_col = new Column(name, AbstractColumn::Numeric);
			new_col->setUndoAware(false);
			new_col->insertRows(0, parser->valuesCount());
			new_col->replaceValues(0, parser->values());
			insertChildBefore(new_col, 0);
		}
		delete(parser);
    }
}

void CantorWorksheet::sessionChanged() {

}

void CantorWorksheet::modelReset() {
	qDebug() << "Model Reset";
	for(int i = 0; i < columnCount(); ++i) {
		column(i)->remove();
	}
}

void CantorWorksheet::rowsAboutToBeRemoved(const QModelIndex & parent, int first, int last) {
	qDebug() << "Removing Row";
	for(int i = first; i <= last; ++i) {
		QString name = m_variableModel->data(m_variableModel->index(first, 0)).toString();
		if(column(name)) column(name)->remove();
	}
}

QList<Cantor::PanelPlugin*> CantorWorksheet::getPlugins(){
	return m_plugins;
}

KParts::ReadWritePart* CantorWorksheet::part() {
	return m_part;
}

QIcon CantorWorksheet::icon() const {
	if(m_session)
		return QIcon::fromTheme(m_session->backend()->icon());
	return QIcon();
}

Column* CantorWorksheet::column(const QString &name) const{
	return child<Column>(name);
}

Column* CantorWorksheet::column(int& index) const{
	return child<Column>(index);
}

int CantorWorksheet::columnCount() const{
	return childCount<Column>();
}

QWidget* CantorWorksheet::view() const {
	if (!m_view) {
		m_view = new CantorWorksheetView(const_cast<CantorWorksheet*>(this));
		m_view->setBaseSize(1500, 1500);
		// 	connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));
	}
	return m_view;
}

//! Return a new context menu.
/**
 * The caller takes ownership of the menu.
 */
QMenu* CantorWorksheet::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	emit requestProjectContextMenu(menu);
	return menu;
}

QString CantorWorksheet::backendName() {
	return this->m_backendName;
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void CantorWorksheet::save(QXmlStreamWriter* writer) const{
	writer->writeStartElement("cantorWorksheet");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	QByteArray content = m_worksheetAccess->saveWorksheetToByteArray();
	writer->writeStartElement("cantor");
	writer->writeAttribute("content", content.toBase64());
	writer->writeEndElement();
	writer->writeEndElement(); // close "cantorWorksheet" section
}

//! Load from XML
bool CantorWorksheet::load(XmlStreamReader* reader){
	if(!reader->isStartElement() || reader->name() != "cantorWorksheet"){
		reader->raiseError(i18n("no cantor worksheet element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;
	QRectF rect;

	while (!reader->atEnd()){
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "cantorWorksheet")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment"){
			if (!readCommentElement(reader)) return false;
		} else if (reader->name() == "cantor"){
			attribs = reader->attributes();

			str = attribs.value("content").toString().trimmed();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'content'"));
			QByteArray content = QByteArray::fromBase64(str.toAscii());
			return init(&content);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}
	return true;
}
