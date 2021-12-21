/*
	File                 : ImportDatasetWidget.h
	Project              : LabPlot
	Description          : import online dataset widget
	--------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Kovacs Ferencz <kferike98@gmail.com>
    SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef IMPORTDATASETWIDGET_H
#define IMPORTDATASETWIDGET_H

#include "ui_importdatasetwidget.h"
#include "QMap"
#include <QJsonArray>
#include <QJsonObject>

class DatasetHandler;
class DatasetModel;
class QCompleter;
class QNetworkAccessManager;
class QNetworkReply;

typedef QMap< QString, QMap<QString, QMap<QString, QVector<QString>>>> DatasetsMap;

class ImportDatasetWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportDatasetWidget(QWidget* parent);
	~ImportDatasetWidget() override;

	QString getSelectedDataset() const;
	void import(DatasetHandler*);
	const DatasetsMap& getDatasetsMap();
	void setCollection(const QString&);
	void setCategory(const QString&);
	void setSubcategory(const QString&);
	void setDataset(const QString&);
	void processTest(const QString& category, const QString& subcategory, const QString& dataset, DatasetHandler*);

private:
	Ui::ImportDatasetWidget ui;
	DatasetsMap m_datasetsMap;
	bool m_allCollections{false};
	QString m_collection;
	QString m_category;
	QString m_subcategory;
	QString m_prevCollection;
	QString m_prevCategory;
	QString m_prevSubcategory;
	QString m_prevDataset;
	QCompleter* m_completer{nullptr};
	QString m_jsonDir;
	bool m_initializing{false};
	DatasetModel* m_model{nullptr};
	QNetworkAccessManager* m_networkManager;
	QJsonArray m_collections;
	QJsonObject m_datasetObject;
	QString m_datasetDescription;

	void updateDatasetCompleter();
	void updateCategoryCompleter();
	void updateDatasets(QTreeWidgetItem*);
	void addDatasetItems(const QString& collection, const QString& category, const QString& subcategory, const QString& filter);

	void loadCategories();
	QJsonObject loadDatasetObject();

private Q_SLOTS:
	void datasetChanged();
	void collectionChanged(int);
	void downloadFinished(QNetworkReply*);
	void updateCategories();

Q_SIGNALS:
	void datasetSelected();
	void datasetDoubleClicked();
};

#endif // IMPORTDATASETWIDGET_H
