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
DatasetModel::DatasetModel(const QMap<QString, QMap<QString, QVector<QString>>>& datasetsMap) {
	initCategories(datasetsMap);
	initSubcategories(datasetsMap);
	initDatasets(datasetsMap);
}

DatasetModel::~DatasetModel() {

}

/**
 * @brief Initializes the list of categories.
 */
void DatasetModel::initCategories(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	m_categoryLists = datasetMap.keys();
}

/**
 * @brief Initializes the list of subcategories.
 */
void DatasetModel::initSubcategories(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	for(auto i = datasetMap.begin(); i != datasetMap.end(); ++i) {
		m_subcategories[i.key()] = i.value().keys();
	}
}

/**
 * @brief Initializes the list of datasets.
 */
void DatasetModel::initDatasets(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	for(auto category = datasetMap.begin(); category != datasetMap.end(); ++category) {
		const QMap<QString, QVector<QString>>& category_ = category.value();

		for(auto subcategory = category_.begin(); subcategory != category_.end(); ++subcategory) {
			m_datasets[category.key()][subcategory.key()] = subcategory.value().toList();
			m_datasetList.append(subcategory.value().toList());
		}
	}
}

/**
 * @brief Returns the list of categories.
 */
QVariant DatasetModel::categories() {
	return QVariant(m_categoryLists);
}

/**
 * @brief Returns the list of subcategories of a given category.
 * @param category the category the subcategories of which will be returned
 */
QVariant DatasetModel::subcategories(const QString& category) {
	return QVariant(m_subcategories[category]);
}

/**
 * @brief Returns the list of datasets of a given category and subcategory.
 */
QVariant DatasetModel::datasets(const QString& category, const QString& subcategory) {
	return QVariant(m_datasets[category][subcategory]);
}

/**
 * @brief Returns the list of every dataset.
 */
QVariant DatasetModel::allDatasets() {
	return QVariant(m_datasetList);
}
