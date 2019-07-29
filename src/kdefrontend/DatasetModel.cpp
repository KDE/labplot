/***************************************************************************
	File                 : DatasetModel.cpp
	Project              : LabPlot
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Ferencz Kovacs (kferike98@gmail.com)
	Description          : Wrapper class for datasets, and also for their categories and subcategories
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

/*!
\class DatasetModel
\brief Wrapper class for datasets, and also for their categories and subcategories

\ingroup kdefrontend
*/
DatasetModel::DatasetModel(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetsMap) {
	initCollections(datasetsMap);
	initCategories(datasetsMap);
	initSubcategories(datasetsMap);
	initDatasets(datasetsMap);
}

DatasetModel::~DatasetModel() {

}

/**
 * @brief Initializes the list of collections.
 */
void DatasetModel::initCollections(const QMap<QString, QMap<QString, QMap<QString, QVector<QString> > > > & datasetMap) {
	m_collectionList = datasetMap.keys();
}

/**
 * @brief Initializes the list of categories.
 */
void DatasetModel::initCategories(const QMap<QString, QMap<QString, QMap<QString, QVector<QString> > > > & datasetMap) {
	for(auto i = datasetMap.begin(); i != datasetMap.end(); ++i) {
		m_categories[i.key()] = i.value().keys();

		for(auto category : i.value().keys()) {
			if(!m_allCategories.contains(category))
				m_allCategories.append(category);
		}
	}
}

/**
 * @brief Initializes the list of subcategories.
 */
void DatasetModel::initSubcategories(const QMap<QString, QMap<QString, QMap<QString, QVector<QString> > > > & datasetMap) {
	for(auto collection = datasetMap.begin(); collection != datasetMap.end(); ++collection) {
		const QMap<QString, QMap<QString, QVector<QString> > > collection_ = collection.value();

		for(auto category = collection_.begin(); category != collection_.end(); category++) {
			m_subcategories[collection.key()][category.key()] = category.value().keys();

			for(auto subcategory: category.value().keys()) {
				if(!m_allSubcategories[category.key()].contains(subcategory))
					m_allSubcategories[category.key()].append(subcategory);
			}
		}
	}
}

/**
 * @brief Initializes the list of datasets.
 */
void DatasetModel::initDatasets(const QMap<QString, QMap<QString, QMap<QString, QVector<QString> > > >& datasetMap) {
	for(auto collection = datasetMap.begin(); collection != datasetMap.end(); ++collection) {
		const QMap<QString, QMap<QString, QVector<QString> > > collection_ = collection.value();

		for(auto category = collection_.begin(); category != collection_.end(); category++) {
			const QMap<QString, QVector<QString> >category_ = category.value();

			for(auto subcategory = category_.begin(); subcategory != category_.end(); subcategory++) {
				m_datasets[collection.key()][category.key()][subcategory.key()] = subcategory.value().toList();

				m_allDatasets[category.key()][subcategory.key()].append(subcategory.value().toList());
				m_datasetList.append(subcategory.value().toList());
			}
		}
	}
}

/**
 * @brief Returns the list of categories.
 */
QVariant DatasetModel::allCategories() {
	return QVariant(m_allCategories);
}

/**
 * @brief Returns the list of subcategories of a given category.
 * @param category the category the subcategories of which will be returned
 */
QVariant DatasetModel::allSubcategories(const QString& category) {
	return QVariant(m_allSubcategories[category]);
}

/**
 * @brief Returns the list of datasets of a given category and subcategory.
 */
QVariant DatasetModel::allDatasets(const QString& category, const QString& subcategory) {
	return QVariant(m_allDatasets[category][subcategory]);
}

/**
 * @brief Returns the list of every dataset.
 */
QVariant DatasetModel::allDatasetsList() {
	return QVariant(m_datasetList);
}

/**
 * @brief Returns the list of categories for a given collection
 */
QStringList DatasetModel::categories(const QString& collection) {
	if(collection.compare("All") != 0) {
		return m_categories[collection];
	}else {
		return allCategories().toStringList();
	}
}

/**
 * @brief  Returns the list of subcategories of a given collection and category.
 */
QStringList DatasetModel::subcategories(const QString& collection, const QString& category) {
	if(collection.compare("All") != 0) {
		return m_subcategories[collection][category];
	} else {
		return allSubcategories(category).toStringList();
	}
}

/**
 * @brief Returns the list of datasets of a given collection, category and subcategory.
 */
QStringList DatasetModel::datasets(const QString& collection, const QString& category, const QString& subcategory) {
	if(collection.compare("All") != 0) {
		return m_datasets[collection][category][subcategory];
	} else {
		return allDatasets(category, subcategory).toStringList();
	}
}

int DatasetModel::datasetCount(const QString& collection) {
	int count = 0;
	for(const QString& category: categories(collection)) {
		for(const QString& subcategory: subcategories(collection, category))	{
			count += datasets(collection, category, subcategory).size();
		}
	}
	return count;
}

int DatasetModel::datasetCount(const QString& collection, const QString& category) {
	int count = 0;
	for(const QString& subcategory: subcategories(collection, category))	{
		count += datasets(collection, category, subcategory).size();
	}
	return count;
}

int DatasetModel::datasetCount(const QString& collection, const QString& category, const QString& subcategory) {
	return datasets(collection, category, subcategory).size();
}

/**
 * @brief Returns the list of every collection.
 */
QStringList DatasetModel::collections() {
	return m_collectionList;
}
