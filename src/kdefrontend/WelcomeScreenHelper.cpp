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


/*!
\class WelcomeScreenHelper
\brief Helper class for the welcome screen

\ingroup kdefrontend
*/
WelcomeScreenHelper::WelcomeScreenHelper() {
	m_datasetWidget = new ImportDatasetWidget(0);
	m_datasetWidget->hide();

	m_datasetModel = new DatasetModel(m_datasetWidget->getDatasetsMap());

}

WelcomeScreenHelper::~WelcomeScreenHelper() {

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

/**
 * @brief Returns a pointer to the datasetModel of the class.
 */
DatasetModel* WelcomeScreenHelper::getDatasetModel() {
	return m_datasetModel;
}
