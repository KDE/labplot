/***************************************************************************
	File                 : WelcomeScreenHelper.cpp
	Project              : LabPlot
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Ferencz Kovacs (kferike98@gmail.com)
	Description          : Helper class for the welcome screen
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
#include "WelcomeScreenHelper.h"
#include "DatasetModel.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "backend/datasources/DatasetHandler.h"

#include <QVector>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QUrl>

#include <KFilterDev>
#include <KCompressionDevice>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <KSharedConfig>
#include <KConfigGroup>


/*!
\class WelcomeScreenHelper
\brief Helper class for the welcome screen

\ingroup kdefrontend
*/
WelcomeScreenHelper::WelcomeScreenHelper() {
	m_datasetWidget = new ImportDatasetWidget(0);
	m_datasetWidget->hide();

	m_datasetModel = new DatasetModel(m_datasetWidget->getDatasetsMap());

	loadConfig();

	processExampleProjects();
}

WelcomeScreenHelper::~WelcomeScreenHelper() {
	//save width&height ratio values
	KConfigGroup conf(KSharedConfig::openConfig(), "WelcomeScreenHelper");

	int widthCount = m_widthScale.size();
	conf.writeEntry("width_count", widthCount);
	int currentWidthIndex = 0;

	for(auto item = m_widthScale.begin(); item != m_widthScale.end() && currentWidthIndex < widthCount; item++) {
		conf.writeEntry("widthName_" + QString::number(currentWidthIndex), item.key());
		conf.writeEntry("widthValue_" + QString::number(currentWidthIndex), item.value());
		currentWidthIndex++;
	}

	int heightCount = m_heightScale.size();
	conf.writeEntry("height_count", widthCount);
	int currentHeightIndex = 0;

	for(auto item = m_heightScale.begin(); item != m_heightScale.end() && currentHeightIndex < heightCount; item++) {
		conf.writeEntry("heightName_" + QString::number(currentHeightIndex), item.key());
		conf.writeEntry("heightValue_" + QString::number(currentHeightIndex), item.value());
		currentHeightIndex++;
	}
}

void WelcomeScreenHelper::loadConfig() {
	KConfigGroup conf(KSharedConfig::openConfig(), "WelcomeScreenHelper");

	int widthCount = conf.readEntry("width_count", -1);
	for(int i = 0; i < widthCount; ++i) {
		QString id = conf.readEntry("widthName_" + QString::number(i), "");
		double value = conf.readEntry("widthValue_" + QString::number(i), -1);

		if(!id.isEmpty() && value != -1)
			m_widthScale[id] = value;
	}

	int heightCount = conf.readEntry("height_count", -1);
	for(int i = 0; i < heightCount; ++i) {
		QString id = conf.readEntry("heightName_" + QString::number(i), "");
		double value = conf.readEntry("heightValue_" + QString::number(i), -1);

		if(!id.isEmpty() && value != -1)
			m_heightScale[id] = value;
	}
}

/**
 * @brief Handles a dataset being clicked in the dataset section of the WelcomeScreen.
 * Initiates listing information about the dataset in the previously mentioned section.
 *
 * @param category the category the dataset belongs to
 * @param subcategory the subcategory the dataset belongs to
 * @param datasetName the name of the dataset
 */
void WelcomeScreenHelper::datasetClicked(const QString& category, const QString& subcategory, const QString& datasetName) {

	m_datasetWidget->setCategory(category);
	m_datasetWidget->setSubcategory(subcategory);
	m_datasetWidget->setDataset(datasetName);

	//m_spreadsheet->clear()

	//if(m_spreadsheet.get() != nullptr)
	//delete m_spreadsheet.get();
	m_spreadsheet.reset(new Spreadsheet(i18n("Dataset%1", 1)));

	if(m_datasetHandler != nullptr)
		delete m_datasetHandler;
	m_datasetHandler = new DatasetHandler(m_spreadsheet.get());

	m_datasetWidget->loadDatasetToProcess(m_datasetHandler);

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;
	connect(m_datasetHandler,  &DatasetHandler::downloadCompleted, &loop, &QEventLoop::quit);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	timer.start(1500);
	loop.exec();

	if(timer.isActive()){
		timer.stop();
		emit datasetFound();
	}
	else
		emit datasetNotFound();
}

/**
 * @brief Returns the dataset's full name.
 */
QVariant WelcomeScreenHelper::datasetName() {
	return QVariant(m_spreadsheet->name());
}

/**
 * @brief Returns the dataset's descripton.
 */
QVariant WelcomeScreenHelper::datasetDescription() {
	return QVariant(m_spreadsheet->comment());
}

/**
 * @brief Returns the number of the dataset's columns.
 */
QVariant WelcomeScreenHelper::datasetColumns() {
	return QVariant(m_spreadsheet->columnCount());
}

/**
 * @brief Returns the number of the dataset's rows.
 */
QVariant WelcomeScreenHelper::datasetRows() {
	return QVariant(m_spreadsheet->rowCount());
}

/**
 * @brief Returns a pointer to the spreadsheet in which the data of the dataset was loaded.
 */
Spreadsheet* WelcomeScreenHelper::releaseConfiguredSpreadsheet() {
	return m_spreadsheet.release();
}

/**
 * @brief Returns the thumbnail image saved with the project.
 * @param url the path to the saved project file.
 */
QVariant WelcomeScreenHelper::getProjectThumbnail(const QUrl& url) {
	QString filename;
	if (url.isLocalFile())	// fix for Windows
		filename = url.toLocalFile();
	else
		filename = url.path();

	qDebug() << "Get thumbnail for: " << filename;
	QIODevice* file;
	// first try gzip compression, because projects can be gzipped and end with .lml
	if (filename.endsWith(QLatin1String(".lml"), Qt::CaseInsensitive))
		file = new KCompressionDevice(filename,KFilterDev::compressionTypeForMimeType("application/x-gzip"));
	else	// opens filename using file ending
		file = new KFilterDev(filename);

	if (!file)
		file = new QFile(filename);

	if (!file->open(QIODevice::ReadOnly)) {
		qDebug() << "Could not open file for reading.";
		return QVariant();
	}

	char c;
	bool rc = file->getChar(&c);
	if (!rc) {
		qDebug() << "The project file is empty.";
		file->close();
		delete file;
		return false;
	}
	file->seek(0);

	//parse XML
	XmlStreamReader reader(file);

	while (!(reader.isStartDocument() || reader.atEnd()))
		reader.readNext();

	if (!(reader.atEnd())) {
		if (!reader.skipToNextTag())
			return false;

		if (reader.name() == "project") {
			QString thumbnail = reader.attributes().value("thumbnail").toString();

			thumbnail.prepend("data:image/jpg;base64,");
			//qDebug() << "Return thumbnail " <<thumbnail;
			return QVariant(thumbnail);
		}
	}

	return QVariant();
}

/**
 * @brief Returns a pointer to the datasetModel of the class.
 */
DatasetModel* WelcomeScreenHelper::getDatasetModel() {
	return m_datasetModel;
}

void WelcomeScreenHelper::processExampleProjects() {

	const QString filePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, "example_projects/example_projects.json");

	qDebug() << "Locating example metadata file" << filePath;

	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly)) {
		qDebug()<< "Process examples";

		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		qDebug() <<"Is array: " << document.isArray() << "  is object " << document.isObject();
		QJsonArray exampleArray = document.array();

		//processing examples
		for(int i = 0 ; i < exampleArray.size(); ++i) {
			const QJsonObject currentExample = exampleArray[i].toObject();

			const QString exampleName = currentExample.value("name").toString();
			qDebug()<< exampleName;
			if(m_projectNameList.contains(exampleName)) {
				qDebug() << "There is already an example file with this name";
			} else {
				qDebug()<< exampleName + "is a new name";
				m_projectNameList.append(exampleName);
				const QString exampleFile = currentExample.value("fileName").toString();
				m_pathMap[exampleName] = exampleFile;

				const QJsonArray tags = currentExample.value("tags").toArray();
				//processing tags
				qDebug()<< "Process tags";
				for(int j = 0; j < tags.size(); ++j) {
					QString tagName = tags[j].toString();
					qDebug()<< "Process tag: " << tagName;
					m_tagMap[tagName].append(exampleName);
					m_datasetTag[exampleName].append(tagName);
				}
			}
		}

		file.close();
	} else {
		qDebug("Couldn't open dataset category file");
	}
}

QVariant WelcomeScreenHelper::getExampleProjectThumbnail(const QString& exampleName) {
	const QString filePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, "example_projects/" + m_pathMap[exampleName]);
	qDebug() << "ExampleProjectThumbnail: " << filePath;
	return getProjectThumbnail(filePath);
}

QVariant WelcomeScreenHelper::getExampleProjects() {
	qDebug() << "ExampleProjects: " << m_projectNameList;
	return QVariant(m_projectNameList);
}

QVariant WelcomeScreenHelper::getExampleProjectTags(const QString& exampleName) {
	QString tags;
	const QStringList& tagList = m_datasetTag[exampleName];

	for(int i = 0; i < tagList.size(); i++) {
		tags.append(tagList[i]);
		if(i < tagList.size() - 1)
			tags.append(", ");
	}

	return QVariant(tags);
}

void WelcomeScreenHelper::exampleProjectClicked(const QString& exampleName) {
	QString path = QStandardPaths::locate(QStandardPaths::AppDataLocation, "example_projects/" + m_pathMap[exampleName]);
	emit openExampleProject(path);
}

QVariant WelcomeScreenHelper::searchExampleProjects(const QString& searchtext) {
	QStringList results;
	for(auto tag = m_tagMap.begin(); tag != m_tagMap.end(); tag++) {
		if (tag.key().contains(searchtext)) {
			for(QString example : tag.value()) {
				if(!results.contains(example))
					results.append(example);
			}
		}
	}

	for(QString example : m_projectNameList) {
		if(example.contains(searchtext) && !results.contains(example))
			results.append(example);
	}

	return QVariant(results);
}

void WelcomeScreenHelper::setWidthScale(QString sectionID, double scale) {
	m_widthScale[sectionID] = scale;
}

void WelcomeScreenHelper::setHeightScale(QString sectionID, double scale) {
	m_heightScale[sectionID] = scale;
}

QVariant WelcomeScreenHelper::getWidthScale(QString sectionID) {
	if(m_widthScale.keys().contains(sectionID))
		return QVariant(m_widthScale[sectionID]);

	return QVariant(-1);
}

QVariant WelcomeScreenHelper::getHeightScale(QString sectionID) {
	if(m_heightScale.keys().contains(sectionID))
		return QVariant(m_heightScale[sectionID]);

	return QVariant(-1);;
}
