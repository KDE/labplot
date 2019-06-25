/***************************************************************************
	File                 : DatasetModel.h
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

DatasetModel::DatasetModel() {
	m_datasetWidget = new ImportDatasetWidget(0);
	m_datasetWidget->hide();
	const QMap<QString, QMap<QString, QVector<QString>>> datasetsMap = m_datasetWidget->getDatasetsMap();
	initCategories(datasetsMap);
	initSubcategories(datasetsMap);
	initDatasets(datasetsMap);
}

DatasetModel::~DatasetModel() {

}

void DatasetModel::initCategories(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	m_categoryLists = datasetMap.keys();
}

void DatasetModel::initSubcategories(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	for(auto i = datasetMap.begin(); i != datasetMap.end(); ++i) {
		m_subcategories[i.key()] = i.value().keys();
	}
}

void DatasetModel::initDatasets(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	for(auto category = datasetMap.begin(); category != datasetMap.end(); ++category) {
		const QMap<QString, QVector<QString>>& category_ = category.value();

		for(auto subcategory = category_.begin(); subcategory != category_.end(); ++subcategory) {
			m_datasets[category.key()][subcategory.key()] = subcategory.value().toList();
		}
	}
}

QVariant DatasetModel::categories() {
	return QVariant(m_categoryLists);
}

QVariant DatasetModel::subcategories(const QString& category) {
	return QVariant(m_subcategories[category]);
}

QVariant DatasetModel::datasets(const QString& category, const QString& subcategory) {
	return QVariant(m_datasets[category][subcategory]);
}

void DatasetModel::datasetClicked(QString category, QString subcategory, QString datasetName) {

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

QVariant DatasetModel::datasetName() {
	return QVariant(m_spreadsheet->name());
}

QVariant DatasetModel::datasetDescription() {
	return QVariant(m_spreadsheet->comment());
}

QVariant DatasetModel::datasetColumns() {
	return QVariant(m_spreadsheet->columnCount());
}
QVariant DatasetModel::datasetRows() {
	return QVariant(m_spreadsheet->rowCount());
}

Spreadsheet* DatasetModel::releaseConfiguredSpreadsheet() {
	return m_spreadsheet.release();
}


QVariant DatasetModel::getProjectThumbnail(const QUrl& url) {
	QString filename;
	if (url.isLocalFile())	// fix for Windows
		filename = url.toLocalFile();
	else
		filename = url.path();

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
			qDebug() << "Return thumbnail " <<thumbnail;
			return QVariant(thumbnail);
		}
	}

	return QVariant();
}
