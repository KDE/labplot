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

#include <QString>
#include "backend/spreadsheet/Spreadsheet.h"

class QJsonObject;
class AsciiFilter;
class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;

class DatasetHandler : public QObject{
	Q_OBJECT

public:
	explicit DatasetHandler(Spreadsheet*);
	~DatasetHandler();
	void processMetadata(const QJsonObject&, const QString&);

private:
	Spreadsheet* m_spreadsheet;
	AsciiFilter* m_filter;
	QJsonObject* m_object{nullptr};
	QNetworkAccessManager* m_downloadManager;
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
	bool isHttpRedirect(QNetworkReply*);
	QString saveFileName(const QUrl&);
	bool saveToDisk(const QString& filename, QIODevice*);
	void markMetadataAsInvalid();

private slots:
	void downloadFinished(QNetworkReply*);

signals:
	void downloadCompleted();
	void downloadProgress(int progress);
};

#endif // DATASETHANDLER_H
