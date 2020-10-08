/***************************************************************************
    File                 : DatasetHandler.cpp
    Project              : LabPlot
    Description          : Processes a dataset's metadata file
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Kovacs Ferencz (kferike98@gmail.com)
    Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)

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

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardPaths>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <KLocalizedString>

/*!
  \class DatasetHandler
  \brief Provides  functionality to process a metadata file of a dataset, configure a spreadsheet and filter based on it, download the dataset
  and load it into the spreadsheet.

  \ingroup datasources
*/
DatasetHandler::DatasetHandler(Spreadsheet* spreadsheet) : m_spreadsheet(spreadsheet),
	m_filter(new AsciiFilter),
	m_downloadManager(new QNetworkAccessManager) {
	connect(m_downloadManager, &QNetworkAccessManager::finished, this, &DatasetHandler::downloadFinished);
	connect(this, &DatasetHandler::downloadCompleted, this, &DatasetHandler::processDataset);
}

DatasetHandler::~DatasetHandler() {
	delete m_downloadManager;
	delete m_filter;
}

/**
 * @brief Initiates processing the metadata file,, located at the given path, belonging to a dataset.
 * @param path the path to the metadata file
 */
void DatasetHandler::processMetadata(const QJsonObject& object) {
	m_object = new QJsonObject(object);
	DEBUG("Start processing dataset...");

	if (!m_object->isEmpty()) {
		configureFilter();
		configureSpreadsheet();
		prepareForDataset();
	}
}

/**
 * @brief Marks the metadata file being invalid by setting the value of a flag, also pops up a messagebox.
 */
void DatasetHandler::markMetadataAsInvalid() {
	m_invalidMetadataFile = true;
	QMessageBox::critical(nullptr, i18n("Invalid metadata file"), i18n("The metadata file for the selected dataset is invalid."));
}

/**
 * @brief Configures the filter, that will be used later, based on the metadata file.
 */
void DatasetHandler::configureFilter() {
	//set some default values common to many datasets
	m_filter->setNumberFormat(QLocale::C);
	m_filter->setSkipEmptyParts(true);
	m_filter->setHeaderEnabled(false);

	//read properties specified in the dataset description
	if (!m_object->isEmpty()) {
		if (m_object->contains("separator"))
			m_filter->setSeparatingCharacter(m_object->value("separator").toString());

		if (m_object->contains("comment_character"))
			m_filter->setCommentCharacter(m_object->value("comment_character").toString());

		if (m_object->contains("create_index_column"))
			m_filter->setCreateIndexEnabled(m_object->value("create_index_column").toBool());

		if (m_object->contains("skip_empty_parts"))
			m_filter->setSkipEmptyParts(m_object->value("skip_empty_parts").toBool());

		if (m_object->contains("simplify_whitespaces"))
			m_filter->setSimplifyWhitespacesEnabled(m_object->value("simplify_whitespaces").toBool());

		if (m_object->contains("remove_quotes"))
			m_filter->setRemoveQuotesEnabled(m_object->value("remove_quotes").toBool());

		if (m_object->contains("use_first_row_for_vectorname"))
			m_filter->setHeaderEnabled(m_object->value("use_first_row_for_vectorname").toBool());

		if (m_object->contains("number_format"))
			m_filter->setNumberFormat(QLocale::Language(m_object->value("number_format").toInt()));

		if (m_object->contains("DateTime_format"))
			m_filter->setDateTimeFormat(m_object->value("DateTime_format").toString());

		if (m_object->contains("columns")) {
			const QJsonArray& columnsArray = m_object->value("columns").toArray();
			QStringList columnNames;
			for (const auto& col : columnsArray)
				columnNames << col.toString();

			m_filter->setVectorNames(columnNames);
		}
	} else {
		DEBUG("Empty object");
		markMetadataAsInvalid();
	}
}

/**
 * @brief Configures the spreadsheet based on the metadata file.
 */
void DatasetHandler::configureSpreadsheet() {
	DEBUG("Start preparing spreadsheet");
	if (!m_object->isEmpty()) {
		if (m_object->contains("name"))
			m_spreadsheet->setName( m_object->value("name").toString());
		else
			markMetadataAsInvalid();

		if (m_object->contains("description_url")) {
			auto* manager = new QNetworkAccessManager(this);
			connect(manager, &QNetworkAccessManager::finished, [this] (QNetworkReply* reply) {
				if (reply->error() == QNetworkReply::NoError) {
					QByteArray ba = reply->readAll();
					QString info(ba);
					m_spreadsheet->setComment(info);
				} else {
					DEBUG("Failed to fetch the description.");
					if (m_object->contains("description"))
						m_spreadsheet->setComment(m_object->value("description").toString());
				}
				reply->deleteLater();
			}
			);
			manager->get(QNetworkRequest(QUrl(m_object->value("description_url").toString())));
		} else if (m_object->contains("description"))
			m_spreadsheet->setComment(m_object->value("description").toString());
	} else {
		markMetadataAsInvalid();
	}
}

/**
 * @brief Extracts the download URL of the dataset and initiates the process of download.
 */
void DatasetHandler::prepareForDataset() {
	DEBUG("Start downloading dataset");
	if (!m_object->isEmpty()) {
		if (m_object->contains("url")) {
			const QString& url =  m_object->value("url").toString();
			doDownload(url);
		}
		else {
			QMessageBox::critical(nullptr, i18n("Invalid metadata file"), i18n("There is no download URL present in the metadata file!"));
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
	DEBUG("Download request");
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
	DEBUG("Download finished");
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

	//get the extension of the downloaded file
	const QString downloadFileName = QFileInfo(path).fileName();
	int lastIndex = downloadFileName.lastIndexOf(".");
	const QString fileExtension = lastIndex >= 0 ?  downloadFileName.right(downloadFileName.length() - lastIndex) : "";

	QString basename = m_object->value("filename").toString() + fileExtension;

	if (basename.isEmpty())
		basename = "url";

	QDir downloadDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/datasets_local/"));
	if (!downloadDir.exists()) {
		if (!downloadDir.mkpath(downloadDir.path())) {
			qDebug()<<"Failed to create the directory " << downloadDir.path();
			return QString();
		}
	}

	QString fileName = downloadDir.path() + QLatin1Char('/') + basename;
	QFileInfo fileInfo (fileName);
	if (QFile::exists(fileName)) {
		if (fileInfo.lastModified().addDays(1) < QDateTime::currentDateTime()){
			QFile removeFile (fileName);
			removeFile.remove();
		} else {
			qDebug() << "Dataset file already exists, no need to download it again";
		}
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

	//set column comments/descriptions, if available
	//TODO:
// 	if (!m_object->isEmpty()) {
// 		int index = 0;
// 		const int columnsCount = m_spreadsheet->columnCount();
// 		while(m_object->contains(i18n("column_description_%1", index)) && (index < columnsCount)) {
// 			m_spreadsheet->column(index)->setComment(m_object->value(i18n("column_description_%1", index)).toString());
// 			++index;
// 		}
// 	}
}
