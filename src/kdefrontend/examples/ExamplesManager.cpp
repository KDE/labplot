/***************************************************************************
	File                 : ExamplesManager.h
	Project              : LabPlot
	Description          : widget showing the available color maps
	--------------------------------------------------------------------
	Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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

#include "kdefrontend/examples/ExamplesManager.h"
#include "backend/lib/macros.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include <KFilterDev>
#include <KLocalizedString>

ExamplesManager* ExamplesManager::m_instance{nullptr};

/*!
	\class ExamplesManager
	\brief Widget for importing data from a dataset.

	\ingroup kdefrontend
 */
ExamplesManager::ExamplesManager() {
	m_jsonDir = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("examples"), QStandardPaths::LocateDirectory);
	loadCollections();
}

ExamplesManager::~ExamplesManager() {

}

ExamplesManager* ExamplesManager::instance() {
	if (!m_instance)
		m_instance = new ExamplesManager();

	return m_instance;
}

QStringList ExamplesManager::collectionNames() const {
	return m_collections.keys();
}

QString ExamplesManager::collectionInfo(const QString& name) const {
	return m_collections[name];
}

/*!
 * \brief returns the list of names of the example projects for the collection \c collecitonName
 */
QStringList ExamplesManager::exampleNames(const QString& collectionName) {
	//load the collection if not done yet
	if (!m_examples.contains(collectionName)) {
		//example projects of the currently selected collection not loaded yet -> load them
		QStringList names;
		QDirIterator it(m_jsonDir + QLatin1Char('/') + collectionName,
						QStringList() << "*.lml" << "*.lml.xz",
						QDir::Files,
						QDirIterator::Subdirectories);
		while (it.hasNext()) {
			const auto& fileName = it.next();
			const auto& name = QFileInfo(fileName).baseName();
			names << name;
			m_paths[name] = fileName;

			//parse the XML and read the description and the preview pixmap of the project file
			QIODevice* file;
			// first try gzip compression, because projects can be gzipped and end with .lml
			if (fileName.endsWith(QLatin1String(".lml"), Qt::CaseInsensitive))
				file = new KCompressionDevice(fileName, KFilterDev::compressionTypeForMimeType("application/x-gzip"));
			else	// opens filename using file ending
				file = new KFilterDev(fileName);

			if (!file->open(QIODevice::ReadOnly))
				continue;

			//parse XML
			QXmlStreamReader reader(file);
			while (!reader.atEnd()) {
				reader.readNext();

				if (reader.isEndElement())
					break;

				if (reader.isStartElement()) {
					if (reader.name() == "project") {
						QString content = reader.attributes().value("thumbnail").toString();
						QByteArray ba = QByteArray::fromBase64(content.toLatin1());
						QPixmap pixmap;
						pixmap.loadFromData(ba);
						m_pixmaps[name] = pixmap;
					} else if (reader.name() == "comment"){
						m_descriptions[name] = reader.readElementText();
						break;
					}
				}
			}

			file->close();
			delete file;
		}

		m_examples[collectionName] = names;
	}

	return m_examples[collectionName];
}

/*!
 * returns the preview pixmap for the example project \c name.
 */
QPixmap ExamplesManager::pixmap(const QString& name) const {
	if (name.isEmpty())
		return QPixmap();

	return m_pixmaps[name];
}

QString ExamplesManager::description(const QString& name) const {
	return m_descriptions[name];
}

QString ExamplesManager::path(const QString& name) const {
	return m_paths[name];
}

/**
 * @brief Processes the json metadata file that contains the list of colormap collections.
 */
void ExamplesManager::loadCollections() {
	const QString& fileName = m_jsonDir + QLatin1String("/ExampleCollections.json");
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly)) {
		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		file.close();
		if (!document.isArray()) {
			QDEBUG("Invalid definition of " + fileName)
			return;
		}

		for (const QJsonValueRef col : document.array()) {
			const QJsonObject& collection = col.toObject();
			const QString& name = collection[QLatin1String("name")].toString();
			const QString& desc = collection[QLatin1String("description")].toString();
			m_collections[name] = desc;
		}
	} else
		QMessageBox::critical(nullptr, i18n("File not found"),
							i18n("Couldn't open the examples collections file %1. Please check your installation.", fileName));
}
