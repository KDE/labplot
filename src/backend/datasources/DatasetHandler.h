/*
	File		: DatasetHandler.h
	Project		: LabPlot
	Description	: Processes a dataset's metadata file
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Kovacs Ferencz <kferike98@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DATASETHANDLER_H
#define DATASETHANDLER_H

#include "backend/spreadsheet/Spreadsheet.h"
#include <QString>

class QJsonObject;
class AsciiFilter;
class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;

class DatasetHandler : public QObject {
	Q_OBJECT

public:
	explicit DatasetHandler(Spreadsheet*);
	~DatasetHandler();
	void processMetadata(const QJsonObject&, const QString&);

private:
	Spreadsheet* m_spreadsheet{nullptr};
	AsciiFilter* m_filter{nullptr};
	QJsonObject* m_object{nullptr};
	QNetworkAccessManager* m_downloadManager{nullptr};
	QNetworkReply* m_currentDownload{nullptr};
	QString m_fileName;
	bool m_invalidMetadataFile{false};
	QString m_containingDir;

	void loadJsonDocument(const QString& path);
	void configureFilter();
	void configureSpreadsheet(const QString&);
	void prepareForDataset();
	void processDataset();
	void doDownload(const QUrl&);
	QString saveFileName(const QUrl&);
	bool saveToDisk(const QString& filename, QIODevice*);
	void markMetadataAsInvalid();

private Q_SLOTS:
	void downloadFinished(QNetworkReply*);

Q_SIGNALS:
	void downloadCompleted();
	void downloadProgress(int progress);
	void error(const QString&);
};

#endif // DATASETHANDLER_H
