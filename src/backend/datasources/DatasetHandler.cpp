/***************************************************************************
File		: MQTTTopic.cpp
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

#include "backend/datasources/DatasetHandler.h"

#include "QJsonDocument"
#include "QJsonArray"
#include "QJsonObject"
#include "QJsonValue"
#include "QFile"
#include "backend/datasources/filters/AsciiFilter.h"
#include <QtNetwork>
#include "QMessageBox"
#include "KLocalizedString"
#include "QStandardPaths"
#include "QDir"

DatasetHandler::DatasetHandler(const QString& name, bool loading) :  Spreadsheet(name, loading),
	m_filter(new AsciiFilter()),
	m_document(new QJsonDocument()),
	m_downloadManager(new QNetworkAccessManager())
{
	connect(m_downloadManager, &QNetworkAccessManager::finished, this, &DatasetHandler::downloadFinished);
	connect(this, &DatasetHandler::downloadCompleted, this, &DatasetHandler::processDataset);
}

DatasetHandler::~DatasetHandler(){

}

void DatasetHandler::processMetadata(const QString& path) {
	qDebug("Start processing dataset...");
	loadJsonDocument(path);

	if(!m_document->isEmpty()) {
		configureFilter();
		configureSpreadsheet();
		prepareForDataset();
	}
}

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

void DatasetHandler::configureFilter() {
	qDebug("Configure filter");
	if(m_document->isObject()) {
		QJsonObject jsonObject = m_document->object();
		if(jsonObject.contains("separator"))
			m_filter->setSeparatingCharacter( jsonObject.value("separator").toString());

		if(jsonObject.contains("comment_character"))
			m_filter->setCommentCharacter(jsonObject.value("comment_character").toString());

		if(jsonObject.contains("create_index_column"))
			m_filter->setCreateIndexEnabled(jsonObject.value("create_index_column").toBool());

		if(jsonObject.contains("skip_empty_parts"))
			m_filter->setSkipEmptyParts(jsonObject.value("skip_empty_parts").toBool());

		if(jsonObject.contains("simplify_whitespaces"))
			m_filter->setSimplifyWhitespacesEnabled(jsonObject.value("simplify_whitespaces").toBool());

		if(jsonObject.contains("remove_quotes"))
			m_filter->setRemoveQuotesEnabled(jsonObject.value("remove_quotes").toBool());

		if(jsonObject.contains("use_first_row_for_vectorname"))
			m_filter->setHeaderEnabled(jsonObject.value("use_first_row_for_vectorname").toBool());

		if(jsonObject.contains("number_format"))
			m_filter->setNumberFormat(QLocale::Language(jsonObject.value("number_format").toInt()));

		if(jsonObject.contains("DateTime_format"))
			m_filter->setDateTimeFormat(jsonObject.value("DateTime_format").toString());
	} else {
		qDebug("Invalid Json document");
	}
}

void DatasetHandler::configureSpreadsheet() {
	qDebug("Conf spreadsheet");
	if(m_document->isObject()) {
		QJsonObject jsonObject = m_document->object();
		if(jsonObject.contains("name"))
			setName( jsonObject.value("name").toString());

		if(jsonObject.contains("description"))
			setComment(jsonObject.value("description").toString());
	} else {
		qDebug("Invalid Json document");
	}
}

void DatasetHandler::prepareForDataset() {
	qDebug("Start downloading dataset");
	if(m_document->isObject()) {
		QJsonObject jsonObject = m_document->object();

		if(jsonObject.contains("download")) {
			QString url =  jsonObject.value("download").toString();
			QUrl downloadUrl = QUrl::fromEncoded(url.toLocal8Bit());
			doDownload(url);
		}
		else {
			QMessageBox::critical(0, i18n("Invalid metadata file"), i18n("There is no download URL present in the metadata file!"));
		}

	} else {
		qDebug("Invalid Json document");
	}
}

void DatasetHandler::doDownload(const QUrl &url)
{
	qDebug("Download request");
	QNetworkRequest request(url);
	QNetworkReply *reply = m_downloadManager->get(request);

	m_currentDownload = reply;
}

void DatasetHandler::downloadFinished(QNetworkReply *reply)
{
	qDebug("Download finished");
	QUrl url = reply->url();
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

bool DatasetHandler::isHttpRedirect(QNetworkReply *reply)
{
	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	return statusCode == 301 || statusCode == 302 || statusCode == 303
			|| statusCode == 305 || statusCode == 307 || statusCode == 308;
}

QString DatasetHandler::saveFileName(const QUrl &url)
{
	QString path = url.path();
	QString basename = QFileInfo(path).fileName();

	if (basename.isEmpty())
		basename = "download";

	QString baseDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
	QString containingDir = ".labplot";
	QString fileName = baseDir + QDir::separator() + containingDir + QDir::separator() + basename;

	if(!QDir(containingDir).exists()) {
		QDir(baseDir).mkdir(containingDir);
	}

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

bool DatasetHandler::saveToDisk(const QString &filename, QIODevice *data) {
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

void DatasetHandler::processDataset() {
	m_filter->readDataFromFile(m_fileName, this);
}
