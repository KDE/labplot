/***************************************************************************
    File                 : Folder.cpp
    Project              : LabPlot
    Description          : Folder in a project
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 Knut Franke (knut.franke@gmx.de)

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

#include "backend/core/Folder.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/matrix/Matrix.h"
#include "backend/note/Note.h"
#include "backend/spreadsheet/Spreadsheet.h"
#ifdef HAVE_CANTOR_LIBS
#include "backend/cantorWorksheet/CantorWorksheet.h"
#endif
#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTSubscription.h"
#endif
#include "backend/worksheet/Worksheet.h"

#include <QIcon>
#include <KLocalizedString>

/**
 * \class Folder
 * \brief Folder in a project
 */

Folder::Folder(const QString &name) : AbstractAspect(name) {

	//when the child being removed is a LiveDataSource, stop reading from the source
	connect(this, &AbstractAspect::aspectAboutToBeRemoved, this, [=](const AbstractAspect* aspect) {
		const LiveDataSource* lds = dynamic_cast<const LiveDataSource*>(aspect);
		if (lds)
			const_cast<LiveDataSource*>(lds)->pauseReading();
	} );


}

QIcon Folder::icon() const {
	return QIcon::fromTheme("folder");
}

/**
 * \brief Return a new context menu.
 *
 * The caller takes ownership of the menu.
 */
QMenu* Folder::createContextMenu() {
	if (project()
#ifdef HAVE_MQTT
		&& !dynamic_cast<const MQTTSubscription*>(this)
#endif
	)
		return project()->createFolderContextMenu(this);
	return nullptr;
}

/**
 * \brief Save as XML
 */
void Folder::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QLatin1String("folder"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	for (auto* child : children<AbstractAspect>(IncludeHidden)) {
		writer->writeStartElement(QLatin1String("child_aspect"));
		child->save(writer);
		writer->writeEndElement(); // "child_aspect"
	}
	writer->writeEndElement(); // "folder"
}

/**
 * \brief Load from XML
 */
bool Folder::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement()) break;

		if (reader->isStartElement()) {
			if (reader->name() == QLatin1String("comment")) {
				if (!readCommentElement(reader))
					return false;
			} else if (reader->name() == QLatin1String("child_aspect")) {
				if (!readChildAspectElement(reader, preview))
					return false;
			} else {// unknown element
				reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
				if (!reader->skipToEndElement()) return false;
			}
		}
	}

	return !reader->hasError();
}

void Folder::setPathesToLoad(const QStringList& pathes) {
	m_pathesToLoad = pathes;
}

const QStringList& Folder::pathesToLoad() const {
	return m_pathesToLoad;
}

/**
 * \brief Read child aspect from XML
 */
bool Folder::readChildAspectElement(XmlStreamReader* reader, bool preview) {
	if (!reader->skipToNextTag())
		return false;

	if (reader->isEndElement() && reader->name() == QLatin1String("child_aspect"))
		return true; // empty element tag

	//check whether we need to skip the loading of the current child aspect
	if (!m_pathesToLoad.isEmpty()) {
		const QString& name = reader->attributes().value("name").toString(); //name of the current child aspect
		const QString childPath = path() + '/' + name; //child's path is not available yet (child not added yet) -> construct it manually

		//skip the current child aspect it is not in the list of aspects to be loaded
		if (m_pathesToLoad.indexOf(childPath) == -1) {
			 //skip to the end of the current element
			if (reader->skipToEndElement())
				return false;

			//skip to the end of the "child_asspect" element
			if (reader->skipToEndElement())
				return false;

			return true;
		}
	}

	QString element_name = reader->name().toString();
	if (element_name == QLatin1String("folder")) {
		Folder* folder = new Folder(QString());

		if (!m_pathesToLoad.isEmpty()) {
			//a child folder to be read -> provide the list of aspects to be loaded to the child folder, too.
			//since the child folder and all its children are not added yet (path() returns empty string),
			//we need to remove the path of the current child folder from the full pathes provided in m_pathesToLoad.
			//E.g. we want to import the path "Project/Folder/Spreadsheet" in the following project
			// Project
			//        \Spreadsheet
			//        \Folder
			//               \Spreadsheet
			//
			//Here, we remove the part "Project/Folder/" and proceed for this child folder with "Spreadsheet" only.
			//With this the logic above where it is determined whether to import the child aspect or not works out.

			//manually construct the path of the child folder to be read
			const QString& curFolderPath = path()  + '/' + reader->attributes().value("name").toString();

			//remove the path of the current child folder
			QStringList pathesToLoadNew;
			for (auto path : m_pathesToLoad) {
				if (path.startsWith(curFolderPath))
					pathesToLoadNew << path.right(path.length() - curFolderPath.length());
			}

			folder->setPathesToLoad(pathesToLoadNew);
		}

		if (!folder->load(reader, preview)) {
			delete folder;
			return false;
		}
		addChildFast(folder);
	} else if (element_name == QLatin1String("workbook")) {
		Workbook* workbook = new Workbook(QString());
		if (!workbook->load(reader, preview)) {
			delete workbook;
			return false;
		}
		addChildFast(workbook);
	} else if (element_name == QLatin1String("spreadsheet")) {
		Spreadsheet* spreadsheet = new Spreadsheet(QString(), true);
		if (!spreadsheet->load(reader, preview)) {
			delete spreadsheet;
			return false;
		}
		addChildFast(spreadsheet);
	} else if (element_name == QLatin1String("matrix")) {
		Matrix* matrix = new Matrix(QString(), true);
		if (!matrix->load(reader, preview)) {
			delete matrix;
			return false;
		}
		addChildFast(matrix);
	} else if (element_name == QLatin1String("worksheet")) {
		Worksheet* worksheet = new Worksheet(QString());
		worksheet->setIsLoading(true);
		if (!worksheet->load(reader, preview)) {
			delete worksheet;
			return false;
		}
		addChildFast(worksheet);
		worksheet->setIsLoading(false);
#ifdef HAVE_CANTOR_LIBS
	} else if (element_name == QLatin1String("cantorWorksheet")) {
		CantorWorksheet* cantorWorksheet = new CantorWorksheet(QLatin1String("null"), true);
		if (!cantorWorksheet->load(reader, preview)) {
			delete cantorWorksheet;
			return false;
		}
		addChildFast(cantorWorksheet);
#endif
#ifdef HAVE_MQTT
	} else if (element_name == QLatin1String("MQTTClient")) {
		qDebug()<<"Load MQTTClient";
		MQTTClient* client = new MQTTClient(QString());
		if (!client->load(reader, preview)) {
			delete client;
			return false;
		}
		addChildFast(client);
#endif
	} else if (element_name == QLatin1String("LiveDataSource")) {
		LiveDataSource* liveDataSource = new LiveDataSource(QString(), true);
		if (!liveDataSource->load(reader, preview)) {
			delete liveDataSource;
			return false;
		}
		addChildFast(liveDataSource);
	} else if (element_name == QLatin1String("datapicker")) {
		Datapicker* datapicker = new Datapicker(QString(), true);
		if (!datapicker->load(reader, preview)) {
			delete datapicker;
			return false;
		}
		addChildFast(datapicker);
	} else if (element_name == QLatin1String("note")) {
		Note* note = new Note(QString());
		if (!note->load(reader, preview)) {
			delete note;
			return false;
		}
		addChildFast(note);
	} else {
		reader->raiseWarning(i18n("unknown element '%1' found", element_name));
		if (!reader->skipToEndElement())
			return false;
	}

	if (!reader->skipToNextTag()) return false;
	return !reader->hasError();
}
