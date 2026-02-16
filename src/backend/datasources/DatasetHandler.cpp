/*
	File                 : DatasetHandler.cpp
	Project              : LabPlot
	Description          : Processes a dataset's metadata file
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Kovacs Ferencz <kferike98@gmail.com>
	SPDX-FileCopyrightText: 2019-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/DatasetHandler.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/lib/macros.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTextEdit>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <KLocalizedString>

/*!
  \class DatasetHandler
  \brief Provides  functionality to process a metadata file of a dataset, configure a spreadsheet and filter based on it, download the dataset
  and load it into the spreadsheet.

  \ingroup datasources
*/
DatasetHandler::DatasetHandler(Spreadsheet* spreadsheet)
	: m_spreadsheet(spreadsheet)
	, m_filter(new AsciiFilter)
	, m_downloadManager(new QNetworkAccessManager) {
	connect(m_downloadManager, &QNetworkAccessManager::finished, this, &DatasetHandler::downloadFinished);
	connect(this, &DatasetHandler::downloadCompleted, this, &DatasetHandler::processDataset);
}

DatasetHandler::~DatasetHandler() {
	delete m_downloadManager;
	delete m_filter;
	delete m_object;
}

/**
 * @brief Initiates processing the metadata file,, located at the given path, belonging to a dataset.
 * @param path the path to the metadata file
 */
void DatasetHandler::processMetadata(const QJsonObject& object, const QString& description) {
	delete m_object;
	m_object = new QJsonObject(object);
	DEBUG("Start processing dataset...");

	if (!m_object->isEmpty()) {
		configureFilter();
		configureSpreadsheet(description);
		prepareForDataset();
	}
}

/**
 * @brief Marks the metadata file being invalid by setting the value of a flag, also pops up a messagebox.
 */
void DatasetHandler::markMetadataAsInvalid() {
	m_invalidMetadataFile = true;
	Q_EMIT error(i18n("The metadata file for the selected dataset is invalid."));
}

/**
 * @brief Configures the filter, that will be used later, based on the metadata file.
 */
void DatasetHandler::configureFilter() {
	// set some default values common to many datasets
	auto properties = m_filter->properties();
	properties.locale = QLocale::C;
	properties.skipEmptyParts = false;
	properties.headerEnabled = false;
	properties.removeQuotes = true;

	// read properties specified in the dataset description
	if (!m_object->isEmpty()) {
		if (m_object->contains(QLatin1String("separator"))) {
			const auto separator = m_object->value(QStringLiteral("separator")).toString();
			if (separator.compare(AsciiFilter::autoSeparatorDetectionString(), Qt::CaseInsensitive) == 0) {
				properties.automaticSeparatorDetection = true;
				properties.separator.clear();
			} else {
				properties.automaticSeparatorDetection = false;
				properties.separator = separator;
			}
		}

		if (m_object->contains(QLatin1String("comment_character")))
			properties.commentCharacter = m_object->value(QStringLiteral("comment_character")).toString();

		if (m_object->contains(QLatin1String("create_index_column")))
			properties.createIndex = m_object->value(QStringLiteral("create_index_column")).toBool();

		if (m_object->contains(QLatin1String("skip_empty_parts")))
			properties.skipEmptyParts = m_object->value(QStringLiteral("skip_empty_parts")).toBool();

		if (m_object->contains(QLatin1String("simplify_whitespaces")))
			properties.simplifyWhitespaces = m_object->value(QStringLiteral("simplify_whitespaces")).toBool();

		if (m_object->contains(QLatin1String("remove_quotes")))
			properties.removeQuotes = m_object->value(QStringLiteral("remove_quotes")).toBool();

		if (m_object->contains(QLatin1String("use_first_row_for_vectorname"))) {
			properties.headerEnabled = m_object->value(QStringLiteral("use_first_row_for_vectorname")).toBool();
			properties.headerLine = 1;
		}

		if (m_object->contains(QLatin1String("number_format")))
			properties.locale = QLocale::Language(m_object->value(QStringLiteral("number_format")).toInt());

		if (m_object->contains(QLatin1String("DateTime_format")))
			properties.dateTimeFormat = m_object->value(QStringLiteral("DateTime_format")).toString();

		if (m_object->contains(QLatin1String("columns"))) {
			const QJsonArray& columnsArray = m_object->value(QStringLiteral("columns")).toArray();
			QStringList columnNames;
			for (const auto& col : columnsArray)
				columnNames << col.toString();

			properties.columnNamesString = columnNames.join(QLatin1Char(','));
		}
	} else {
		DEBUG("Empty object");
		markMetadataAsInvalid();
	}

	m_filter->setProperties(properties);
}

/**
 * @brief Configures the spreadsheet based on the metadata file.
 */
void DatasetHandler::configureSpreadsheet(const QString& description) {
	DEBUG("Start preparing spreadsheet");
	if (!m_object->isEmpty()) {
		if (m_object->contains(QLatin1String("name")))
			m_spreadsheet->setName(m_object->value(QStringLiteral("name")).toString());
		else
			markMetadataAsInvalid();

		if (description.startsWith(QLatin1String("<!DOCTYPE html"))) {
			// remove html-formatting
			QTextEdit te;
			te.setHtml(description);
			m_spreadsheet->setComment(te.toPlainText());
		} else
			m_spreadsheet->setComment(description);
	} else
		markMetadataAsInvalid();
}

/**
 * @brief Extracts the download URL of the dataset and initiates the process of download.
 */
void DatasetHandler::prepareForDataset() {
	DEBUG("Start downloading dataset");
	if (!m_object->isEmpty()) {
		if (m_object->contains(QLatin1String("url"))) {
			const QString& url = m_object->value(QStringLiteral("url")).toString();
			doDownload(QUrl(url));
		} else
			Q_EMIT error(i18n("There is no download URL present in the metadata file!"));
	} else
		markMetadataAsInvalid();
}

/**
 * @brief Starts the download of the dataset.
 * @param url the download URL of the dataset
 */
void DatasetHandler::doDownload(const QUrl& url) {
	QDEBUG("Download request " << url);
	QNetworkRequest request(url);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
	m_currentDownload = m_downloadManager->get(request);
	connect(m_currentDownload, &QNetworkReply::downloadProgress, [this](qint64 bytesReceived, qint64 bytesTotal) {
		double progress;
		if (bytesTotal <= 0)
			progress = 0;
		else
			progress = 100 * (static_cast<double>(bytesReceived) / static_cast<double>(bytesTotal));

		Q_EMIT downloadProgress(progress);
	});
}

/**
 * @brief Called when the download of the dataset is finished.
 */
void DatasetHandler::downloadFinished(QNetworkReply* reply) {
	DEBUG("Download finished");
	const QUrl& url = reply->url();
	if (reply->error()) {
		Q_EMIT error(i18n("Failed to download the dataset from %1.\n%2.", url.toDisplayString(), reply->errorString()));
	} else {
		QString filename = saveFileName(url);
		if (saveToDisk(filename, reply)) {
			// qDebug("Download of %s succeeded (saved to %s)\n", url.toEncoded().constData(), qPrintable(filename));
			m_fileName = std::move(filename);
			Q_EMIT downloadCompleted();
		}
	}

	m_currentDownload = nullptr;
	reply->deleteLater();
}

/**
 * @brief Returns the name and path of the file that will contain the content of the reply (based on the URL).
 * @param url
 */
QString DatasetHandler::saveFileName(const QUrl& url) {
	const QString path = url.path();

	// get the extension of the downloaded file
	const QString downloadFileName = QFileInfo(path).fileName();
	int lastIndex = downloadFileName.lastIndexOf(QLatin1Char('.'));
	const QString fileExtension = lastIndex >= 0 ? downloadFileName.right(downloadFileName.length() - lastIndex) : QString();

	QString basename = m_object->value(QStringLiteral("filename")).toString() + fileExtension;

	if (basename.isEmpty())
		basename = QStringLiteral("url");

	QDir downloadDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/datasets_local/"));
	if (!downloadDir.exists()) {
		if (!downloadDir.mkpath(downloadDir.path())) {
			Q_EMIT error(i18n("Failed to create the directory %1 to save the dataset.", downloadDir.path()));
			return {};
		}
	}

	QString fileName = downloadDir.path() + QLatin1Char('/') + basename;
	QFileInfo fileInfo(fileName);
	if (QFile::exists(fileName)) {
		if (fileInfo.lastModified().addDays(1) < QDateTime::currentDateTime()) {
			QFile removeFile(fileName);
			removeFile.remove();
		} else
			DEBUG("Dataset file already exists, no need to download it again.");
	}
	return fileName;
}

/**
 * @brief Saves the content of the network reply to the given path under the given name.
 */
bool DatasetHandler::saveToDisk(const QString& filename, QIODevice* data) {
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		Q_EMIT error(i18n("Couldn't open the file %1 for writing.\n%2", filename, file.errorString()));
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

	// set column comments/descriptions, if available
	// TODO:
	// 	if (!m_object->isEmpty()) {
	// 		int index = 0;
	// 		const int columnsCount = m_spreadsheet->columnCount();
	// 		while(m_object->contains(i18n("column_description_%1", index)) && (index < columnsCount)) {
	// 			m_spreadsheet->column(index)->setComment(m_object->value(i18n("column_description_%1", index)).toString());
	// 			++index;
	// 		}
	// 	}
}
