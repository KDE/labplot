/*
	File                 : Folder.cpp
	Project              : LabPlot
	Description          : Folder in a project
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke@gmx.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/core/Folder.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#ifndef SDK
#include "backend/core/Workbook.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/matrix/Matrix.h"
#include "backend/note/Note.h"
#ifdef HAVE_CANTOR_LIBS
#include "backend/cantorWorksheet/CantorWorksheet.h"
#endif
#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif
#endif

#include "backend/lib/XmlStreamReader.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"

#include <KLocalizedString>
#include <QDropEvent>
#include <QIcon>
#include <QMimeData>

/**
 * \class Folder
 * \brief Folder in a project
 */

Folder::Folder(const QString& name, AspectType type)
	: AbstractAspect(name, type) {
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
		&& type() != AspectType::MQTTSubscription
#endif
	)
		return project()->createFolderContextMenu(this);
	return nullptr;
}

bool Folder::isDraggable() const {
	if (dynamic_cast<const Project*>(this))
		return false;
	else
		return true;
}

QVector<AspectType> Folder::pasteTypes() const {
	return QVector<AspectType>{AspectType::Folder,
							   AspectType::Worksheet,
							   AspectType::Workbook,
							   AspectType::Spreadsheet,
							   AspectType::Matrix,
							   AspectType::Datapicker,
							   AspectType::LiveDataSource,
							   AspectType::Note,
							   AspectType::CantorWorksheet};
}

QVector<AspectType> Folder::dropableOn() const {
	return QVector<AspectType>{AspectType::Folder, AspectType::Project};
}

void Folder::processDropEvent(const QVector<quintptr>& vec) {
	// reparent AbstractPart and Folder objects only
	AbstractAspect* lastMovedAspect{nullptr};
	for (auto a : vec) {
		auto* aspect = reinterpret_cast<AbstractAspect*>(a);
		auto* part = dynamic_cast<AbstractPart*>(aspect);
		if (part) {
			part->reparent(this);
			lastMovedAspect = part;
		} else {
			auto* folder = dynamic_cast<Folder*>(aspect);
			if (folder && folder != this) {
				folder->reparent(this);
				lastMovedAspect = folder;
			}
		}
	}

	// select the last moved aspect in the project explorer
	if (lastMovedAspect)
		lastMovedAspect->setSelected(true);
}

/**
 * \brief Save as XML
 */
void Folder::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QLatin1String("folder"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	const auto& children = this->children<AbstractAspect>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children) {
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

		if (reader->isEndElement())
			break;

		if (reader->isStartElement()) {
			if (reader->name() == QLatin1String("comment")) {
				if (!readCommentElement(reader))
					return false;
			} else if (reader->name() == QLatin1String("child_aspect")) {
				if (!readChildAspectElement(reader, preview))
					return false;
			} else { // unknown element
				reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
				if (!reader->skipToEndElement())
					return false;
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

	// check whether we need to skip the loading of the current child aspect
	if (!m_pathesToLoad.isEmpty()) {
		const QString& name = reader->attributes().value("name").toString(); // name of the current child aspect
		const QString childPath = path() + '/' + name; // child's path is not available yet (child not added yet) -> construct it manually

		// skip the current child aspect it is not in the list of aspects to be loaded
		if (m_pathesToLoad.indexOf(childPath) == -1) {
			// skip to the end of the current element
			if (!reader->skipToEndElement())
				return false;

			// skip to the end of the "child_asspect" element
			if (!reader->skipToEndElement())
				return false;
			return true;
		}
	}

	QString element_name = reader->name().toString();
	if (element_name == QLatin1String("folder")) {
		auto* folder = new Folder(QString());

		if (!m_pathesToLoad.isEmpty()) {
			// a child folder to be read -> provide the list of aspects to be loaded to the child folder, too.
			// since the child folder and all its children are not added yet (path() returns empty string),
			// we need to remove the path of the current child folder from the full pathes provided in m_pathesToLoad.
			// E.g. we want to import the path "Project/Folder/Spreadsheet" in the following project
			//  Project
			//         \Spreadsheet
			//         \Folder
			//                \Spreadsheet
			//
			// Here, we remove the part "Project/Folder/" and proceed for this child folder with "Spreadsheet" only.
			// With this the logic above where it is determined whether to import the child aspect or not works out.

			// manually construct the path of the child folder to be read
			const QString& curFolderPath = path() + '/' + reader->attributes().value("name").toString();

			// remove the path of the current child folder
			QStringList pathesToLoadNew;
			for (const auto& path : qAsConst(m_pathesToLoad)) {
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
#ifndef SDK
		auto* workbook = new Workbook(QString());
		if (!workbook->load(reader, preview)) {
			delete workbook;
			return false;
		}
		addChildFast(workbook);
#endif
	} else if (element_name == QLatin1String("spreadsheet")) {
#ifndef SDK
		auto* spreadsheet = new Spreadsheet(QString(), true);
		if (!spreadsheet->load(reader, preview)) {
			delete spreadsheet;
			return false;
		}
		addChildFast(spreadsheet);
#endif
	} else if (element_name == QLatin1String("matrix")) {
#ifndef SDK
		auto* matrix = new Matrix(QString(), true);
		if (!matrix->load(reader, preview)) {
			delete matrix;
			return false;
		}
		addChildFast(matrix);
#endif
	} else if (element_name == QLatin1String("worksheet")) {
		auto* worksheet = new Worksheet(QString(), true);
		worksheet->setIsLoading(true);
		if (!worksheet->load(reader, preview)) {
			delete worksheet;
			return false;
		}
		addChildFast(worksheet);
		worksheet->setIsLoading(false);
	} else if (element_name == QLatin1String("cantorWorksheet")) {
#ifdef HAVE_CANTOR_LIBS
#ifndef SDK
		auto* cantorWorksheet = new CantorWorksheet(QLatin1String("null"), true);
		if (!cantorWorksheet->load(reader, preview)) {
			delete cantorWorksheet;

			// if we only failed to load because of the missing CAS, don't return with false here.
			// in this case we continue loading the project and show a warning about missing CAS at the end.
			if (!reader->failedCASMissing())
				return false;
			else {
				// failed because of the missing CAS. Read until the end of the current
				// element in XML and continue loading the project.
				while (!reader->atEnd()) {
					reader->readNext();
					if (reader->isEndElement() && reader->name() == QLatin1String("cantorWorksheet"))
						break;
				}
			}
		} else
			addChildFast(cantorWorksheet);
#endif
#else
		if (!preview) {
			while (!reader->atEnd()) {
				reader->readNext();
				if (reader->isEndElement() && reader->name() == QLatin1String("cantorWorksheet"))
					break;

				if (!reader->isStartElement())
					continue;

				if (reader->name() == QLatin1String("general")) {
					const QString& backendName = reader->attributes().value("backend_name").toString().trimmed();
					if (!backendName.isEmpty())
						reader->raiseMissingCASWarning(backendName);
				} else
					reader->skipToEndElement();
			}
		}
#endif
#ifdef HAVE_MQTT
#ifndef SDK
	} else if (element_name == QLatin1String("MQTTClient")) {
		auto* client = new MQTTClient(QString());
		if (!client->load(reader, preview)) {
			delete client;
			return false;
		}
		addChildFast(client);
#endif
#endif
	} else if (element_name == QLatin1String("liveDataSource")
			   || element_name == QLatin1String("LiveDataSource")) { // TODO: remove "LiveDataSources" in couple of releases
#ifndef SDK
		auto* liveDataSource = new LiveDataSource(QString(), true);
		if (!liveDataSource->load(reader, preview)) {
			delete liveDataSource;
			return false;
		}
		addChildFast(liveDataSource);
#endif
	} else if (element_name == QLatin1String("datapicker")) {
#ifndef SDK
		auto* datapicker = new Datapicker(QString(), true);
		if (!datapicker->load(reader, preview)) {
			delete datapicker;
			return false;
		}
		addChildFast(datapicker);
#endif
	} else if (element_name == QLatin1String("note")) {
#ifndef SDK
		Note* note = new Note(QString());
		if (!note->load(reader, preview)) {
			delete note;
			return false;
		}
		addChildFast(note);
#endif
	} else {
		reader->raiseWarning(i18n("unknown element '%1' found", element_name));
		if (!reader->skipToEndElement())
			return false;
	}

	if (!reader->skipToNextTag())
		return false;
	return !reader->hasError();
}
