/***************************************************************************
File		: DatasetHandler.cpp
Project		: LabPlot
Description	: Processes a dataset's metadata file
--------------------------------------------------------------------
Copyright	: (C) 2019 Kovacs Ferencz (kferike98@gmail.com)

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

#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/DatasetHandler.h"

#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QStandardPaths>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

/*!
  \class DatasetHandler
  \brief Provides  functionality to process a metadata file of a dataset, configure a spreadsheet and filter based on it, download the dataset
  and load it into the spreadsheet.

  \ingroup datasources
*/
DatasetHandler::DatasetHandler(Spreadsheet* spreadsheet) : m_spreadsheet(spreadsheet),
	m_filter(new AsciiFilter),
	m_document(new QJsonDocument),
	m_downloadManager(new QNetworkAccessManager) {
	connect(m_downloadManager, &QNetworkAccessManager::finished, this, &DatasetHandler::downloadFinished);
	connect(this, &DatasetHandler::downloadCompleted, this, &DatasetHandler::processDataset);
}

DatasetHandler::~DatasetHandler() {
	delete m_document;
	delete m_downloadManager;
	delete m_filter;
}

/**
 * @brief Initiates processing the metadata file,, located at the given path, belonging to a dataset.
 * @param path the path to the metadata file
 */
void DatasetHandler::processMetadata(const QString& path) {
	qDebug("Start processing dataset...");
	loadJsonDocument(path);

	m_containingDir = path.left(path.lastIndexOf(QDir::separator()));
	qDebug() << m_containingDir;

	if(!m_document->isEmpty()) {
		configureFilter();
		configureSpreadsheet();
		prepareForDataset();
	}
}

/**
 * @brief Opens the json file at the given path and creates a QJsonDocument.
 * @param path the path to the file
 */
void DatasetHandler::loadJsonDocument(const QString& path) {
	qDebug("Load Json document for metadata");
	QFile file(path);
	if (file.open(QIODevice::ReadOnly)) {
		m_document = new QJsonDocument(QJsonDocument::fromJson(file.readAll()));
		file.close();
	} else {
		qDebug("Couldn't open dataset category file");
	}
}

/**
 * @brief Marks the metadata file being invalid by setting the value of a flag, also pops up a messagebox.
 */
void DatasetHandler::markMetadataAsInvalid() {
	m_invalidMetadataFile = true;
	QMessageBox::critical(0, "Invalid metadata file", "The metadata file for the choosen dataset is invalid!");
}

/**
 * @brief Configures the filter, that will be used later, based on the metadata file.
 */
void DatasetHandler::configureFilter() {
	qDebug("Configure filter");
	if(m_document->isObject()) {
		QJsonObject jsonObject = m_document->object();
		if(jsonObject.contains("separator"))
			m_filter->setSeparatingCharacter( jsonObject.value("separator").toString());
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("comment_character"))
			m_filter->setCommentCharacter(jsonObject.value("comment_character").toString());
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("create_index_column"))
			m_filter->setCreateIndexEnabled(jsonObject.value("create_index_column").toBool());
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("skip_empty_parts"))
			m_filter->setSkipEmptyParts(jsonObject.value("skip_empty_parts").toBool());
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("simplify_whitespaces"))
			m_filter->setSimplifyWhitespacesEnabled(jsonObject.value("simplify_whitespaces").toBool());
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("remove_quotes"))
			m_filter->setRemoveQuotesEnabled(jsonObject.value("remove_quotes").toBool());
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("use_first_row_for_vectorname"))
			m_filter->setHeaderEnabled(jsonObject.value("use_first_row_for_vectorname").toBool());
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("number_format"))
			m_filter->setNumberFormat(QLocale::Language(jsonObject.value("number_format").toInt()));
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("DateTime_format"))
			m_filter->setDateTimeFormat(jsonObject.value("DateTime_format").toString());
		else
			markMetadataAsInvalid();

	} else {
		markMetadataAsInvalid();
	}
}

/**
 * @brief Configures the spreadsheet based on the metadata file.
 */
void DatasetHandler::configureSpreadsheet() {
	qDebug("Conf spreadsheet");
	if(m_document->isObject()) {
		const QJsonObject& jsonObject = m_document->object();
		if(jsonObject.contains("name"))
			m_spreadsheet->setName( jsonObject.value("name").toString());
		else
			markMetadataAsInvalid();

		if(jsonObject.contains("description"))
			m_spreadsheet->setComment(jsonObject.value("description").toString());
	} else {
		markMetadataAsInvalid();
	}
}

/**
 * @brief Extracts the download URL of the dataset and initiates the process of download.
 */
void DatasetHandler::prepareForDataset() {
	qDebug("Start downloading dataset");
	if(m_document->isObject()) {
		const QJsonObject& jsonObject = m_document->object();

		if(jsonObject.contains("download")) {
			const QString& url =  jsonObject.value("download").toString();
			const QUrl downloadUrl = QUrl::fromEncoded(url.toLocal8Bit());
			doDownload(url);
		}
		else {
			QMessageBox::critical(0, i18n("Invalid metadata file"), i18n("There is no download URL present in the metadata file!"));
		}

	} else {
		markMetadataAsInvalid();
	}
}

/**
 * @brief Starts the download of the dataset.
 * @param url the download URL of the dataset
 */
void DatasetHandler::doDownload(const QUrl& url) {
	qDebug("Download request");
	QNetworkRequest request(url);
	m_currentDownload = m_downloadManager->get(request);
	connect(m_currentDownload, &QNetworkReply::downloadProgress, [this] (qint64 bytesReceived, qint64 bytesTotal) {
		double progress;
		if (bytesTotal == -1)
			progress = 0;
		else
			progress = 100 * (static_cast<double>(bytesReceived) / static_cast<double>(bytesTotal));
		qDebug() << "Progress: " << progress;
		emit downloadProgress(progress);
	});
}

/**
 * @brief Called when the download of the dataset is finished.
 */
void DatasetHandler::downloadFinished(QNetworkReply* reply) {
	qDebug("Download finished");
	const QUrl& url = reply->url();
	if (reply->error()) {
		qDebug("Download of %s failed: %s\n",
			   url.toEncoded().constData(),
			   qPrintable(reply->errorString()));
	} else {
		if (isHttpRedirect(reply)) {
			qDebug("Request was redirected.\n");
		} else {
			QString filename = saveFileName(url);
			if (saveToDisk(filename, reply)) {
				qDebug("Download of %s succeeded (saved to %s)\n",
					   url.toEncoded().constData(), qPrintable(filename));
				m_fileName = filename;
				emit downloadCompleted();
			}
		}
	}

	m_currentDownload = nullptr;
	reply->deleteLater();
}

/**
 * @brief Checks whether the GET request was redirected or not.
 */
bool DatasetHandler::isHttpRedirect(QNetworkReply* reply) {
	const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	// TODO enum/defines for status codes ?
	return statusCode == 301 || statusCode == 302 || statusCode == 303
			|| statusCode == 305 || statusCode == 307 || statusCode == 308;
}

/**
 * @brief Returns the name and path of the file that will contain the content of the reply (based on the URL).
 * @param url
 */
QString DatasetHandler::saveFileName(const QUrl& url) {
	const QString path = url.path();
	QString basename = QFileInfo(path).fileName();

	if (basename.isEmpty())
		basename = "download";

	QString fileName = m_containingDir + QDir::separator() + basename;

	if (QFile::exists(fileName)) {
		// already exists, don't overwrite
		int i = 0;
		fileName += '.';
		while (QFile::exists(fileName + QString::number(i)))
			++i;

		fileName += QString::number(i);
	}
	return fileName;
}

/**
 * @brief Saves the content of the network reply to the given path under the given name.
 */
bool DatasetHandler::saveToDisk(const QString& filename, QIODevice* data) {
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		qDebug("Could not open %s for writing: %s\n",
			   qPrintable(filename),
			   qPrintable(file.errorString()));
		return false;
	}

	file.write(data->readAll());
	file.close();

	return true;
}

/**
 * @brief Processes the downloaded dataset with the help of the already configured filter.
 */
void DatasetHandler::processDataset() {
	m_filter->readDataFromFile(m_fileName, m_spreadsheet);
	configureColumns();
}

/**
 * @brief Configures the columns of the spreadsheet, based on the metadata file.
 */
void DatasetHandler::configureColumns() {
	if(m_document->isObject()) {
		const QJsonObject jsonObject = m_document->object();

		int index = 0;
		const int columnsCount = m_spreadsheet->columnCount();
		while(jsonObject.contains(i18n("column_description_%1", index)) && (index < columnsCount)) {
			m_spreadsheet->column(index)->setComment(jsonObject.value(i18n("column_description_%1", index)).toString());
			++index;
		}
	} else {
		qDebug("Invalid Json document");
	}
}
