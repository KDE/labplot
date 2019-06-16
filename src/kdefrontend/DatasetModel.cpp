/***************************************************************************
	File                 : DatasetModel.h
	Project              : LabPlot
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Ferencz Kovacs (kferike98@gmail.com)
	Description          : Helper class for the welcome screen
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

#include <QVector>

DatasetModel::DatasetModel(const QMap<QString, QMap<QString, QVector<QString>>>& datasetsMap) {
	initCategories(datasetsMap);
	initSubcategories(datasetsMap);
	initDatasets(datasetsMap);
}

DatasetModel::~DatasetModel() {

}

void DatasetModel::initCategories(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	m_categoryLists = datasetMap.keys();
}

void DatasetModel::initSubcategories(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	for(auto i = datasetMap.begin(); i != datasetMap.end(); ++i) {
		m_subcategories[i.key()] = i.value().keys();
	}
}

void DatasetModel::initDatasets(const QMap<QString, QMap<QString, QVector<QString>>>& datasetMap) {
	for(auto category = datasetMap.begin(); category != datasetMap.end(); ++category) {
		const QMap<QString, QVector<QString>>& category_ = category.value();

		for(auto subcategory = category_.begin(); subcategory != category_.end(); ++subcategory) {
			m_datasets[category.key()][subcategory.key()] = subcategory.value().toList();
		}
	}
}

QVariant DatasetModel::categories() {
	return QVariant(m_categoryLists);
}

QVariant DatasetModel::subcategories(const QString& category) {
	return QVariant(m_subcategories[category]);
}

QVariant DatasetModel::datasets(const QString& category, const QString& subcategory) {
	return QVariant(m_datasets[category][subcategory]);
}
