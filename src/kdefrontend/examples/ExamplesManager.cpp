/*
    File                 : ExamplesManager.h
    Project              : LabPlot
    Description          : widget showing the available color maps
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
						QStringList() << "*.lml",
						QDir::Files,
						QDirIterator::Subdirectories);
		while (it.hasNext()) {
			const auto& fileName = it.next();
			const auto& name = QFileInfo(fileName).baseName();
			names << name;
			m_paths[name] = fileName;

			// check compression, s.a. Project::load()
			QIODevice* file = new QFile(fileName);
			if (!file->open(QIODevice::ReadOnly)) {
				delete file;
				continue;
			}

			QDataStream in(file);
			quint16 magic;
			in >> magic;
			file->close();
			delete file;

			if (!magic) //empty file
				continue;

			if (magic == 0xfd37)	// XZ compressed data
				file = new KCompressionDevice(fileName, KCompressionDevice::Xz);
			else	// gzip or not compressed data
				file = new KCompressionDevice(fileName, KCompressionDevice::GZip);

			if (!file->open(QIODevice::ReadOnly)) {
				file->close();
				delete file;
				continue;
			}

			//parse the XML and read the description and the preview pixmap of the project file
			QXmlStreamReader reader(file);
			reader.readNext();
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
