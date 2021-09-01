/*
	File                 : DatasetModel.h
	Project              : LabPlot
	Description          : Wrapper class for the collections of datasets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Kovacs Ferencz (kferike98@gmail.com)
	SPDX-FileCopyrightText: 2019 Alexander Semke (alexander.semke@web.de)

*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef DATASETMODEL_H
#define DATASETMODEL_H

#include <QObject>
#include <QMap>
#include <QVariant>

class DatasetModel : public QObject {
	Q_OBJECT

public:
	explicit DatasetModel(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>&);
	~DatasetModel();

	QStringList collections();
	QStringList categories(const QString&);
	QStringList subcategories(const QString&, const QString&);
	QStringList datasets(const QString&, const QString&, const QString&);
	int datasetCount(const QString& collection);
	int datasetCount(const QString& collection, const QString& category);
	int datasetCount(const QString& collection, const QString& category, const QString& subcategory);

	Q_INVOKABLE QStringList allCategories();
	Q_INVOKABLE QStringList allSubcategories(const QString&);
	Q_INVOKABLE QVariant allDatasets(const QString&, const QString&);
	Q_INVOKABLE QVariant allDatasetsList();

private:
	QStringList m_collectionList;
	QStringList m_allCategories;
	QMap<QString, QStringList> m_allSubcategories;
	QMap<QString, QMap<QString, QStringList>> m_allDatasets;
	QMap<QString, QStringList> m_categories;
	QMap<QString, QMap<QString, QStringList>> m_subcategories;
	QMap<QString,QMap<QString, QMap<QString, QStringList>>> m_datasets;
	QStringList m_datasetList;

	void initCollections(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>&);
	void initCategories(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetMap);
	void initSubcategories(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetMap);
	void initDatasets(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetMap);
};

#endif //DATASETMODEL_H
